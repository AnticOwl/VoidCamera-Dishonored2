# Dishonored 2 render CVar runtime validation

Build: Dishonored 2 retail 1.77.9.0, executable SHA256 `b671f8249422862bf557d00aab2f4bb671ed4515cc10ad8a8a9decad84516180`.

## Method

Read-only scan of the live retail process. For each candidate, the scan matched
the exact CVar name pointer at object `+0x30`, verified the integer and float
caches at `+0x28` and `+0x2C`, and read the default and help strings referenced
by `+0x38` and `+0x40`. Addresses below are object RVAs for the supported retail
image; the tool checks the name at runtime before enabling a control. No values
were written during this inspection. The debug-build CVar list was not used as
proof of retail availability.

## Post-process controls

| UI control | Retail CVar | Object RVA | Runtime int / float | Default string | Retail help string |
|---|---|---:|---:|---|---|
| Auto exposure | `r_skipAutoExposure` | `0x32F7D40` | `0 / 0` | `0` | Skips image exposure adjustment |
| Freeze exposure adaptation | `r_skipAutoExposureTemporalAdapatation` | `0x32F7DC0` | `0 / 0` | `0` | Skips auto-exposure temporal adaptation (used to freeze it for HQScreenShots) |
| Bloom | `r_skipBloom` | `0x32F7F40` | `0 / 0` | `0` | Skips bloom post-processing |
| Lens flares | `r_skipLensFlares` | `0x32F7FC0` | `0 / 0` | `0` | Skips lens-flares post-process |
| Color grading | `r_skipColorGrading` | `0x32F8040` | `0 / 0` | `0` | Skips color grading post-process |
| Depth of field | `r_skipDOF` | `0x32F3D50` | `0 / 0` | `0` | DOF on/off |
| Camera motion blur | `r_skipCameraMotionBlur` | `0x39A4220` | `1 / 1` | `0` | If true (set to 1 ), skip completely the Camera Motion Blur |
| Tone mapping | `r_skipToneMapping` | `0x32F7EC0` | `0 / 0` | `0` | Skips S-curve tone mapping |
| Custom EV bias | `r_postProcessCustomEV` | `0x32F82C0` | `0 / 0` | `0.0` | Sets an additional custom Exposure Value (EV) bias on top of environment's EV |
| Focal distance | `r_DofFocalDist` | `0x32F44D0` | `5 / 5` | `5.0` | (in meter) |
| Focus range | `r_DofFocusRange` | `0x32F4550` | `10000 / 10000` | `10000.0` | (in meter) |
| Circle of confusion | `r_DofCOC` | `0x32F47D0` | `0 / 0.025` | `0.025` | circle of confusion (in millimeter) |

The `r_skip*` controls are displayed as effect checkboxes: checked means the
effect is on, so the underlying value is written as zero. The adaptation control
is worded as a freeze option and maps checked to `r_skipAutoExposureTemporalAdapatation=1`.
Motion blur is skipped in the initial runtime state (`1`). The displayed defaults
are initialized from the live value, not assumed from the debug list or string.

## Environment controls

| UI control | Retail CVar | Object RVA | Runtime int / float | Default string | Retail help string |
|---|---|---:|---:|---|---|
| Fog | `r_skipFog` | `0x397AC50` | `0 / 0` | `0` | skip rendering fog |
| God rays | `r_skipGodRays` | `0x397AD50` | `0 / 0` | `0` | Skip screen space god-rays |
| Fog light scattering | `r_skipLightScattering` | `0x397ACD0` | `0 / 0` | `0` | Skip rendering light-scattering in fog target |
| Sky | `r_skipSky` | `0x397AF50` | `0 / 0` | `0` | skip rendering sky |
| Area-light diffuse factor | `r_areaLights_FactorDiffuse` | `0x32FBD20` | `3 / 3` | `3.0` | Diffuse factor to apply to area lights globally. |
| Area-light specular factor | `r_areaLights_FactorSpecular` | `0x32FBDA0` | `0 / 0.08` | `0.08` | Specular factor to apply to area lights globally. |
| Area-light shadow lower bound | `r_areaLights_ShadowMin` | `0x32FBE20` | `0 / 0` | `0.0` | Shadow lower bound to consider as completely in shadow. |
| Area-light shadow upper bound | `r_areaLights_ShadowMax` | `0x32FBEA0` | `0 / 0.1` | `0.1` | Shadow higher bound to consider as completely in light. |
| Area-light pool visualization | `r_postProcessDebugAreaLights` | `0x32F86C0` | `0 / 0` | `0` | Shows the area lights currently in the pool. |
| Light-volume debug mode | `r_showLights` | `0x3937E60` | `0 / 0` | `0` | 1 prints volume counts/highlights, 2 also draws planes, 3 also draws edges. |
| Model shadow distance | `r_shadowCullDistance` | `0x32F8970` | `75 / 75` | `25.0f` | Max distance to display shadow for models |

The live shadow-distance value differs from its CVar default string (75 vs 25),
so the tool preserves 75 as the session's restore value. Render controls write
both runtime caches; floating-point integer caches follow the observed native
truncation behavior (for example, `0.08` has integer cache `0`).

## Light feature boundary

On the supported retail EXE, the strings `popLight` and `clearLights` are
present with descriptions for removing the last created light and clearing all
lights. Searches for `addLight`, `pushLight`, `spawnLight`, `createLight` and
`ListRenderLights` returned no matches. The debug-command list also includes
`popLight`/`clearLights`, but no matching creation command was identified.
Therefore these strings alone do not establish a retail light-spawn interface.
The retail image also contains editor CVar strings and RTTI for `idEditEntities`,
`idEditorLightInterface`, and game-editor interfaces. In a live test, setting
`g_editEntityMode=1` and `g_showEditEntityInfo=1` in the CVar text and numeric
caches produced no visible markers; both values were restored to zero. Void
Camera 1.15 exposes verified area-light controls and renderer diagnostics;
spawning/editing individual lights remains unimplemented until a native retail
creation path is identified and validated.

## Limits

Retail name resolution and live values were verified. The visual effect of each
setting still needs an in-game check with Void Camera 1.0, because the running
process used for this scan had Void Camera 0.9 and no render controls. The restore
button restores the values captured at injection, not an unknown game menu
profile. No claim is made that every environment-related CVar is exposed; this
table records the supported controls implemented in version 1.0.

## Current PP controls and numeric-effect limits

The verified retail controls available in the PP tab are: auto-exposure toggle,
exposure-adaptation freeze, additive custom EV, bloom toggle, lens-flare toggle,
color-grading toggle, DOF toggle plus focal distance/focus range/CoC, camera
motion-blur toggle, and tone-mapping toggle. Environment controls separately
expose fog, god rays, fog light scattering, sky, and area-light diffuse/specular
factors.

The retail render-CVar name scan contains `r_skipBloom`, `r_skipLensFlares`,
`r_skipEmissiveGlare`, and `r_useCheapLensFlareDistortion`; it contains no radial
blur CVar and no numeric bloom intensity, threshold, lens-dirt, or flare-strength
CVar. The public 1.77.9.0 CVar dump also has no radial-blur entry. This is
evidence only about CVar controls: it does not rule out per-scene post-process
or environment-object parameters. Those require separate runtime attribution
and cannot be exposed as numeric sliders from the CVar map alone.

### Retail post-process parameter leads

A separate raw-string scan of the same retail executable found parameter and
shader names beyond the CVar interface: `env/bloom/lensDirt`,
`PostFX/Bloom/upsample`, `PostFX/Bloom/enabled`, `PostFX/Bloom/dUV`,
`PostFX/Bloom/texBloom`, `PostFX/LensDirt/UseDefault`,
`PostFX/Bloom/lensDirt_From`, `PostFX/Bloom/lensDirt_To`,
`PostFX/Bloom/lensDirt_BlendWeight`, and `PostFX_LensFlares_Threshold`.
These confirm there are bloom/lens-dirt/flare parameters in the render pipeline,
but the strings alone do not prove which are per-scene inputs, shader outputs,
or writable live settings. They are new leads for tracing the retail render
parameter path; do not expose them as controls until a live value is resolved
and its visual effect is verified.
