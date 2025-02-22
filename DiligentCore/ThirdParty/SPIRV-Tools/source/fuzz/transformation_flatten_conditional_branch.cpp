// Copyright (c) 2020 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "source/fuzz/transformation_flatten_conditional_branch.h"

#include "source/fuzz/fuzzer_util.h"
#include "source/fuzz/instruction_descriptor.h"

namespace spvtools {
namespace fuzz {

TransformationFlattenConditionalBranch::TransformationFlattenConditionalBranch(
    const protobufs::TransformationFlattenConditionalBranch& message)
    : message_(message) {}

TransformationFlattenConditionalBranch::TransformationFlattenConditionalBranch(
    uint32_t header_block_id, bool true_branch_first,
    const std::vector<protobufs::SideEffectWrapperInfo>&
        side_effect_wrappers_info) {
  message_.set_header_block_id(header_block_id);
  message_.set_true_branch_first(true_branch_first);
  for (auto const& side_effect_wrapper_info : side_effect_wrappers_info) {
    *message_.add_side_effect_wrapper_info() = side_effect_wrapper_info;
  }
}

bool TransformationFlattenConditionalBranch::IsApplicable(
    opt::IRContext* ir_context,
    const TransformationContext& transformation_context) const {
  uint32_t header_block_id = message_.header_block_id();
  auto header_block = fuzzerutil::MaybeFindBlock(ir_context, header_block_id);

  // The block must have been found and it must be a selection header.
  if (!header_block || !header_block->GetMergeInst() ||
      header_block->GetMergeInst()->opcode() != SpvOpSelectionMerge) {
    return false;
  }

  // The header block must end with an OpBranchConditional instruction.
  if (header_block->terminator()->opcode() != SpvOpBranchConditional) {
    return false;
  }

  // Use a set to keep track of the instructions that require fresh ids.
  std::set<opt::Instruction*> instructions_that_need_ids;

  // Check that, if there are enough ids, the conditional can be flattened and,
  // if so, add all the problematic instructions that need to be enclosed inside
  // conditionals to |instructions_that_need_ids|.
  if (!GetProblematicInstructionsIfConditionalCanBeFlattened(
          ir_context, header_block, &instructions_that_need_ids)) {
    return false;
  }

  // Get the mapping from instructions to the fresh ids needed to enclose them
  // inside conditionals.
  auto insts_to_wrapper_info = GetInstructionsToWrapperInfo(ir_context);

  {
    std::set<uint32_t> used_fresh_ids;

    // Check the ids in the map.
    for (const auto& inst_to_info : insts_to_wrapper_info) {
      // Check the fresh ids needed for all of the instructions that need to be
      // enclosed inside a conditional.
      for (uint32_t id : {inst_to_info.second.merge_block_id(),
                          inst_to_info.second.execute_block_id()}) {
        if (!id || !CheckIdIsFreshAndNotUsedByThisTransformation(
                       id, ir_context, &used_fresh_ids)) {
          return false;
        }
      }

      // Check the other ids needed, if the instruction needs a placeholder.
      if (InstructionNeedsPlaceholder(ir_context, *inst_to_info.first)) {
        // Check the fresh ids.
        for (uint32_t id : {inst_to_info.second.actual_result_id(),
                            inst_to_info.second.alternative_block_id(),
                            inst_to_info.second.placeholder_result_id()}) {
          if (!id || !CheckIdIsFreshAndNotUsedByThisTransformation(
                         id, ir_context, &used_fresh_ids)) {
            return false;
          }
        }

        // Check that the placeholder value id exists, has the right type and is
        // available to use at this point.
        auto value_def = ir_context->get_def_use_mgr()->GetDef(
            inst_to_info.second.value_to_copy_id());
        if (!value_def ||
            value_def->type_id() != inst_to_info.first->type_id() ||
            !fuzzerutil::IdIsAvailableBeforeInstruction(
                ir_context, inst_to_info.first,
                inst_to_info.second.value_to_copy_id())) {
          return false;
        }
      }
    }
  }

  // If some instructions that require ids are not in the map, the
  // transformation needs overflow ids to be applicable.
  for (auto instruction : instructions_that_need_ids) {
    if (insts_to_wrapper_info.count(instruction) == 0 &&
        !transformation_context.GetOverflowIdSource()->HasOverflowIds()) {
      return false;
    }
  }

  // All checks were passed.
  return true;
}

void TransformationFlattenConditionalBranch::Apply(
    opt::IRContext* ir_context,
    TransformationContext* transformation_context) const {
  uint32_t header_block_id = message_.header_block_id();
  auto header_block = ir_context->cfg()->block(header_block_id);

  // Find the first block where flow converges (it is not necessarily the merge
  // block) by walking the true branch until reaching a block that
  // post-dominates the header.
  // This is necessary because a potential common set of blocks at the end of
  // the construct should not be duplicated.
  uint32_t convergence_block_id =
      header_block->terminator()->GetSingleWordInOperand(1);
  auto postdominator_analysis =
      ir_context->GetPostDominatorAnalysis(header_block->GetParent());
  while (!postdominator_analysis->Dominates(convergence_block_id,
                                            header_block_id)) {
    auto current_block = ir_context->get_instr_block(convergence_block_id);
    // If the transformation is applicable, the terminator is OpBranch.
    convergence_block_id =
        current_block->terminator()->GetSingleWordInOperand(0);
  }

  auto branch_instruction = header_block->terminator();

  // branch = 1 corresponds to the true branch, branch = 2 corresponds to the
  // false branch. If the true branch is to be laid out first, we need to visit
  // the false branch first, because each branch is moved to right after the
  // header while it is visited.
  std::vector<uint32_t> branches = {2, 1};
  if (!message_.true_branch_first()) {
    // Similarly, we need to visit the true branch first, if we want it to be
    // laid out after the false branch.
    branches = {1, 2};
  }

  // Get the ids of the starting blocks of the first and last branches to be
  // laid out. The first branch is the true branch iff
  // |message_.true_branch_first| is true.
  uint32_t first_block_first_branch_id =
      branch_instruction->GetSingleWordInOperand(branches[1]);
  uint32_t first_block_last_branch_id =
      branch_instruction->GetSingleWordInOperand(branches[0]);

  // If the OpBranchConditional instruction in the header branches to the same
  // block for both values of the condition, this is the convergence block (the
  // flow does not actually diverge) and the OpPhi instructions in it are still
  // valid, so we do not need to make any changes.
  if (first_block_first_branch_id != first_block_last_branch_id) {
    // Replace all of the current OpPhi instructions in the convergence block
    // with OpSelect.
    ir_context->get_instr_block(convergence_block_id)
        ->ForEachPhiInst([branch_instruction, header_block,
                          ir_context](opt::Instruction* phi_inst) {
          assert(phi_inst->NumInOperands() == 4 &&
                 "We are going to replace an OpPhi with an OpSelect.  This "
                 "only makes sense if the block has two distinct "
                 "predecessors.");
          // The OpPhi takes values from two distinct predecessors.  One
          // predecessor is associated with the "true" path of the conditional
          // we are flattening, the other with the "false" path, but these
          // predecessors can appear in either order as operands to the OpPhi
          // instruction.

          std::vector<opt::Operand> operands;
          operands.emplace_back(branch_instruction->GetInOperand(0));

          uint32_t branch_instruction_true_block_id =
              branch_instruction->GetSingleWordInOperand(1);

          if (ir_context->GetDominatorAnalysis(header_block->GetParent())
                  ->Dominates(branch_instruction_true_block_id,
                              phi_inst->GetSingleWordInOperand(1))) {
            // The "true" branch is handled first in the OpPhi's operands; we
            // thus provide operands to OpSelect in the same order that they
            // appear in the OpPhi.
            operands.emplace_back(phi_inst->GetInOperand(0));
            operands.emplace_back(phi_inst->GetInOperand(2));
          } else {
            // The "false" branch is handled first in the OpPhi's operands; we
            // thus provide operands to OpSelect in reverse of the order that
            // they appear in the OpPhi.
            operands.emplace_back(phi_inst->GetInOperand(2));
            operands.emplace_back(phi_inst->GetInOperand(0));
          }
          phi_inst->SetOpcode(SpvOpSelect);
          phi_inst->SetInOperands(std::move(operands));
        });
  }

  // Get the mapping from instructions to fresh ids.
  auto insts_to_info = GetInstructionsToWrapperInfo(ir_context);

  // Get a reference to the last block in the first branch that will be laid out
  // (this depends on |message_.true_branch_first|). The last block is the block
  // in the branch just before flow converges (it might not exist).
  opt::BasicBlock* last_block_first_branch = nullptr;

  // Keep track of blocks and ids for which we should later add dead block and
  // irrelevant id facts.  We wait until we have finished applying the
  // transformation before adding these facts, so that the fact manager has
  // access to the fully up-to-date module.
  std::vector<uint32_t> dead_blocks;
  std::vector<uint32_t> irrelevant_ids;

  // Adjust the conditional branches by enclosing problematic instructions
  // within conditionals and get references to the last block in each branch.
  for (uint32_t branch : branches) {
    auto current_block = header_block;
    // Get the id of the first block in this branch.
    uint32_t next_block_id = branch_instruction->GetSingleWordInOperand(branch);

    // Consider all blocks in the branch until the convergence block is reached.
    while (next_block_id != convergence_block_id) {
      // Move the next block to right after the current one.
      current_block->GetParent()->MoveBasicBlockToAfter(next_block_id,
                                                        current_block);

      // Move forward in the branch.
      current_block = ir_context->cfg()->block(next_block_id);

      // Find all the instructions in the current block which need to be
      // enclosed inside conditionals.
      std::vector<opt::Instruction*> problematic_instructions;

      current_block->ForEachInst(
          [&problematic_instructions](opt::Instruction* instruction) {
            if (instruction->opcode() != SpvOpLabel &&
                instruction->opcode() != SpvOpBranch &&
                !fuzzerutil::InstructionHasNoSideEffects(*instruction)) {
              problematic_instructions.push_back(instruction);
            }
          });

      uint32_t condition_id =
          header_block->terminator()->GetSingleWordInOperand(0);

      // Enclose all of the problematic instructions in conditionals, with the
      // same condition as the selection construct being flattened.
      for (auto instruction : problematic_instructions) {
        // Get the info needed by this instruction to wrap it inside a
        // conditional.
        protobufs::SideEffectWrapperInfo wrapper_info;

        if (insts_to_info.count(instruction) != 0) {
          // Get the fresh ids from the map, if present.
          wrapper_info = insts_to_info[instruction];
        } else {
          // If we could not get it from the map, use overflow ids. We don't
          // need to set |wrapper_info.instruction|, as it will not be used.
          wrapper_info.set_merge_block_id(
              transformation_context->GetOverflowIdSource()
                  ->GetNextOverflowId());
          wrapper_info.set_execute_block_id(
              transformation_context->GetOverflowIdSource()
                  ->GetNextOverflowId());

          if (InstructionNeedsPlaceholder(ir_context, *instruction)) {
            // Ge the fresh ids from the overflow ids.
            wrapper_info.set_actual_result_id(
                transformation_context->GetOverflowIdSource()
                    ->GetNextOverflowId());
            wrapper_info.set_alternative_block_id(
                transformation_context->GetOverflowIdSource()
                    ->GetNextOverflowId());
            wrapper_info.set_placeholder_result_id(
                transformation_context->GetOverflowIdSource()
                    ->GetNextOverflowId());

            // Try to find a zero constant. It does not matter whether it is
            // relevant or irrelevant.
            for (bool is_irrelevant : {true, false}) {
              wrapper_info.set_value_to_copy_id(
                  fuzzerutil::MaybeGetZeroConstant(
                      ir_context, *transformation_context,
                      instruction->type_id(), is_irrelevant));
              if (wrapper_info.value_to_copy_id()) {
                break;
              }
            }
          }
        }

        // Enclose the instruction in a conditional and get the merge block
        // generated by this operation (this is where all the following
        // instructions will be).
        current_block = EncloseInstructionInConditional(
            ir_context, *transformation_context, current_block, instruction,
            wrapper_info, condition_id, branch == 1, &dead_blocks,
            &irrelevant_ids);
      }

      next_block_id = current_block->terminator()->GetSingleWordInOperand(0);

      // If the next block is the convergence block and this the branch that
      // will be laid out right after the header, record this as the last block
      // in the first branch.
      if (next_block_id == convergence_block_id && branch == branches[1]) {
        last_block_first_branch = current_block;
      }
    }
  }

  // The current header should unconditionally branch to the starting block in
  // the first branch to be laid out, if such a branch exists (i.e. the header
  // does not branch directly to the convergence block), and to the starting
  // block in the last branch to be laid out otherwise.
  uint32_t after_header = first_block_first_branch_id != convergence_block_id
                              ? first_block_first_branch_id
                              : first_block_last_branch_id;

  // Kill the merge instruction and the branch instruction in the current
  // header.
  auto merge_inst = header_block->GetMergeInst();
  ir_context->KillInst(branch_instruction);
  ir_context->KillInst(merge_inst);

  // Add a new, unconditional, branch instruction from the current header to
  // |after_header|.
  header_block->AddInstruction(MakeUnique<opt::Instruction>(
      ir_context, SpvOpBranch, 0, 0,
      opt::Instruction::OperandList{{SPV_OPERAND_TYPE_ID, {after_header}}}));

  // If the first branch to be laid out exists, change the branch instruction so
  // that the last block in such branch unconditionally branches to the first
  // block in the other branch (or the convergence block if there is no other
  // branch) and change the OpPhi instructions in the last branch accordingly
  // (the predecessor changed).
  if (last_block_first_branch) {
    last_block_first_branch->terminator()->SetInOperand(
        0, {first_block_last_branch_id});

    // Change the OpPhi instructions of the last branch (if there is another
    // branch) so that the predecessor is now the last block of the first
    // branch. The block must have a single predecessor, so the operand
    // specifying the predecessor is always in the same position.
    if (first_block_last_branch_id != convergence_block_id) {
      ir_context->get_instr_block(first_block_last_branch_id)
          ->ForEachPhiInst(
              [&last_block_first_branch](opt::Instruction* phi_inst) {
                // The operand specifying the predecessor is the second input
                // operand.
                phi_inst->SetInOperand(1, {last_block_first_branch->id()});
              });
    }
  }

  // Invalidate all analyses
  ir_context->InvalidateAnalysesExceptFor(opt::IRContext::kAnalysisNone);

  // Now that we have finished adding blocks and ids to the module and
  // invalidated existing analyses, update the fact manager regarding dead
  // blocks and irrelevant ids.
  for (auto dead_block : dead_blocks) {
    transformation_context->GetFactManager()->AddFactBlockIsDead(dead_block);
  }
  for (auto irrelevant_id : irrelevant_ids) {
    transformation_context->GetFactManager()->AddFactIdIsIrrelevant(
        irrelevant_id);
  }
}

protobufs::Transformation TransformationFlattenConditionalBranch::ToMessage()
    const {
  protobufs::Transformation result;
  *result.mutable_flatten_conditional_branch() = message_;
  return result;
}

bool TransformationFlattenConditionalBranch::
    GetProblematicInstructionsIfConditionalCanBeFlattened(
        opt::IRContext* ir_context, opt::BasicBlock* header,
        std::set<opt::Instruction*>* instructions_that_need_ids) {
  uint32_t merge_block_id = header->MergeBlockIdIfAny();
  assert(merge_block_id &&
         header->GetMergeInst()->opcode() == SpvOpSelectionMerge &&
         header->terminator()->opcode() == SpvOpBranchConditional &&
         "|header| must be the header of a conditional.");

  auto enclosing_function = header->GetParent();
  auto dominator_analysis =
      ir_context->GetDominatorAnalysis(enclosing_function);
  auto postdominator_analysis =
      ir_context->GetPostDominatorAnalysis(enclosing_function);

  // Check that the header and the merge block describe a single-entry,
  // single-exit region.
  if (!dominator_analysis->Dominates(header->id(), merge_block_id) ||
      !postdominator_analysis->Dominates(merge_block_id, header->id())) {
    return false;
  }

  // Traverse the CFG starting from the header and check that, for all the
  // blocks that can be reached by the header before the flow converges:
  //  - they don't contain merge, barrier or OpSampledImage instructions
  //  - they branch unconditionally to another block
  //  Add any side-effecting instruction, requiring fresh ids, to
  //  |instructions_that_need_ids|
  std::list<uint32_t> to_check;
  header->ForEachSuccessorLabel(
      [&to_check](uint32_t label) { to_check.push_back(label); });

  while (!to_check.empty()) {
    uint32_t block_id = to_check.front();
    to_check.pop_front();

    // If the block post-dominates the header, this is where flow converges, and
    // we don't need to check this branch any further, because the
    // transformation will only change the part of the graph where flow is
    // divergent.
    if (postdominator_analysis->Dominates(block_id, header->id())) {
      continue;
    }

    auto block = ir_context->cfg()->block(block_id);

    // The block must not have a merge instruction, because inner constructs are
    // not allowed.
    if (block->GetMergeInst()) {
      return false;
    }

    // The terminator instruction for the block must be OpBranch.
    if (block->terminator()->opcode() != SpvOpBranch) {
      return false;
    }

    // Check all of the instructions in the block.
    bool all_instructions_compatible =
        block->WhileEachInst([ir_context, instructions_that_need_ids](
                                 opt::Instruction* instruction) {
          // We can ignore OpLabel instructions.
          if (instruction->opcode() == SpvOpLabel) {
            return true;
          }

          // If the instruction is a branch, it must be an unconditional branch.
          if (instruction->IsBranch()) {
            return instruction->opcode() == SpvOpBranch;
          }

          // We cannot go ahead if we encounter an instruction that cannot be
          // handled.
          if (!InstructionCanBeHandled(ir_context, *instruction)) {
            return false;
          }

          // If the instruction has side effects, add it to the
          // |instructions_that_need_ids| set.
          if (!fuzzerutil::InstructionHasNoSideEffects(*instruction)) {
            instructions_that_need_ids->emplace(instruction);
          }

          return true;
        });

    if (!all_instructions_compatible) {
      return false;
    }

    // Add the successor of this block to the list of blocks that need to be
    // checked.
    to_check.push_back(block->terminator()->GetSingleWordInOperand(0));
  }

  // All the blocks are compatible with the transformation and this is indeed a
  // single-entry, single-exit region.
  return true;
}

bool TransformationFlattenConditionalBranch::InstructionNeedsPlaceholder(
    opt::IRContext* ir_context, const opt::Instruction& instruction) {
  assert(!fuzzerutil::InstructionHasNoSideEffects(instruction) &&
         InstructionCanBeHandled(ir_context, instruction) &&
         "The instruction must have side effects and it must be possible to "
         "enclose it inside a conditional.");

  if (instruction.HasResultId()) {
    // We need a placeholder iff the type is not Void.
    auto type = ir_context->get_type_mgr()->GetType(instruction.type_id());
    return type && !type->AsVoid();
  }

  return false;
}

std::unordered_map<opt::Instruction*, protobufs::SideEffectWrapperInfo>
TransformationFlattenConditionalBranch::GetInstructionsToWrapperInfo(
    opt::IRContext* ir_context) const {
  std::unordered_map<opt::Instruction*, protobufs::SideEffectWrapperInfo>
      instructions_to_ids;
  for (const auto& wrapper_info : message_.side_effect_wrapper_info()) {
    auto instruction = FindInstruction(wrapper_info.instruction(), ir_context);
    if (instruction) {
      instructions_to_ids.emplace(instruction, wrapper_info);
    }
  }

  return instructions_to_ids;
}

opt::BasicBlock*
TransformationFlattenConditionalBranch::EncloseInstructionInConditional(
    opt::IRContext* ir_context,
    const TransformationContext& transformation_context, opt::BasicBlock* block,
    opt::Instruction* instruction,
    const protobufs::SideEffectWrapperInfo& wrapper_info, uint32_t condition_id,
    bool exec_if_cond_true, std::vector<uint32_t>* dead_blocks,
    std::vector<uint32_t>* irrelevant_ids) const {
  // Get the next instruction (it will be useful for splitting).
  auto next_instruction = instruction->NextNode();

  // Update the module id bound.
  for (uint32_t id :
       {wrapper_info.merge_block_id(), wrapper_info.execute_block_id()}) {
    fuzzerutil::UpdateModuleIdBound(ir_context, id);
  }

  // Create the block where the instruction is executed by splitting the
  // original block.
  auto execute_block = block->SplitBasicBlock(
      ir_context, wrapper_info.execute_block_id(),
      fuzzerutil::GetIteratorForInstruction(block, instruction));

  // Create the merge block for the conditional that we are about to create by
  // splitting execute_block (this will leave |instruction| as the only
  // instruction in |execute_block|).
  auto merge_block = execute_block->SplitBasicBlock(
      ir_context, wrapper_info.merge_block_id(),
      fuzzerutil::GetIteratorForInstruction(execute_block, next_instruction));

  // Propagate the fact that the block is dead to the newly-created blocks.
  if (transformation_context.GetFactManager()->BlockIsDead(block->id())) {
    dead_blocks->emplace_back(execute_block->id());
    dead_blocks->emplace_back(merge_block->id());
  }

  // Initially, consider the merge block as the alternative block to branch to
  // if the instruction should not be executed.
  auto alternative_block = merge_block;

  // Add an unconditional branch from |execute_block| to |merge_block|.
  execute_block->AddInstruction(MakeUnique<opt::Instruction>(
      ir_context, SpvOpBranch, 0, 0,
      opt::Instruction::OperandList{
          {SPV_OPERAND_TYPE_ID, {merge_block->id()}}}));

  // If the instruction requires a placeholder, it means that it has a result id
  // and its result needs to be able to be used later on, and we need to:
  // - add an additional block |ids.alternative_block_id| where a placeholder
  //   result id (using fresh id |ids.placeholder_result_id|) is obtained either
  //   by using OpCopyObject and copying |ids.value_to_copy_id| or, if such id
  //   was not given and a suitable constant was not found, by using OpUndef.
  // - mark |ids.placeholder_result_id| as irrelevant
  // - change the result id of the instruction to a fresh id
  //   (|ids.actual_result_id|).
  // - add an OpPhi instruction, which will have the original result id of the
  //   instruction, in the merge block.
  if (InstructionNeedsPlaceholder(ir_context, *instruction)) {
    // Update the module id bound with the additional ids.
    for (uint32_t id :
         {wrapper_info.actual_result_id(), wrapper_info.alternative_block_id(),
          wrapper_info.placeholder_result_id()}) {
      fuzzerutil::UpdateModuleIdBound(ir_context, id);
    }

    // Create a new block using |fresh_ids.alternative_block_id| for its label.
    auto alternative_block_temp =
        MakeUnique<opt::BasicBlock>(MakeUnique<opt::Instruction>(
            ir_context, SpvOpLabel, 0, wrapper_info.alternative_block_id(),
            opt::Instruction::OperandList{}));

    // Keep the original result id of the instruction in a variable.
    uint32_t original_result_id = instruction->result_id();

    // Set the result id of the instruction to be |ids.actual_result_id|.
    instruction->SetResultId(wrapper_info.actual_result_id());

    // Add a placeholder instruction, with the same type as the original
    // instruction and id |ids.placeholder_result_id|, to the new block.
    if (wrapper_info.value_to_copy_id()) {
      // If there is an available id to copy from, the placeholder instruction
      // will be %placeholder_result_id = OpCopyObject %type %value_to_copy_id
      alternative_block_temp->AddInstruction(MakeUnique<opt::Instruction>(
          ir_context, SpvOpCopyObject, instruction->type_id(),
          wrapper_info.placeholder_result_id(),
          opt::Instruction::OperandList{
              {SPV_OPERAND_TYPE_ID, {wrapper_info.value_to_copy_id()}}}));
    } else {
      // If there is no such id, use an OpUndef instruction.
      alternative_block_temp->AddInstruction(MakeUnique<opt::Instruction>(
          ir_context, SpvOpUndef, instruction->type_id(),
          wrapper_info.placeholder_result_id(),
          opt::Instruction::OperandList{}));
    }

    // Mark |ids.placeholder_result_id| as irrelevant.
    irrelevant_ids->emplace_back(wrapper_info.placeholder_result_id());

    // Add an unconditional branch from the new block to the merge block.
    alternative_block_temp->AddInstruction(MakeUnique<opt::Instruction>(
        ir_context, SpvOpBranch, 0, 0,
        opt::Instruction::OperandList{
            {SPV_OPERAND_TYPE_ID, {merge_block->id()}}}));

    // Insert the block before the merge block.
    alternative_block = block->GetParent()->InsertBasicBlockBefore(
        std::move(alternative_block_temp), merge_block);

    // Using the original instruction result id, add an OpPhi instruction to the
    // merge block, which will either take the value of the result of the
    // instruction or the placeholder value defined in the alternative block.
    merge_block->begin().InsertBefore(MakeUnique<opt::Instruction>(
        ir_context, SpvOpPhi, instruction->type_id(), original_result_id,
        opt::Instruction::OperandList{
            {SPV_OPERAND_TYPE_ID, {instruction->result_id()}},
            {SPV_OPERAND_TYPE_ID, {execute_block->id()}},
            {SPV_OPERAND_TYPE_ID, {wrapper_info.placeholder_result_id()}},
            {SPV_OPERAND_TYPE_ID, {alternative_block->id()}}}));

    // Propagate the fact that the block is dead to the new block.
    if (transformation_context.GetFactManager()->BlockIsDead(block->id())) {
      dead_blocks->emplace_back(alternative_block->id());
    }
  }

  // Depending on whether the instruction should be executed in the if branch or
  // in the else branch, get the corresponding ids.
  auto if_block_id = (exec_if_cond_true ? execute_block : alternative_block)
                         ->GetLabel()
                         ->result_id();
  auto else_block_id = (exec_if_cond_true ? alternative_block : execute_block)
                           ->GetLabel()
                           ->result_id();

  // Add an OpSelectionMerge instruction to the block.
  block->AddInstruction(MakeUnique<opt::Instruction>(
      ir_context, SpvOpSelectionMerge, 0, 0,
      opt::Instruction::OperandList{{SPV_OPERAND_TYPE_ID, {merge_block->id()}},
                                    {SPV_OPERAND_TYPE_SELECTION_CONTROL,
                                     {SpvSelectionControlMaskNone}}}));

  // Add an OpBranchConditional, to the block, using |condition_id| as the
  // condition and branching to |if_block_id| if the condition is true and to
  // |else_block_id| if the condition is false.
  block->AddInstruction(MakeUnique<opt::Instruction>(
      ir_context, SpvOpBranchConditional, 0, 0,
      opt::Instruction::OperandList{{SPV_OPERAND_TYPE_ID, {condition_id}},
                                    {SPV_OPERAND_TYPE_ID, {if_block_id}},
                                    {SPV_OPERAND_TYPE_ID, {else_block_id}}}));

  return merge_block;
}

bool TransformationFlattenConditionalBranch::InstructionCanBeHandled(
    opt::IRContext* ir_context, const opt::Instruction& instruction) {
  // We can handle all instructions with no side effects.
  if (fuzzerutil::InstructionHasNoSideEffects(instruction)) {
    return true;
  }

  // We cannot handle barrier instructions, while we should be able to handle
  // all other instructions by enclosing them inside a conditional.
  if (instruction.opcode() == SpvOpControlBarrier ||
      instruction.opcode() == SpvOpMemoryBarrier ||
      instruction.opcode() == SpvOpNamedBarrierInitialize ||
      instruction.opcode() == SpvOpMemoryNamedBarrier ||
      instruction.opcode() == SpvOpTypeNamedBarrier) {
    return false;
  }

  // We cannot handle OpSampledImage instructions, as they need to be in the
  // same block as their use.
  if (instruction.opcode() == SpvOpSampledImage) {
    return false;
  }

  // We cannot handle instructions with an id which return a void type, if the
  // result id is used in the module (e.g. a function call to a function that
  // returns nothing).
  if (instruction.HasResultId()) {
    auto type = ir_context->get_type_mgr()->GetType(instruction.type_id());
    assert(type && "The type should be found in the module");

    if (type->AsVoid() &&
        !ir_context->get_def_use_mgr()->WhileEachUse(
            instruction.result_id(),
            [](opt::Instruction* use_inst, uint32_t use_index) {
              // Return false if the id is used as an input operand.
              return use_index <
                     use_inst->NumOperands() - use_inst->NumInOperands();
            })) {
      return false;
    }
  }

  return true;
}

std::unordered_set<uint32_t>
TransformationFlattenConditionalBranch::GetFreshIds() const {
  std::unordered_set<uint32_t> result;
  for (auto& side_effect_wrapper_info : message_.side_effect_wrapper_info()) {
    result.insert(side_effect_wrapper_info.merge_block_id());
    result.insert(side_effect_wrapper_info.execute_block_id());
    result.insert(side_effect_wrapper_info.actual_result_id());
    result.insert(side_effect_wrapper_info.alternative_block_id());
    result.insert(side_effect_wrapper_info.placeholder_result_id());
  }
  return result;
}

}  // namespace fuzz
}  // namespace spvtools
