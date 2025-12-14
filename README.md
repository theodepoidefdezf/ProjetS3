Projet S3 – Fonctionnement général

Le projet est basé sur les réseaux de neurones et le deep learning. Il permet de traiter des images, de reconnaître des caractères (OCR) et de résoudre le problème final à l’aide d’un solveur. Le projet est divisé en plusieurs modules.


Interface graphique (Display)

Aller dans le dossier display
Compiler avec :

make

Lancer l’interface graphique avec :

./display

L’interface affiche tous les modules du projet et, lors du run final, toutes les images générées.


Prétraitement

Aller dans le dossier Preprocessing
Compiler avec :

make

Lancer le prétraitement avec :

./preprocessing_test ../Images/Niveau…

On peut choisir le niveau et l’image à traiter.
L’image finale est générée dans le dossier output.


Découpage

Compiler le module avec :

make

Lancer le découpage sur l’image prétraitée :

fichier executable ../output/image_noise_auto.bmp


Réseau de neurones (OCR)

Aller dans le dossier ocr
Compiler avec :

make

Lancer l’exécutable.
Les caractères reconnus s’affichent directement.


Solveur

Aller dans le dossier solver
Compiler avec :

make

Lancer le solveur avec :

./solver grid mots

Le résultat final s’affiche à l’écran.