# Native camera 0.1 — 2026-10-23

## Build and process checks
- MSVC 19.44.35229, x64 Release build succeeded.
- Initial process 25540 contained a previous CE-style hook at EXE+375711.
  DLL initialization rejected it without replacing the patch (error 1).
- User restarted the game after the automated launch failed.
- New process 30796: original hook bytes and both secondary signatures matched;
  Ansel-active byte was zero. DLL injection returned ready=1, error=0.

## Observed live results
- In-game DX11 ImGui overlay displayed at the reference 620×530 size.
- Camera activation: enabled=1 and applied counter advanced.
- Baseline: XYZ (-30.81336,192.69846,2.95181),
  PYR (-0.015,-90.039,-0.025), vertical FOV 50.534.
- Command XYZ (-30.81336,192.69846,4.95181), PYR (0,-70,0), FOV 60:
  elevated viewpoint, changed heading and wider view were visible.
- Command PYR (-15,-70,20): upward pitch and tilted horizon were visible.
- End hid the overlay while the selected camera pose remained applied.
- Automated KP_8 pulse did not change coordinates; sustained key behavior remains
  a manual check. Do not claim the movement keys passed from this pulse.
- User asked for a sustained movement / rotation test to assess flicker.

## Still to validate
- Continuous keyboard input, perceived smoothness and absence of transient flashes.
- Camera disable and return to gameplay view.
- UI numeric editing and Alt fine adjustment in actual use.
- Performance has not been measured.

Counters and screenshots demonstrate application of the selected pose;
they do not alone establish complete frame-by-frame flicker suppression.

## Confirmation utilisateur
Le 2026-10-23, l'utilisateur confirme : « la camera fonctionne ». Ne pas refaire les tests caméra déjà validés sans changement ou nouvelle incertitude. Le détail de chaque touche et les performances ne sont pas explicitement évalués par cette confirmation.

## Native 0.2 — time stop implementation
- Added Numpad 0 and panel checkbox controls for time stop.
- Runtime resolves retail CVar objects at `Dishonored2.exe+0x39A0E10` (`g_stopTime`) and `+0x39A60E0` (`g_freezeTime`), checks each name pointer/string, then writes the cached integer at object `+0x28` (`+0x39A0E38` and `+0x39A6108`).
- On activation, saves the previous integers and sets both to 1; on release, restores those previous values. Injector supports `--pause`, `--resume`, and reports `timeStopReady` / `timeStopped`.
- Built and packaged as `bin/v0.2/Release` and `dist/VoidCamera_0.2`. Injected successfully into restarted retail process PID 36496. Runtime reported `timeStopReady=1`; `--pause` changed `timeStopped=0` to `1`, then `--resume` returned it to `0`.
- Camera polling and pose publishing remain independent of the simulation pause. The 3-second missed-hook failsafe is suspended while time stop is active so the camera is not silently disabled during a pause.
- This runtime check was at the game front end (`view=0`, `captures=0`), because the UI automation inventory exposed no game window. It verifies CVar resolution and write/restore state, not the actual world-simulation effect. A loaded-save visual check must confirm world animation stops/resumes and the overlay/input stays responsive before claiming time stop fully validated.
- After the paused-camera guard change, rebuilt/repackaged and injected again into PID 20088. `timeStopReady=1`; `--pause` reported `timeStopped=1` and `--resume` restored `timeStopped=0`. The process again had no gameplay view, so actual camera movement while the world is frozen remains unobserved.

