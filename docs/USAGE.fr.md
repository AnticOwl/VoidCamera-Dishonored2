# Guide d’utilisation — Void Camera 1.49.0

Ce guide concerne Dishonored 2 retail 1.77.9.0. L’interface est en anglais; les noms des commandes ci-dessous reprennent donc les libellés affichés dans le jeu.

## Installation et démarrage

1. Décompressez l’archive dans un dossier. Gardez `VoidCameraInjector.exe` et `VoidCamera.dll` côte à côte.
2. Lancez Dishonored 2 et attendez le menu principal.
3. Lancez `VoidCameraInjector.exe`. Il vérifie l’exécutable du jeu et injecte la DLL.
4. Chargez votre partie. Appuyez sur **F10** pour afficher ou masquer le panneau.

N’injectez pas une deuxième caméra en même temps. Si l’injecteur signale qu’un hook est déjà occupé, fermez complètement le jeu puis relancez-le avant de réessayer. Windows décharge la DLL quand le processus du jeu se termine.

## Caméra

Dans l’onglet **Camera**, choisissez **Keyboard** ou **Gamepad** dans **Control device**, puis activez **Free camera (Insert)**. Le choix du contrôleur et la vitesse de rotation sont conservés dans `%LOCALAPPDATA%\VoidCamera\settings.ini`.

Masquez le panneau avec **F10** pour reprendre le contrôle du jeu. Les commandes clavier sont lues lorsque la fenêtre du jeu est au premier plan. Relâchez les touches avant d’activer la caméra.

| Action | Clavier | Gamepad XInput |
|---|---|---|
| Avancer / reculer | Numpad 8 / 5 | Stick gauche haut / bas |
| Gauche / droite | Numpad 4 / 6 | Stick gauche gauche / droite |
| Monter / descendre | Numpad 9 / 7 | Gâchettes gauche / droite |
| Regarder | Flèches | Stick droit |
| Roll | Numpad 1 / 3 | D-pad gauche / droite |
| Réinitialiser le roll | Numpad 2 | — |
| Modifier le FOV vertical | Numpad + / - | D-pad haut / bas |
| Accélérer / ralentir | Shift / Ctrl | A / X |
| Réinitialiser le FOV | — | B |

**Alt** ralentit encore davantage les déplacements et rotations. Les modificateurs clavier sont Shift ×5, Ctrl ×0,1 et Alt ×0,01. La vitesse de rotation **PYR rotation speed (deg/s)** est réglable dans le panneau; la vitesse de déplacement est indépendante.

**Lock camera (Home)** fige la pose courante sans désactiver la caméra. Appuyez de nouveau sur Home pour la déverrouiller. Vous pouvez verrouiller la caméra avant de redimensionner la fenêtre avec SRWE.

Le panneau affiche la position XYZ, le Pitch/Yaw/Roll et le FOV vertical. Les valeurs peuvent être saisies directement. Le FOV horizontal est recalculé à partir du ratio d’image actif; le FOV vertical reste inchangé, y compris en portrait et en ultrawide.

## Pause et interface du jeu

- **Time stop (Numpad 0)** ou **Numpad 0** arrête ou reprend la simulation. La caméra reste contrôlable pendant l’arrêt du temps.
- **Hide game UI + markers + subtitles (Delete)** ou **Delete** masque ou restaure le HUD, les marqueurs du monde et les sous-titres de cutscene lorsque leur contrôle est disponible.
- Les bandes noires et l’indication **Skip** des cutscenes ne sont pas gérées par cette caméra.
- La caméra désactive automatiquement la pause du jeu lors d’une perte de focus, afin qu’il continue à tourner pendant l’utilisation du panneau.

## Camera path

1. Activez **Free camera** et placez la caméra au début du plan.
2. Dans **Camera path**, choisissez **Add current pose** pour créer un nœud.
3. Déplacez la caméra et ajoutez les nœuds suivants. Il faut au moins deux nœuds pour lancer la lecture.
4. Sélectionnez un nœud pour régler **Seconds to next node**. Cette durée définit le temps du segment qui part de ce nœud vers le suivant. La durée du dernier nœud n’ajoute pas de segment, sauf si **Loop path** est activé.
5. Activez **Smooth interpolation** si vous souhaitez une trajectoire lissée, puis choisissez **Play from start**.

Les commandes de l’onglet permettent aussi d’insérer un nœud après la sélection, déplacer, mettre à jour, supprimer ou rejoindre un nœud, arrêter la lecture, enregistrer ou charger un trajet. Les trajets sont conservés dans `%LOCALAPPDATA%\VoidCamera\Dishonored2_CameraPath.vcp`.

Panneau masqué, les raccourcis de trajet restent disponibles : **F6** ajoute un nœud, **Shift+F6** l’insère après le nœud sélectionné, **F7** supprime le nœud sélectionné, **Ctrl+Page Up/Down** le déplace, **F8** lance la lecture depuis le début et **F9** l’arrête. Sélectionnez le nœud dans l’onglet Camera path avant de masquer le panneau pour utiliser les commandes qui agissent sur la sélection.

## Post-process, Environment et LOD

Les contrôles agissent sur les variables retail reconnues par cette version du jeu. Un contrôle grisé n’a pas été résolu et ne peut pas être modifié. **Restore initial render settings** restaure les valeurs capturées à l’injection.

- **Post-process** : exposition, bloom, lens flares, color grading, profondeur de champ, flou de mouvement et tone mapping. Les réglages DOF natifs du jeu sont distincts de l’IGCS DOF.
- **Environment** : fog, god rays, fog light scattering et sky. Ces options agissent sur l’atmosphère de la scène.
- **LOD** : **World LOD detail scale** contrôle le détail du monde statique; une valeur plus basse favorise davantage de détail. **Model shadow distance** règle la distance de culling des ombres des modèles. Ce panneau ne fournit pas un réglage général vérifié de la distance d’apparition de tous les objets.

## Résolutions personnalisées avec SRWE

La tab Hotsampling n’est pas incluse dans la 1.49.0. Utilisez SRWE pour redimensionner la zone cliente du jeu :

1. Mettez Dishonored 2 en mode **Windowed**.
2. Réglez la largeur et la hauteur de la zone cliente dans SRWE, puis appliquez la taille.
3. Si vous voulez garder la caméra immobile pendant le changement, verrouillez-la avec **Home** avant le redimensionnement.
4. Déverrouillez-la ensuite si vous souhaitez reprendre les mouvements.

Void Camera suit le ratio de rendu et recalcule le FOV horizontal. La taille personnalisée est gérée par SRWE; la caméra ne modifie pas le réglage interne de résolution du jeu.

## IGCS DOF

Void Camera se connecte directement à IGCS Connector via ReShade. Il faut ReShade avec support des add-ons, IGCS Connector et les fichiers de shader et textures IGCS DOF. Installez les fichiers dans les dossiers correspondants de ReShade, activez la technique `IgcsDOF`, puis démarrez la caméra avant la session DOF. L’état du connecteur est indiqué dans les onglets Camera et Post-process.

## Dépannage rapide

- **Injector: Unsupported Dishonored 2 executable** : cette version ne reconnaît que l’exécutable retail vérifié. N’injectez pas dans un autre exécutable ou une autre version.
- **Hook already occupied** : fermez le jeu complètement et redémarrez-le pour libérer un hook de caméra laissé par une autre caméra ou une session précédente.
- **Camera not detected** : attendez que la partie soit chargée et que la vue du jeu soit active.
- **Contrôles inactifs** : vérifiez que la caméra est déverrouillée, que le jeu est au premier plan et que le périphérique voulu est sélectionné.
- **Le jeu se met en pause quand vous changez de fenêtre** : vérifiez que l’injecteur a terminé sans erreur; le réglage focus-loss est appliqué lors de l’injection.

Pour les bandes noires et **Skip** des cutscenes, utilisez une configuration Shader Toggler distincte.
