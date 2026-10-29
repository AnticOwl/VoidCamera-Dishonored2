# Dishonored 2 retail UI CVar correspondences

Build: Dishonored 2 retail 1.77.9.0, image SHA256 `b671f8249422862bf557d00aab2f4bb671ed4515cc10ad8a8a9decad84516180`.

These are runtime correspondences resolved from CVar name pointers in the live process image. They were not taken from the debug-build CVar dump. At each object RVA, the name pointer is at `+0x30`, integer cache at `+0x28`, and float cache at `+0x2C`. The runtime scan validated the name string at each object.

| CVar | Object RVA | Runtime int / float observed in process scan |
|---|---:|---:|
| `g_showHud` | `0x39A3610` | `1 / 1` |
| `g_showGuis` | `0x39A3810` | `1 / 1` |
| `ark_show_ui` | `0x398EBB0` | `0 / 0` |
| `ark_hud_enable_keyhole` | `0x397F2A0` | `1 / 1` |
| `ark_ui_crosshair_opacity` | `0x398A000` | `1 / 1` |
| `ark_ui_show_awareness_markers` | `0x3985BC0` | `1 / 1` |
| `ark_ui_show_contextual_icons` | `0x397D380` | `1 / 1` |
| `ark_ui_show_firelock_markers` | `0x3995FC0` | `1 / 1` |
| `ark_ui_show_grenade_cooking_gauge` | `0x397F220` | `1 / 1` |
| `ark_ui_show_grenade_markers` | `0x397F1A0` | `1 / 1` |
| `ark_ui_show_heart_markers` | `0x3989C20` | `1 / 1` |
| `ark_ui_show_information_messages` | `0x397D0A0` | `1 / 1` |
| `ark_ui_show_interactions` | `0x397D300` | `1 / 1` |
| `ark_ui_show_location_discovery` | `0x3989840` | `1 / 1` |
| `ark_ui_show_master_hud` | `0x398EC30` | `2 / 2` |
| `ark_ui_show_notifications` | `0x397F410` | `1 / 1` |
| `ark_ui_show_objective_markers` | `0x3989BA0` | `1 / 1` |
| `ark_ui_show_objective_notifications` | `0x3989AA0` | `1 / 1` |
| `ark_ui_show_pickup_log` | `0x398A080` | `1 / 1` |
| `ark_ui_show_shortcuts` | `0x397F680` | `1 / 1` |
| `ark_ui_show_snap_reaction_gauge` | `0x397D410` | `1 / 1` |
| `ark_ui_show_target_notifications` | `0x3989B20` | `1 / 1` |
| `ark_ui_show_tutorial_messages` | `0x3981ED0` | `1 / 1` |

The screenshot with the fixed HUD hidden still showed floating objective and awareness markers. The injected 0.8 process had `ark_show_ui=0`, but the Arkane marker CVar values remained enabled. Version 0.9 resolves the table above by exact name, saves each integer/float value, sets the relevant UI visibility values to zero, and restores the saved values when the toggle is released. `ark_ui_show_master_hud` has runtime value 2, so the zero/restore behavior still needs visual confirmation for that entry.

Other retail strings under `ark_ui_*` were found but did not resolve to a live static CVar object in this scan, or are debug/update settings rather than visibility controls. They are not written by the tool.

## Hide-marker regression review, 2026-10-26

The user reports that Void Camera 1.17 leaves floating world markers visible
while `Hide game UI + markers` is checked. Source comparison found no HUD logic
change between the 1.16 release and the 1.17 working tree; 1.17 only changed
the LOD slider minimum/help text and version labels. The current `hudToggle()`
zeros the CVar integer and float caches, but unlike the render CVar writer it
does not update each CVar's inline current-value string at object `+0x10`.
Because the render controls required updating this string to affect the game's
effective value, this is the likely reason marker visibility is not reliably
being suppressed. The working fix now saves, writes `"0"` to, and restores the
inline string alongside both numeric caches. Visual confirmation in the game
is still required; the user's screenshot shows the failure but cannot establish
whether the game rewrites those CVars after the initial toggle.

## Cutscene skip prompt report, 2026-10-28

The user reports that the cutscene `Skip` prompt remains visible after the HUD
toggle. The extracted retail CVar registration text and the runtime-verified UI
CVar table contain no verified cutscene/skip-specific visibility CVar. The
current toggle covers `g_showHud`, `g_showGuis`, `ark_show_ui`, and the verified
`ark_ui_show_*` variables, but that set is not sufficient for the cutscene
prompt. Do not add a guessed CVar or claim this fixed. Capture the relevant CVar
and UI changes while entering a cutscene, or trace the cutscene UI render path,
before extending the toggle.

## Retail cutscene UI and subtitles static trace, 2026-10-28

Static analysis of the supported retail 1.77.9.0 executable found a real
subtitle-level control, but it is a console command rather than a normal
`ark_*` CVar:

| Name | Type | Evidence | Behavior |
|---|---|---|---|
| `arkIggySubtitleLevel` | Console command | Registered at RVA `0x1352cc0`; callback RVA `0x0035720` | Accepts `0`, `1`, or `2`; description says `0 = none`, `1 = main only`, `2 = all`. |
| `s_arkIggySubtitleInitialDisplayLevel` | CVar | Registered at RVA `0x1352df0` | Sets subtitle display level at application initialization; it is not the runtime toggle. |
| `r_skipGuis` | Render CVar | Retail 1.77.9.0 CVar list, object RVA `0x3598D20` | Skips rendering all GUIs. Retail runtime test did not resolve/use it; excluded from release. |
| `r_skipInGameGuis` | Render CVar | Retail 1.77.9.0 CVar list, object RVA `0x3598DA0` | Skips rendering in-game GUIs. Retail runtime test did not resolve/use it; excluded from release. |
| `r_skipIggyFullscreen` | Render CVar | Retail 1.77.9.0 CVar list, object RVA `0x3598FA0` | Skips Iggy full-screen rendering. Retail runtime test did not resolve/use it; excluded from release. |

The command callback calls a runtime setter at RVA `0x0037c40`. The setter
writes the level at offset `+0x1096d0` in the subtitle-manager subobject passed
at `PTR_DAT_142274270 + 0x858`, then performs subtitle visibility/update work.
The static singleton slot is at RVA `0x02274270` and points to RVA `0x02436d40`.
This gives a concrete runtime path to test. Resolve the singleton under ASLR
and invoke the setter on a game-safe thread; do not write only the integer
field, because the setter also updates subtitle state. The command has not yet
been tested live, and its current level still needs to be preserved/restored
around the camera HUD toggle.

The cinematic Skip UI is a Flash/Iggy movie object named `_skipCine`. Its
cinematic UI loader binds `_C_Show`, `_C_Hide`, and input callbacks to it, and
registers `OnSkipCinematicConfirmed` / `OnSkipCinematicDisabled`. The black-bar
element is named `_blackLettering` in the
`arkGameLogicStatePause::OnFlashLoaded` movie bindings. No direct runtime Iggy
object handle was established.

The user's `ShaderToggler.ini` provides hashes that can help identify the two
pixel shaders used for the cinematic Skip prompt and black bars:

| Pixel shader CRC32 | Decimal hash from INI | Role |
|---|---:|---|
| `0xE3BB1976` | `3820689782` | Member of shared Skip / letterbox group |
| `0x28E5E21A` | `686154266` | Member of shared Skip / letterbox group |

The INI groups both hashes together, so it does not identify which individual
hash draws the prompt versus the bars. Treat the hashes as reverse-engineering
evidence only; Void Camera does not depend on Shader Toggler. A first D3D11
prototype only observed shaders created after injection and reported 0/2 in
the user's running game, so that approach was removed. The native GUI-skip CVar
attempt also failed to affect the cutscene elements and is excluded from release.

## 1.49-test implementation notes, 2026-10-28

The local 1.49-test candidate adds two partial runtime controls:

- `renderView_t::radialBlurEnabled` is the verified one-byte field at `+0xB91`. The render hook reads its current value and applies an explicit UI override each time it captures a render view. The Post-process restore button clears the override. This does not expose `radialBlurStrength`.
- The HUD toggle queues `arkIggySubtitleLevel` changes to the hooked game-window message handler. It calls the retail setter at RVA `0x37C40` with the validated global subtitle manager subobject (`*(*(base+0x2274270)) + 0x858`), sets level 0 while hidden, and restores the level read immediately before hiding.

The 1.49-test subtitle path was retained for 1.48.1: the Camera tab reports runtime setter readiness, and Delete captures the current subtitle level, sets it to 0, then restores it on the next toggle. This is compiled but still needs live confirmation. The radial-blur override and GUI CVar experiments were removed from the release. Skip and letterbox bars remain Shader Toggler effects; Void Camera does not depend on Shader Toggler. The Gist's CVar offsets are debug-build offsets and must not be used as retail addresses. The `radialBlurStrength` name is present as a fullscreen-quad shader/render parameter; there is still no confirmed writable storage mapping for a slider.
