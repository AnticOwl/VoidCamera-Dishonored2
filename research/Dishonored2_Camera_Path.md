# Dishonored 2 camera path

## Void Camera 1.48.7 implementation

The Camera Path tab records the active free-camera pose (XYZ, pitch/yaw/roll,
vertical FOV, and movement speed) as nodes. Each node stores the duration to
the next node. Playback interpolates camera position, orientation, and FOV,
supports looping, and can use Catmull-Rom smoothing. The path can be saved and
loaded at `%LOCALAPPDATA%\VoidCamera\Dishonored2_CameraPath.vcp`.

Playback requires the free camera to be enabled and at least two nodes. Turning
off the free camera stops playback. Save format is versioned as
`VOID_CAMERA_PATH_V1`; loading rejects non-finite/out-of-range pose data and
keeps the current path if the file is invalid. Maximum path size is 256 nodes.

## Prior references

- `CONTROL/ControlToolkit_InGame_v0.5f_IGCSDOFPhotoModeSignal/src/toolkit.cpp`
  implements node editing, per-segment duration, loop playback, Catmull-Rom
  position interpolation, shortest-angle rotation interpolation, and text
  save/load.
- `BioShockInfinite_CameraPath_DLL_v0.2_ImGui/src/UI.cpp` and `src/dllmain.cpp`
  provide another node-based camera path workflow and playback controls.

The Dishonored implementation uses the existing camera `Pose` and publishes the
interpolated pose through the normal camera update path. It has compiled, but
has not yet been visually tested in Dishonored 2; verify playback, loop closure,
save/load, and the shortest path across angle wrap (for example yaw 179 to -179)
before treating it as stable.

The 1.48.7 build adds overlay-hidden hotkeys: F6 add, Shift+F6 insert after the
selected node, F7 delete, Ctrl+Page Up/Down reorder, F8 play, F9 stop. Node
selection is made in the Camera Path tab before hiding the overlay.
