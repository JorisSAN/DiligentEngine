# ADD the Log Class

If you want to add a log window: 
* Copy log.h and the log.cpp file into your directory and add them to the cmake text file
* Reconfigure and regenerate your project

To use it in your scene:
* Include ```"log.h"```
* Create a Log variable  like that   ``` Log log;  ```
* In your scene update do  ``` log.Draw(); ```


To add info to the logs use  ```log.addInfo(my_message); ```
The variable my_message is a of type string

You can Save in the file  ```log.txt ```  with 
 ``` log.save(); ```
 or by using the save button in the menu of the log window
 
 You can also clear the logs by using 
  ```log.clear(); ```
or by using  the clear button in the menu

You can also save at each draw by ticking the autosave checkbox
or by using  ```log.setAutoSave(true); ```

