# Dishonored 2 — direct IGCS DOF connection

## Implementation

VoidCamera connects directly to the in-process ReShade addon. It enumerates
loaded modules, resolves the addon's exported `connectFromCameraTools()` and
`getDataFromCameraToolsBuffer()` functions, calls the first, and writes the
IGCS `CameraToolsData` layout into the returned addon-owned buffer every camera
frame. There is no relay DLL, named pipe, or shared-memory bridge.

The data layout is checked at compile time: size 84 bytes; FOV at offset 4;
position 8; quaternion 20; up/right/forward vectors 36/48/60; pitch/yaw/roll
72/76/80. Values are sourced from VoidCamera's live pose; angles are radians,
FOV degrees. Local movement semantics define forward as matrix row 0, right as
the negative of row 1, and up as row 2.

The Camera and Post-process tabs report whether the addon handshake has
returned its data buffer.

IGCS Connector also requires the camera-tools screenshot-session exports to
consider a camera connected. DOF calls `IGCS_StartScreenshotSession` with
`ScreenshotType::MultiShot` and then moves the camera for each aperture sample.
VoidCamera now saves the starting pose, applies each sample in the same
right/up basis it uses for live movement, locks out local camera input during
sampling, and restores the starting pose on session end. The live movement
frame intentionally excludes roll so control directions do not rotate when
the view is rolled. DOF samples use this same roll-independent frame: IGCS's
focus delta and aperture X/Y are screen-axis parameters, and applying the full
roll to camera offsets rotated the bokeh sampling axis in rolled views. The
Connector continues to receive the full look quaternion and rolled basis for
camera metadata. The bundled `extras/IgcsDof.fx` reads
`IGCS_cameraRotationRoll` and rotates the focus-alignment vector separately,
so Focus delta follows the rolled camera even though aperture sample movement
uses the stable control frame. Replace the installed `IgcsDof.fx` with this
version for rolled shots. Panorama stitching remains unavailable.

## Runtime requirements and validation

- ReShade 6.4+ with add-on support.
- Official `Igcsconnector.addon64` installed beside `Dishonored2.exe`.
- The official IGCS DOF shader and its textures installed under the matching
  ReShade `reshade-shaders` folders; enable `IgcsDOF` in the Home tab.
- Enable VoidCamera, then verify the Connector's camera data is live before
  starting a DOF session.

The local IGCS Connector source documents this function/buffer protocol.
Working patterns in other workspace projects were compared: DXMD snapshots and
restores its native camera matrix/position/FOV; Watch Dogs snapshots the camera,
builds a right/up basis, and publishes every sample; Fallout 4 validates and
restores its native free-camera state; Skyrim uses the same session lifecycle
and logs sample movement. VoidCamera adapts that lifecycle to its owned `Pose`
and direct Connector buffer. The user confirmed on 2026-10-24 that IGCS DOF
works in game with this implementation.

Sources: local `IGCSConnector/src/Main.cpp`, `CameraToolsData.h`, and
`CameraToolsConnector.cpp`; official IGCS DOF guide:
https://opm.fransbouma.com/igcsdof.htm
