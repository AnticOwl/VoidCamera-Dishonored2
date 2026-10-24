# Dishonored 2 — Static binary analysis (v1)

Sources: uploaded Dishonored2.exe; Dis2-Cmds.txt debug CVar dump; uploaded VoidPhotoMode 0.8.0 backend. Static inspection only. No runtime verification.

## Executable identity

- SHA-256: `b671f8249422862bf557d00aab2f4bb671ed4515cc10ad8a8a9decad84516180`. Matches VoidPhotoMode README expected SHA-256.
- x64 PE, preferred ImageBase 0x140000000, image size 0x41CB000, entry RVA 0x11B8BE4. ASLR enabled.
- PDB RSDS identity: `1bfecd57-a090-4e64-8dd0-0e89a6e9ac31; age 9`. The embedded PDB path was omitted because it contains a developer's local filesystem path.
- No embedded PDB symbol table inferred from this reference. PE exports absent. Windows/import symbols do not identify private engine functions.

## Interpretation

CVar names found in the EXE show name strings in the image, **not verified behavior or writable offsets**. Backend strings similarly are not proof of writes. The debug dump provides values and the GitHub gist provides candidate offsets; the EXE is a retail image with zero-initialized/unmapped static ranges, so offset values require checking a running process.

- Parsed `2448` CVar-value lines from debug dump; `1485` had exact null-terminated ANSI/UTF16 strings in EXE; `12` were found as ANSI strings in backend. (This is a conservative search, not a full runtime registry dump.)

## Selected CVar cross-check

| CVar | Debug value | EXE exact string count | VoidPhoto backend exact string count | EXE string RVA(s) |
|---|---:|---:|---:|---|
| `g_freeCam` | `1` | 1 | 0 | .rdata:0x1c8bb88 |
| `g_freezeTime` | `0` | 1 | 1 | .rdata:0x1c95418 |
| `g_stopTime` | `0` | 1 | 1 | .rdata:0x1ac7c00 |
| `g_runFrames` | `0` | 1 | 1 | .rdata:0x1b2fe68 |
| `g_showHud` | `1` | 1 | 0 | .rdata:0x1c8d010 |
| `ark_show_ui` | `1` | 1 | 0 | .rdata:0x1c60548 |
| `cam_freecam_movespeed` | `2` | 1 | 0 | .rdata:0x1c71e78 |
| `cam_freecam_turnspeed` | `300` | 1 | 0 | .rdata:0x1c71fb0 |
| `ark_fieldOfView` | `80` | 1 | 0 | .rdata:0x1c8de50 |
| `r_forceLOD` | `0` | 0 | 0 | - |
| `lod_debug_force_value` | `-1` | 0 | 0 | - |
| `lod_dist_far` | `50` | 1 | 0 | .rdata:0x1ba2d00 |
| `lod_dist_level0` | `30` | 1 | 0 | .rdata:0x1ba2c00 |
| `lod_dist_level1` | `40` | 1 | 0 | .rdata:0x1ba2c40 |
| `lod_dist_level2` | `50` | 1 | 0 | .rdata:0x1ba2c80 |
| `lod_dist_level3` | `70` | 1 | 0 | .rdata:0x1ba2cc0 |
| `lod_forcevisibledistance` | `10` | 1 | 0 | .rdata:0x1ba2bb0 |
| `r_disableStaticWorldLOD` | `0` | 0 | 0 | - |
| `r_viewDistPreset` | `5` | 1 | 0 | .rdata:0x1b716d0 |
| `r_zfar` | `7000` | 1 | 0 | .rdata:0x1b31b80 |
| `r_postProcessCustomEV` | `0.0` | 1 | 1 | .rdata:0x1b28010 |
| `r_skipAutoExposure` | `0` | 1 | 0 | .rdata:0x1b27c28 |
| `r_skipAutoExposureTemporalAdapatation` | `0` | 1 | 1 | .rdata:0x1b27c90 |
| `r_skipBloom` | `0` | 1 | 1 | .rdata:0x1b27d88 |
| `r_skipLensFlares` | `0` | 1 | 0 | .rdata:0x1b27db8 |
| `r_skipColorGrading` | `0` | 1 | 1 | .rdata:0x1b27df8 |
| `r_skipDOF` | `0` | 1 | 1 | .rdata:0x1b23808 |
| `r_DofFocalDist` | `5.0` | 1 | 0 | .rdata:0x1b23b28 |
| `r_DofFocusRange` | `10000.0` | 1 | 0 | .rdata:0x1b23b40 |
| `r_DofCOC` | `0.025` | 1 | 0 | .rdata:0x1b23be0 |
| `r_skipFog` | `0` | 2 | 0 | .rdata:0x1b361e0; .rdata:0x1c91c57 |
| `r_skipGodRays` | `0` | 1 | 0 | .rdata:0x1b36258 |
| `r_skipCameraMotionBlur` | `1` | 1 | 0 | .rdata:0x1c8df18 |
| `r_shadowCullDistance` | `75` | 1 | 0 | .rdata:0x1b29f90 |
| `r_showLights` | `0` | 1 | 0 | .rdata:0x1b320a0 |
| `r_areaLights_FactorDiffuse` | `3.0` | 1 | 0 | .rdata:0x1b2e4d0 |
| `r_areaLights_FactorSpecular` | `0.08` | 1 | 0 | .rdata:0x1b2e530 |
| `r_lightIlluminanceFactor` | `5` | 0 | 0 | - |
| `r_cullLightArea` | `8` | 0 | 0 | - |
| `r_windowWidth` | `1904` | 1 | 0 | .rdata:0x1ae23e8 |
| `r_windowHeight` | `1001` | 1 | 0 | .rdata:0x1ae23f8 |
| `rs_enable` | `3` | 1 | 1 | .rdata:0x1b36ef0 |
| `rs_forceFractionX` | `0.0` | 1 | 0 | .rdata:0x1b36f78 |
| `rs_forceFractionY` | `0.0` | 1 | 0 | .rdata:0x1b36fc8 |
| `r_subpixelXoffset` | `0.0` | 1 | 0 | .rdata:0x1b31c48 |
| `r_subpixelYoffset` | `0.0` | 1 | 0 | .rdata:0x1b31c88 |
| `win_pauseOnFocusLoss` | `0` | 1 | 0 | .rdata:0x1ae2308 |
| `image_lodbias` | `0` | 1 | 0 | .rdata:0x1b1d718 |
| `r_forceFirstMipMap` | `0` | 1 | 0 | .rdata:0x1b31d48 |
| `r_addAllMips` | `0` | 1 | 0 | .rdata:0x1b227e8 |
| `r_textureStreamerMemoryBudgetMB` | `4096` | 1 | 0 | .rdata:0x1b22a70 |

## Priority native commands in debug dump

`bugit`, `bugitgocam`, `getviewpos`, `setviewpos`, `ListRenderLights`, `popLight`, `clearLights`, `editEnvironment`, `listCvars -type`, `listCvars -flags`, `listCvars -help` appear in debug command list. That does **not** establish availability in retail executable.

## Actionable next validation

1. Identify retail CVar registration and lookup routine from references to names and vtable/type metadata. Validate GitHub offsets against retail process and read flags before writes.
2. Verify `r_forceLOD` semantics (0 may mean disabled or select level, unspecified); inspect code using its value and LOD driver.
3. Map live camera location/rotation/FOV and preserve original render state on detach. Do not use a console debug freecam in place of controllable native camera.
4. Check `r_postProcessCustomEV` and bloom/lens flare/color grading actual render consumers and restore original values.
5. Inspect DXGI swapchain resize paths for hotsampling and overlay mouse mapping. IGCSConnector is connection-status-only UI.
6. Discover light enumeration/spawn native structures only if practical (optional feature).

Excluded from tool: environment tab, resolution scale, built-in screenshot capture.

## GitHub cross-check: known CVar object/value offsets

From fakuivan's `dishonored2_cvars.txt` for 1.77.9.0 (these are GitHub-source offsets, not independently live-validated):

| Name | CVar object RVA | Value RVA | In supplied retail EXE name strings? |
|---|---|---|---|
| `g_freeCam` | `0x3BFA3F0` | `0x3BFA418` | Yes |
| `g_freezeTime` | `0x3BFE060` | `0x3BFE088` | Yes |
| `lod_dist_level0` | `0x3BEA1E0` | `0x3BEA208` | Yes |
| `lod_dist_level1` | `0x3BEA260` | `0x3BEA288` | Yes |
| `lod_dist_level2` | `0x3BEA2E0` | `0x3BEA308` | Yes |
| `lod_dist_level3` | `0x3BEA360` | `0x3BEA388` | Yes |
| `lod_dist_far` | `0x3BEA3E0` | `0x3BEA408` | Yes |
| `lod_forcevisibledistance` | `0x3BEA160` | `0x3BEA188` | Yes |
| `r_postProcessCustomEV` | `0x359A630` | `0x359A658` | Yes |

**Critical discrepancy**: `r_forceLOD`, `lod_debug_force_value`, and `r_disableStaticWorldLOD` appear in the user's debug build CVar dump but were not found as exact null-terminated ASCII CVar names in this supplied retail EXE. The GitHub dump does not document `r_forceLOD`. The forced-LOD feature is thus *not confirmed* for retail. Direct code-level analysis of LOD selection/registration is necessary; do not supply a fake ImGui toggle.

**PDB identity for symbol-server lookup:** GUID `1bfecd57-a090-4e64-8dd0-0e89a6e9ac31`, age `9`, file `Dishonored2_x64ShippingRetail.pdb`. A reference does not provide the actual symbol database.

**Important**: `Dis2-Cmds.txt` claims 2865 CVar entries, while the conservative line parser obtained 2448 CVar/value lines (some entries use other formats). CSV reflects parsed lines, not necessarily the entire game CVar registry.
