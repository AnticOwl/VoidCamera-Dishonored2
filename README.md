# Void Camera 1.16 — Dishonored 2 camera, render controls and time stop

Native x64 camera tool for Dishonored 2 retail 1.77.9.0.

## Run
1. Start Dishonored 2 and stay at the main menu.
2. Run `VoidCameraInjector.exe` beside `VoidCamera.dll`; set shadow-buffer options before loading a save so the level can build them with the new values.
3. Load a save.
4. End shows/hides the in-game panel and mouse cursor. Insert enables/disables the camera. Delete hides/restores the HUD.
5. Use the Post-process, Environment and LOD tabs to change render settings. A control is enabled only after its retail CVar name is verified. Restore initial render settings returns exposed values to their state at injection.
6. Hide the panel to use movement keys; enable Num Lock.
7. Choose Keyboard or Gamepad in the panel before enabling the camera; the choice is saved in `%LOCALAPPDATA%\VoidCamera\settings.ini`. Press Home to lock/unlock camera input before switching focus or resizing.

Do not enable another camera hook simultaneously. An occupied hook is rejected;
restart the game to clear a leftover CE prototype. The DLL remains loaded until
game exit. A different DLL build requires a game restart.

## Controls
- Numpad 8/5: forward/back; 4/6: left/right; 9/7: world height.
- Arrows: pitch/yaw; Numpad 1/3: roll; Numpad 2: reset roll.
- Numpad +/-: vertical FOV.
- Home: lock/unlock camera input while preserving the current pose.
- Gamepad (XInput, first controller): left stick moves, right stick looks, triggers change height, D-pad adjusts roll/FOV, A is faster, X is slower, B resets FOV.
- Numpad 0 or the panel checkbox: toggle time stop.
- Delete or the panel checkbox: hide/restore the HUD, Arkane UI elements and world markers (objectives, awareness, interactions, contextual icons, notices and related gauges).
- The DLL disables the game's pause-on-focus-loss behavior automatically when injected, so the game keeps running while you use another window.
- Camera movement and pose publishing remain enabled during time stop; hide the panel to use the selected control device.
- Shift: 5× speed; Ctrl: 0.1× speed; Alt: 0.01× speed.
- Panel: Alt + drag for fine adjustments; Ctrl + click for numeric input.

Hotsampling uses the game's support for custom aspect ratios in Windowed mode: set the game to Windowed, enable the camera, lock its pose with Home, then resize the game window with SRWE and capture with your preferred screenshot tool. The camera keeps vertical FOV and recalculates horizontal FOV as `2 × atan(tan(verticalFOV / 2) × aspect)` from the live view dimensions. The panel displays the live aspect ratio and horizontal/vertical FOV so you can confirm the 16:9 to 21:9 change. Restore the original window size before unlocking. Dishonored 2: Death of the Outsider does not support this hotsampling workflow. Resolution scaling and integrated HQScreenShot capture are not included.

Panel uses the Deus Ex measurements: 13 px font, 620×530 initial size,
WindowPadding 8×8, FramePadding 4×3, ItemSpacing 8×4, roundings 4/2.
The palette is muted grey/brass. Numeric formatting uses decimal points.

## Scope and limitations
Implemented: in-game DX11 ImGui panel, XYZ, Pitch/Yaw/Roll, vertical FOV,
camera speed, keyboard and XInput gamepad movement with a selectable control device,
Home pose lock, coherent last-pose cache from CE v0.6, time stop through the retail
`g_stopTime` and `g_freezeTime` CVar integer values, HUD/game GUI visibility through
`ark_show_ui`, `g_showHud`, `g_showGuis`, and the live-resolved `ark_ui_show_*` visibility CVars (integer and float caches); focus-loss behavior is enabled automatically at
injection. The DLL verifies each HUD CVar name in the supported executable
before enabling the control and restores saved values when a toggle is released.
The world-simulation effect was previously tested by the user in a loaded save.

Post-process: auto exposure, exposure adaptation freeze, additive custom EV,
bloom, lens flares, color grading, depth of field (focal distance, focus range,
circle of confusion), camera motion blur and tone mapping. Environment: fog, god
rays, fog light scattering and sky. Render CVar names are checked against the
supported retail image at runtime. Current values are saved at injection, can be
adjusted live, and can be restored with the tab button.

LOD: world LOD detail scale (lower retains more static-mesh detail) and model shadow culling distance. The retail build has no verified `r_forceLOD`, `lod_debug_force_value` or `r_disableStaticWorldLOD` CVar. Entity thresholds, quality presets, texture-mip overrides, shadow-buffer resolution and cascade controls are omitted from the panel until their visible effects are confirmed.

Not implemented: player input suppression while flying,
automated window resizing for hotsampling, spawning or editing individual lights,
Death of the Outsider support. Camera keys are processed only while the
game is focused and the panel is hidden. Disable before changing levels.

## IGCS Depth of Field
VoidCamera connects directly to the IGCS Connector ReShade addon through its
exported functions and writes the camera pose into the addon's own data buffer.
There is no relay DLL or shared-memory intermediary. The Camera and Post-process
tabs show whether the addon connection is available. This lets IGCS DOF use
VoidCamera. ReShade 6.4+ with add-on support is required,
then install the official Connector addon, shader, and its required textures using
the [IGCS DOF installation guide](https://opm.fransbouma.com/igcsdof.htm).
Keep the Connector addon beside `Dishonored2.exe`. Install the files from its
`Shaders` and `Textures` folders into ReShade's matching folders, and enable
the `IgcsDOF` technique in ReShade's Home tab. Enable VoidCamera before
starting a DOF session.

VoidCamera implements the Connector camera-data bridge and IGCS multi-shot
sessions used by DOF. Start a session with the camera enabled; the camera pose
is saved, each sample moves within the original camera plane, and the pose is
restored when the session ends. Panorama stitching remains unavailable. The
IGCS DOF workflow was confirmed working in game on 2026-10-24.

The injector checks the exact executable SHA256. The DLL also checks PE identity
and the three original camera signatures before installing its hook.

## Build
Run `tools/prepare.py` once with Python (local dependencies are reused from the
DXMD project when absent), then:

```text
cmake -S . -B build -A x64 -T "v143,version=14.44.35207"
cmake --build build --config Release
```

Output: `bin/v1.16/Release`. Distribution: `dist/VoidCamera_1.16`.

## Sources
Camera baseline supplied by the user: `research/Dishonored2_Camera_v0.6.lua`.
`tools/prepare.py` translates its hook syntax to MASM; game offsets are preserved.
For additional, experimental retail render variables to explore in Cheat Engine,
see `tools/Dishonored2_PostFX_Retail_1.77.9.0.CT` and the notes in `research/`.
Dear ImGui and MinHook are included under their respective license files.
