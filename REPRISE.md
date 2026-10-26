# Reprise du 2026-10-23 — Dishonored 2 / Death of the Outsider

Source lue : https://chatgpt.com/share/6adbb8ba-26fc-83eb-91bf-e956ac103663 (Moderniser les caméras).
Les messages de l'ancien assistant constituent des recherches à vérifier, pas des preuves de tests en jeu.

## Demande utilisateur
- Dishonored 2 en premier, Death of the Outsider ensuite.
- DLL avec interface ImGui en jeu + injecteur EXE.
- Camera : XYZ, Pitch/Yaw/Roll, FOV, Camera Speed.
- Post Process natif : exposure, bloom, lens flare, saturation, contrast, color grading, etc.
- LOD : distances/far/draw et priorité au forçage LOD (r_forceLOD à rechercher, pas validé en retail).
- Keys : Insert freecam, Delete HUD, Numpad0 pause. Déplacements NP8/5/4/6, hauteur monde NP9/7, flèches PY, NP1/3 roll, NP2 reset roll, NP+/- FOV, Shift/Ctrl/Alt vitesses.
- IGCSConnector/IGCSDOF : connexion et état affichés.
- Hotsampling conservé. Resolution scale retiré explicitement. Pas de capture HQScreenShot intégrée.
- Env retiré explicitement. Lights seulement optionnel si faisable.
- Mesures et livraisons Deus Ex selon AGENTS.md. UI anglaise, décimales avec points.

## État transmis par la conversation
- Prototype CE v0.4 hook de préparation de la vue : fonctionne, erreurs d'axes.
- v0.5 correction lignes/colonnes et déplacements : utilisateur dit « bien mieux », mais flickering intermittent.
- v0.6 proposée : conserve dernière transformation complète pendant publication Lua; pas de NOP global. Tests hors jeu annoncés. Validation visuelle du correctif non présente dans la conversation partagée.
- Ne pas refaire les anciennes recherches d'offsets ni réutiliser les offsets GitHub invalidés.
- Valeurs retail corrigées selon l'ancien travail : g_freeCam EXE+39A17B8, g_stopTime +39A0E38, g_freezeTime +39A6108. À comparer à l'artefact v0.6 avant usage.
- EXE SHA256 du rapport : b671f8249422862bf557d00aab2f4bb671ed4515cc10ad8a8a9decad84516180.

## Artefacts retrouvés
- Rapport statique copié dans research (certaines pistes d'offsets y sont historiques et invalidées ensuite).
- Sources IGCS : F:/CE Projects/InjectableGenericCameraSystem-master/Cameras/Dishonored2.
- Outils IGCS existants : C:/Users/olivi/Documents/VP Tools/Dishonored 2/New 1.0.3.
- Jeu : D:/SteamLibrary/steamapps/common/Dishonored2.
- v0.6 reçue de l'utilisateur et conservée intégralement dans `research/Dishonored2_Camera_v0.6.lua` (aucune modification). SHA256 : B7D8EFEF6AB38B875D968CA465BC07AF36D122AB010977363FAC458002389E25.
- Le 2026-10-23, SHA256 de l'exécutable installé vérifié : correspond exactement au binaire ciblé par la v0.6 et au rapport. Cette vérification sur disque ne vaut pas validation du hook en mémoire ou du rendu.
- Le 2026-10-23, la 0.2 ajoute le time stop via `g_stopTime` et `g_freezeTime`. La résolution en mémoire des deux noms et les commandes pause/reprise ont été vérifiées sur PID 36496 à l'écran d'accueil. L'effet sur la simulation en partie reste à valider visuellement.

## Base de portage v0.6
- Hook de préparation de vue : RVA 0x375711, original `44 38 25 84 FB 62 03`. Refuser un hook déjà occupé.
- Le prototype conserve une pose complète de 14 floats (FOV horizontal/vertical, XYZ, matrice 3×3) quand la publication Lua est incomplète, au lieu de laisser réapparaître la caméra joueur.
- Axes natifs : Forward, Left, Up ; transposition aux frontières avec le quaternion caméra-vers-monde. Conserver cette correction de v0.5.
- Les offsets CVar corrigés ci-dessus sont cohérents avec l'artefact reçu ; celui-ci les lit uniquement et n'implémente pas le HUD/post-process. La version native 0.2 écrit `g_stopTime` et `g_freezeTime` après vérification runtime des noms.
- Le prototype CE ne bloque pas les entrées du joueur et n'est pas une livraison ImGui. Le portage DLL devra traiter séparément le routage des entrées.
- Aucun script exécuté ni hook installé lors de l'import. Validation visuelle v0.6 encore non établie.

## Portage natif 0.1
- Sources C++/MASM créées à partir du Lua v0.6 ; ImGui DX11 et MinHook locaux réutilisés.
- DLL + injecteur compilés dans bin/v0.1/Release.
- Ancien hook CE détecté puis session redémarrée. Injection native réussie dans PID 30796.
- Panneau en jeu, changements XYZ/PYR/FOV et masquage End observés.
- Voir research/Native_0.1_tests.md pour les limites et essais restant à valider.
- Cette version est un jalon caméra : ne pas présenter HUD/PP/LOD/IGCSConnector/hotsampling/DOTO comme implémentés.
- Native 0.2 ajoute le time stop : la détection des CVars et les changements d'état pause/reprise sont vérifiés au menu, mais l'effet de gel du monde reste à tester en partie.

## Native 0.3 — hotsampling et mode de contrôle
- Home active/désactive `cameraLocked`; la pose continue d'être publiée pour tenir l'angle pendant que SRWE redimensionne le jeu. L'aspect est actualisé depuis la vue capturée.
- Le panneau sélectionne Keyboard ou Gamepad. La manette XInput 0 est mappée : stick gauche translation, stick droit orientation, gâchettes hauteur, D-pad roll/FOV, A rapide, X lent, B reset FOV.
- Hotsampling documenté pour Dishonored 2 retail en mode Windowed avec SRWE; pas de redimensionnement automatisé, résolution scale, HQScreenShot ni prise en charge Death of the Outsider.
- Build Release et paquet versionné : `bin/v0.3/Release`, `dist/VoidCamera_0.3.zip`. Les essais clavier/manette et le hotsampling en partie n'ont pas été faits, à la demande de l'utilisateur qui testera une fois l'étape terminée.

## Native 0.4 — HUD, focus loss et commandes caméra
- `g_showHud` (RVA CVar `0x3BFC270`) et `g_showGuis` (`0x3BFC470`) sont vérifiés par nom runtime. Delete et le panneau basculent les deux valeurs en conservant et restaurant les valeurs précédentes.
- La bascule focus loss réutilise la signature IGCS `83 3D ?? ?? ?? ?? 00 74 4E 48 8B 0D ?? ?? ?? ?? 48 8B 01` pour retrouver l'adresse RIP-relative de `win_pauseOnLossOfFocus`. Une occurrence unique a été trouvée dans le `.text` retail (RVA `0xEA34C`), ciblant le `.data` à RVA `0x3139408`. Activée, la bascule met la valeur à 0; désactivée, elle restaure la valeur initiale. Si la signature est absente ou ambiguë, le contrôle reste désactivé.
- Commandes clavier déjà traitées par le moteur : Numpad 8/5 avancer/reculer, 4/6 gauche/droite, 9/7 hauteur, flèches orientation, Numpad 1/3 roll, 2 reset roll, +/- FOV. La sélection Gamepad conserve les sticks, gâchettes et D-pad XInput existants. Masquer le panneau avec End pour utiliser les entrées.
- Build Release réussi avec VS 2022/MSVC 14.44.35207; paquet : `bin/v0.4/Release`, `dist/VoidCamera_0.4.zip`. Fonctions HUD/focus loss compilées mais pas encore validées visuellement en jeu; hotsampling reste à reprendre ensuite.

## Native 0.5 — accès souris et focus loss automatique
- Le curseur ImGui logiciel est désormais dessiné quand le panneau est visible, pour cliquer sur la case HUD et les contrôles du panneau.
- `win_pauseOnLossOfFocus` est forcé à 0 dès l'injection, comme dans IGCS. Le contrôle n'est plus une case dans l'UI; un statut confirme que la résolution a réussi.
- La cible du CVar focus est manipulée sur un seul octet, comme l'écriture IGCS, au lieu d'écraser 4 octets.
- L'origine des offsets HUD a été recoupée avec le CVar dump 1.77.9.0 : `g_showHud` RVA `0x3BFC270`, `g_showGuis` RVA `0x3BFC470`; leurs valeurs sont à +0x28.
- Build Release réussi avec VS 2022/MSVC 14.44.35207, paquet `bin/v0.5/Release`, `dist/VoidCamera_0.5.zip`. Affichage curseur, clic HUD et maintien actif hors focus encore à valider par l'utilisateur en jeu.

## Native 0.6 — hotsampling FOV / diagnostic correction
- Found and corrected the cursor regression: the render loop forced `MouseDrawCursor=false` every frame, overriding initialization. It now remains enabled while drawing the panel.
- The vertical FOV remains the camera pose control. Horizontal FOV is recalculated on each publication from captured live view width/height with `2*atan(tan(VFOV/2)*aspect)`. Aspect and H/V FOV are now visible in the panel and injector status to inspect 16:9 versus 21:9 resize behavior.
- Focus-loss resolution now checks the IGCS-derived target against the supported build's `.data` RVA `0x3139408`, verifies that it is writable committed memory, and only writes a boolean byte value. This avoids touching an unexpected signature target.
- The hotsampling flow continues to use SRWE while Windowed and the Home pose lock; no automated resize/capture was added. No live game test was run because the previous release was reported to crash; locally available Windows Application Error events and CrashDumps contained no Dishonored2 report to identify its cause. User acceptance test remains necessary before claiming the crash fixed.

## Native 0.7 — correspondances retail HUD / focus
- L'utilisateur a confirmé que l'ancienne liste CVar provient d'une debug build; ne pas utiliser ses objets `g_showHud` / `g_showGuis` en retail.
- Dans le processus retail en cours (Dishonored2.exe, image SHA256 correspondant au build supporté), les chaînes `.rdata` et leurs références live conduisent à `g_showHud`: nom RVA `0x1C8D010`, objet RVA `0x39A3610`, valeur int RVA `0x39A3638` (valeur observée 1); `g_showGuis`: nom RVA `0x1C8D178`, objet RVA `0x39A3810`, valeur int RVA `0x39A3838` (valeur observée 1). Résolution validée par lecture seule de la mémoire du processus; vérifier les noms avant écriture.
- Les deux états `HUD=0` et `focusLoss=0` vus après injection de la 0.6 provenaient de contrôles non résolus. Pour le focus, le scanner comparait par erreur les quatre octets wildcard du second déplacement RIP à zéro. Le pattern IGCS doit ignorer les bytes 12..15, puis résoudre le premier déplacement RIP; la cible est gardée sous contrôle par l'RVA retail attendue `0x3139408` et vérification d'une page commise modifiable et d'une valeur booléenne.
- La 0.6 avait en outre forcé `MouseDrawCursor=false` à chaque image; corrigé en 0.7 et conservé vrai à chaque frame lorsque l'UI est rendue. Son status reflète l'état du curseur/UI par le rendu, tandis que l'injecteur affiche les états prêts HUD/focus pour diagnostic.
- Hotsampling en attente de validation manuelle : VFOV maintenu, HFOV recalculé à l'aspect courant; terminer la validation HUD/focus avant d'entamer le reste du mapping CVar destiné au LOD.
- Référence utilisateur pour le futur travail LOD (vidéo du 2026-10-24) : `C:/Users/olivi/AppData/Local/Packages/Microsoft.ScreenSketch_8wekyb3d8bbwe/TempState/Recordings/20261024-0541-02.1080750.mp4`. L'utilisateur signale des ombres et des objets qui changent de forme au loin; reprendre ces symptômes lors de l'analyse LOD. Le fichier temporaire n'était pas accessible depuis le shell à cette session, donc ces observations sont consignées comme signalement utilisateur, sans inspection visuelle indépendante.

## Native 0.8 — HUD retail et mode d'entrée mémorisé
- Inspection de la DLL Frans v1.0.2 (SHA256 `77DD4A309D046C0992F8A066303E8E8DF90F251F6CB022730134C664DEFAA4E8`), de son Readme et du code IGCS local : elle ne contient pas de bascule HUD; les notes IGCS disent explicitement que l'HUD reste visible avec la caméra Ansel sans overlay. Elle met notamment `win_pauseOnLossOfFocus` à 0.
- La CVar Arkane `ark_show_ui` est présente dans l'EXE retail et son objet a été résolu dans le processus retail actif via la référence au nom : nom `.rdata` RVA `0x1C60548`, objet RVA `0x398EBB0`, valeur int `+0x28` observée à 1. L'ancienne bascule n'écrivait que les caches int de `g_showHud`/`g_showGuis`; 0.8 met à jour les trois CVars et leurs caches int/float puis restaure les valeurs originales.
- Le choix Keyboard/Gamepad est maintenant accessible avant d'activer la caméra, chargé et enregistré dans `%LOCALAPPDATA%\\VoidCamera\\settings.ini`.
- Le masquage HUD par `ark_show_ui` et le stockage clavier/manette sont compilés en 0.8; validation visuelle en jeu encore requise.

## Native 0.9 — marqueurs UI Arkane
- Les captures envoyées montrent que `ark_show_ui=0` enlève le HUD fixe mais laisse les marqueurs flottants (Bonecharm, awareness, icônes contextuelles). Dans le processus 0.8 en partie, ces CVars de marqueurs restaient à 1.
- Correspondances retail résolues par leur pointeur de nom en mémoire vive : `ark_ui_show_objective_markers`, `ark_ui_show_awareness_markers`, `ark_ui_show_contextual_icons` et les CVars de visibilité/gauges/notifications; table complète dans `research/Dishonored2_UI_Cvars_Retail.md`. Inclure uniquement les objets résolus et comparer le nom avant écriture, jamais reprendre la liste de la debug build.
- La 0.9 met les CVar de visibilité Arkane et les caches int/float à zéro avec le HUD, puis restaure leurs valeurs exactes au rétablissement. `ark_ui_show_master_hud` vaut 2 en situation normale et devra être confirmé visuellement après la mise à zéro.
# Native 1.13 — direct IGCS DOF adapter

- Added the in-process Connector handshake: VoidCamera resolves the addon's
  exported connection and buffer functions, then writes camera status, FOV,
  position, quaternion, rotation basis and pitch/yaw/roll directly into its
  `CameraToolsData` buffer. No relay process, relay DLL, or shared-memory hop.
- The ABI layout is guarded by compile-time offset and size checks against the
  local Connector source. Verified the built DLL exports all four required
  `IGCS_*` camera-tools symbols; screenshot and panorama features remain
  unsupported and the session start callback returns the Connector's
  `Error_CameraFeatureNotAvailable` code.
- ReShade is already present per user. Runtime connection and DOF image still
  need an in-game check with the Connector addon and `IgcsDOF` technique.
- Build: `build-v13-vs17`, Release; package: `dist/VoidCamera_1.13.zip`.
- Detailed protocol and setup: `research/Dishonored2_IGCSDOF.md`.

## Native 1.14 — IGCS DOF multi-shot session

- Compared the session lifecycle in DXMD, Watch Dogs, Fallout 4 and Skyrim
  implementations already in the workspace. The common working pattern is to
  validate and snapshot the live camera, move each sample along the camera's
  right/up axes, and restore the snapshot at session end.
- Replaced the previous unsupported-session return code and empty multi-shot
  callback. VoidCamera now requires the live camera, rejects duplicate or
  non-multishot sessions, holds the camera pose against local input during
  sampling, publishes each sample directly to the IGCS Connector buffer, and
  restores position, rotation and FOV after the session.
- Panorama stitching remains unsupported. The user confirmed IGCS DOF works
  in game on 2026-10-24.
- Build: `build-v14-vs17`, Release; package: `bin/v1.14/Release`,
  `dist/VoidCamera_1.14.zip`.

## Native 1.15 — area-light controls and diagnostics

- Added a dedicated Lights tab for verified retail area-light diffuse/specular
  factors, shadow minimum/maximum thresholds, area-light pool visualization,
  and the `r_showLights` diagnostic modes. Every UI setting has a help marker;
  CVar names are resolved and checked at runtime before they are enabled.
- Moved the area-light factors out of Environment, which now covers atmosphere.
- The supported retail executable contains `popLight` and `clearLights` command
  strings with removal/reset descriptions. Searches for `addLight`, `pushLight`,
  `spawnLight`, `createLight`, and `ListRenderLights` found no matches. This does
  not establish a retail creation interface; spawning or editing individual
  lights is still an open research item.
- Release build succeeded with Visual Studio 2022/MSVC 14.44.35207 in
  `build-release-lights-v115`; package: `bin/v1.15/Release`,
  `dist/VoidCamera_1.15.zip`.
- Read-only runtime inspection of the active retail process reconfirmed the
  exact CVar names and initial values for both area-light thresholds and both
  diagnostics. VoidCamera 1.14 is already loaded in that game process, so the
  new UI/effects have not been visually tested; restart before injecting 1.15.

## Native 1.16 — render controls and PP Cheat Engine table

- Removed the Lights tab from the in-game UI at the user's request. Area-light
  CVar research remains documented, but those controls are no longer surfaced.
- Expanded `tools/Dishonored2_PostFX_Retail_1.77.9.0.CT` with live-registered
  fog blur, god-rays blur, gamma, transparency/particle, exposure convergence,
  and underwater fog/god-rays CVars. Each entry is annotated as experimental
  where its visual effect or useful range has not been tested; writes validate
  the exact retail CVar name and synchronize text, integer, and float fields.
- FOV reminder: Numpad `+`/`-` adjusts vertical FOV while free camera is enabled
  and the panel is hidden. Horizontal FOV is derived from live aspect ratio.
- User clarified the new visual issue concerns LOD relative to FOV, not changing
  the camera FOV itself. The live retail binary has no separate `lodAtFov` CVar
  string; `r_lodZoomMultiplicator` is the documented static-world LOD zoom
  multiplier and is the likely setting to revisit alongside the reported chair
  appearance. Do not claim that relationship is confirmed without visual testing.
- The attached 2026-10-25 video was not readable from its protected temporary
  recording path in this session; chair visibility/pop-in still needs visual
  attribution before changing the camera or culling behavior.
- Release build succeeded with Visual Studio 2022/MSVC 14.44.35207 in
  `build-v16-vs17`; package: `bin/v1.16/Release`, `dist/VoidCamera_1.16.zip`.

