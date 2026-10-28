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

## Native 1.17 — lower static LOD multiplier

- Lowered the `r_lodZoomMultiplicator` UI minimum from `0.01` to `0.001` and
  added three-decimal display. This allows testing finer static-world LOD at
  the same camera view; `0.001` has not been visually tested.
- Static retail disassembly found only registration/forwarding references to
  the CVar object at RVA `0x32FBF20`; no direct reference to its float cache or
  an obvious `r_forceLOD` retail consumer. The exact zoom-driven LOD selection
  path remains unresolved and needs a runtime read trace while the scene zoom
  makes the chair/details appear.

## Native 1.18 — HUD marker CVar synchronization

- Fixed the HUD toggle to synchronize each verified CVar's inline current-value
  string along with its integer and float caches, matching the known retail
  CVar update path. The hide operation still requires visual confirmation in
  game; the 1.17 screenshot showed markers visible with the toggle checked.
- Release build and package: `bin/v1.18/Release`, `dist/VoidCamera_1.18.zip`.

## Native 1.19 — in-game hotsampling tab

- Added a Hotsampling tab with direct client-width/height entry, common aspect
  presets, current output size, saved restore size, and an apply/restore flow.
  It only resizes a windowed swap chain; if the camera is active it must be
  pose-locked before applying the resize. Window resizing is queued to the game
  window procedure to avoid resizing the swap chain inside Present.
- The tab reports the current client area and aspect; live FOV behavior should
  be confirmed in game at non-16:9 resolutions. This does not add resolution
  scaling or screenshot capture.
- Release build succeeded with Visual Studio 2022/MSVC 14.44.35207; package:
  `bin/v1.19/Release`, `dist/VoidCamera_1.19.zip`. The in-game resize and restore
  flow still needs manual validation with Dishonored 2 in Windowed mode.

## Native 1.20 — hotsampling resize correction

- User reported the 1.19 Hotsampling tab did not resize the game window. Corrected
  client-area sizing to use DPI-aware non-client frame metrics, restore a
  maximized window before resizing, and reconcile the resulting client dimensions
  against the requested size.
- The tab now displays actual client dimensions after applying and reports when
  Windows or the game constrains the requested size instead of showing a false
  success. This build still needs user confirmation in the running game.
- Release package target: `bin/v1.20/Release`, `dist/VoidCamera_1.20.zip`.

## Native 1.21 — hotsampling controls

- Apply is available while the camera is either locked or unlocked; there is no
  automatic camera lock.
- Removed the captured-size Restore button. The Apply button now remains enabled
  for a normal titled game window even if the swap chain reports fullscreen, and
  requests a windowed swap chain during resize if needed. Current size fields can
  be copied back to return to the prior dimensions.
- Package target: `bin/v1.21/Release`, `dist/VoidCamera_1.21.zip`.

## Native 1.22 — oversized hotsampling windows

- The user screenshot from 1.20 showed a windowed swap chain and an active Apply
  button, but applying 4200×1800 left the client area at 2564×1421. This rules
  out the earlier fullscreen-disable explanation for that attempt.
- The game window procedure now raises `WM_GETMINMAXINFO.ptMaxTrackSize` to allow
  client sizes larger than the desktop. Apply works while the camera is locked
  or unlocked; the obsolete restore-size button remains removed.
- Removed the aspect-ratio preset. Width and height are entered directly.
- Package target: `bin/v1.22/Release`, `dist/VoidCamera_1.22.zip`. Oversized
  resizing needs confirmation in game.

## Native 1.23 — Deus Ex camera UI template

- Matched the hotsampling tab to the DXMD Freecam layout: saved successful
  resolutions appear above the size fields, most recent first, and selecting one
  applies it directly. Up to 16 unique resolutions persist beside the DLL in
  `VoidCamera-Hotsampling.ini`.
- `Use default res` fills the fields with the swap-chain render size detected at
  first draw after injection. A size enters history only once the live render
  buffer reports that requested size.
- The overlay toggle and window title now use F10. F10 system-key messages are
  consumed so Windows does not open its system menu.
- Package target: `bin/v1.23/Release`, `dist/VoidCamera_1.23.zip`.

## Native 1.24 — FOV / aspect preservation

- Corrected the camera writeback: retail field `view+0x3A6C` is the aspect ratio,
  not horizontal FOV. `publish()` writes the engine's captured aspect value
  there and keeps vertical FOV in `view+0x3A70`; horizontal FOV remains a
  derived UI value. This prevents a radian FOV value from corrupting projection
  aspect at wide and portrait resolutions.
- Snapshot validation now accepts aspect ratios up to 10:1, including 32:9.
  Hotsampling shows the live camera AR and horizontal/vertical FOV for checking.
- Moved saved resolutions below the target controls, matching the DXMD tab order.
- Package target: `bin/v1.24/Release`, `dist/VoidCamera_1.24.zip`. Visual FOV
  confirmation remains pending.

## Native 1.25 — use Dishonored 2 native aspect

- The user clarified that Deus Ex Camera is a UI template only; each game has its
  own camera/FOV implementation. Switched the live aspect source to Dishonored
  2's captured camera field at `view+0x3A6C` instead of deriving it from the
  registers' width/height values. The hook preserves that native AR value while
  retaining vertical FOV at `view+0x3A70`.
- Package target: `bin/v1.25/Release`, `dist/VoidCamera_1.25.zip`. In-game
  validation is required to confirm the stretch is gone.

## Native 1.26 — sync game AR to live render target

- The 1.25 screenshots showed the render target at 2100×900 (AR 2.333) and
  3000×4000 (portrait AR 0.75), while the camera's native AR field stayed at
  1.2936. Preserving the native field therefore preserved a stale ratio.
- The overlay now publishes the actual DXGI backbuffer width and height to the
  camera loop. Dishonored 2's native aspect field is updated from that live
  buffer ratio before the camera pose/FOV is published. This covers both
  landscape and portrait aspect ratios; vertical FOV stays unchanged.
- Package target: `bin/v1.26/Release`, `dist/VoidCamera_1.26.zip`. The 3000×4000
  portrait shot and a wide shot still need visual confirmation with this build.

## Native 1.27 — aspect writeback experiment (regressed projection)

- Experiment wrote the ratio directly into `view+0x3A6C`, while sourcing the
  ratio from the DXGI backbuffer. The user confirmed the displayed AR was right
  but the projection remained wrong after enabling the camera.
- Package target: `bin/v1.27/Release`, `dist/VoidCamera_1.27.zip`.

## Native 1.28 — restore Dishonored 2 camera projection calculation

- Compared against the known-good 1.16 source. That camera calculates the
  game's first projection input as `2*atan(tan(verticalFov/2)*aspect)`; 1.27
  replaced it with the raw aspect ratio. Restored the original calculation.
- The in-game hotsampling tab still supplies its actual DXGI backbuffer aspect
  to that game-specific calculation, covering landscape, ultrawide, and
  portrait sizes without changing the original vertical FOV behavior.
- Package target: `bin/v1.28/Release`, `dist/VoidCamera_1.28.zip`. Visual
  confirmation: user tested the build and confirmed « OK, maintenant c'est bon ».
  The restored Dishonored 2 projection calculation works with hotsampling.

## Native 1.29 — entity LOD / Spyglass diagnostic build

- Restores the verified retail entity distance controls in the LOD tab:
  `lod_forcevisibledistance`, `lod_dist_level0..3`, and `lod_dist_far`.
- Adds a reversible test preset (`100000 / 100000 / 200000 / 300000 / 400000 /
  500000`) that attempts to keep highest entity detail farther away. This is
  separate from static-world `r_lodZoomMultiplicator`; no exact retail
  `r_forceLOD` control has been confirmed.
- Adds a start/stop CSV logger at
  `%LOCALAPPDATA%\VoidCamera\lod-spyglass-test.csv`, sampling every 250 ms.
  It records ZOOM key Z, MOVE_FORWARD key W, gamepad buttons/sticks/triggers,
  camera position and captured view dimensions, render aspect, static LOD scale,
  and all six entity distance caches. The user can log an approach-to-table
  pass and a separate Spyglass pass to compare distance transitions with input.
- Package target: `bin/v1.29/Release`, `dist/VoidCamera_1.29.zip`. Logging and
  forced entity LOD still need runtime/visual validation.

## Native 1.30 — mesh appearance distance diagnostic

- Runtime read-only inspection of the open retail game confirmed `r_viewDistPreset=5` (Ultra),
  `r_zfar=7000`, and `r_lodZoomMultiplicator=0.001`.
- The user reports the 1.29 entity LOD distance controls and force preset did
  not change the observed mesh appearance range. Added `r_viewDistPreset` and
  `r_zfar` to the LOD tab as diagnostic candidates; the latter is documented
  as far Z clipping, not mesh streaming.
- Replaced the coarse Spyglass log with a 100 ms mesh log and explicit absent / visible
  markers. Marker rows capture camera position, input state, entity/static LOD
  values, view-distance preset, and far clip so an approaching-object event can
  be compared with a fixed-position Spyglass toggle.
- Log path: `%LOCALAPPDATA%\VoidCamera\lod-mesh-test.csv`.
- Package target: `bin/v1.30/Release`, `dist/VoidCamera_1.30.zip`. The user must
  test with the same mesh and mark its state at the observed transition.

