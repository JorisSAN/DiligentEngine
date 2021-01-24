# React Physic Test

---------------------------------
Installer React
---------------------------------

1 - Cloner le repo : https://github.com/DanielChappuis/reactphysics3d.git

2 - Avoir Cmake et Visual Studio 2017 ou 2019

3 - Utiliser Cmake pour faire un build du projet ReactPhysic

4 - Ouvrir le .sln créer par le build. Passer le type de solution crée de Debug à Release
    (tout en haut de visual stidio, à gauche de la ou est écrit x64).
    Ouvrir l'explorateur de solution à gauche, click droit sur la solution reactphysics3d et build.

5 - Ouvrir visual studio en mode admin, ouvrir le .sln de ReactPhysic3D.
    Passer le type de solution crée de Debug à Release.
    Ouvrir l'explorateur de solution, click droit sur la solution INSTALL et buid.
    Ca permet d'installer ReactPhysics sur votre ordi, l'adresse est C:\Program Files (x86)\ReactPhysics3D\


6 - GG vous avez installé react.


---------------------------------
Ajouter React dans votre .sln
---------------------------------
1 - Passez votre projet de Debug à Release.

2 - Ouvrir l'explorateur de solution, ouvrir le dossier Projects, click droit sur ReactPhysicTest, ouvrir Propriété (tout en bas).

3 - Ouvrir C++ -> General -> "Considérer les avertissements comme des erreurs" -> "Non (/WX-)
    (La bibliothèque react à son propre système de type, elle effectue des conversions de ses types avec les types c++ standards.
     Ca provoque des warnings qui peuvent être pris pour des erreurs, c'est pour ça qu'on doit le désactiver, je suis d'accord c'est pas ouf)

4 - Ouvrir C++ -> General -> "Autres répertoires include" -> Ajouter "C:\Program Files (x86)\ReactPhysics3D\include"
    (Si l'installation s'est bien déroulée, c'est ici que vous devriez trouver ReactPhysic)

4 - Ouvrir Editeurs de liens -> Entrée -> "Dépendances supplémentaires" -> Ajouter"C:\Program Files (x86)\ReactPhysics3D\lib\reactphysics3d.lib"

5 - Clicker sur "Appliquer"

6 - Pour ajouter React dans votre projet il suffit maintenant d'ajouter  "#include <reactphysics3d/reactphysics3d.h>"

Pour le moment ces étapes doivent être refaites à chaque fois qu'on build avec CMake, c'est pas ouf, je vais regarder ce que je peux faire pour automatiser ça


Mon tuto est un condensé du tuto de react : https://www.reactphysics3d.com/usermanual.html