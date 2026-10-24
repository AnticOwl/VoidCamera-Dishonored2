# Dishonored 2 LOD CVar runtime validation

Build: Dishonored 2 retail 1.77.9.0, executable SHA256
`b671f8249422862bf557d00aab2f4bb671ed4515cc10ad8a8a9decad84516180`.

## Method and live profile

Read-only inspection of the running retail process. Each object was located by
the pointer to its exact CVar name at `+0x30`, and the runtime integer and float
caches at `+0x28` and `+0x2C` were read along with the default and help strings.
All object RVAs below were verified this way. The debug-build CVar list and
third-party offsets were not treated as proof of retail availability.

The process had view-distance preset 5 (Ultra), shadow-quality preset 5 (Ultra),
and these active entity distances: visibility 10, LOD0 30, LOD1 40, LOD2 50,
LOD3 70 and far LOD 50. The current values differ from several CVar default
strings, so the tool saves the live values at injection and does not reset them
to compile-time defaults.

## Geometry and shadow LOD

Retail help distinguishes the entity LOD thresholds from static-world LOD. The
`lod_dist_*` family describes entities; `r_lodZoomMultiplicator` explicitly
describes static LOD. User visual testing in Void Camera 1.7 confirmed that
lower values retain more detail at distance. `r_modelsQualityPreset` is the game-wide model detail
preset corresponding to the `ARK_VISUALSETTING_MODEL_DETAILS` menu setting.

| UI control | Retail CVar | Object RVA | Live int / float | Default string | Retail help |
|---|---|---:|---:|---|---|
| View-distance preset | `r_viewDistPreset` | `0x3982E70` | `5 / 5` | `3` | View distance preset : 0 Very Low 1 Low 2 Medium 3 High 4 Very High 5 Ultra |
| Shadow-quality preset | `r_shadowQualityPreset` | `0x3982CF0` | `5 / 5` | `3` | Shadow quality preset : 0 Very Low 1 Low 2 Medium 3 High 4 Very High 5 Ultra |
| Model-quality preset | `r_modelsQualityPreset` | `0x3982DF0` | `5 / 5` | `3` | Models quality preset : 0 Very Low 1 Low 2 Medium 3 High 4 Very High 5 Ultra |
| Static-world LOD multiplier | `r_lodZoomMultiplicator` | `0x32FBF20` | `0 / 0.6` | `1.0f` | Zoom multiplicator for static LODs |
| Entity visibility distance | `lod_forcevisibledistance` | `0x3988BB0` | `10 / 10` | `10` | under that distance the entity is considered visible. |
| LOD level 0 distance | `lod_dist_level0` | `0x3988C30` | `30 / 30` | `3` | under that distance the entity lod level0. |
| LOD level 1 distance | `lod_dist_level1` | `0x3988CB0` | `40 / 40` | `6` | under that distance the entity lod level1. |
| LOD level 2 distance | `lod_dist_level2` | `0x3988D30` | `50 / 50` | `12` | under that distance the entity lod level2. |
| LOD level 3 distance | `lod_dist_level3` | `0x3988DB0` | `70 / 70` | `20` | under that distance the entity lod level3. |
| Far LOD threshold | `lod_dist_far` | `0x3988E30` | `50 / 50` | `50` | under that distance the entity lod isn't far. |
| Texture-detail preset | `r_textureDetailsPreset` | `0x3982F70` | `5 / 5` | `3` | Texture details preset : 0 Very Low 1 Low 2 Medium 3 High 4 Very High 5 Ultra |
| Model shadow distance | `r_shadowCullDistance` | `0x32F8970` | `75 / 75` | `25.0f` | Max distance to display shadow for models |

The native help strings do not specify the distance unit, so the interface labels
these values as distances without claiming meters. The checked build has no
retail name string for `r_forceLOD`, `lod_debug_force_value`, or
`r_disableStaticWorldLOD`; there is no verified forced-mesh-level toggle.

The LOD distances previously listed from a third-party CVar dump at RVAs
`0x3BEA160` through `0x3BEA3E0` are not the live retail CVar objects in this
process. The validated objects are in the `0x3988xxx` range shown above.

## Texture detail

| UI control | Retail CVar | Object RVA | Live int / float | Default string | Retail help |
|---|---|---:|---:|---|---|
| Texture mip bias | `image_lodbias` | `0x32EA470` | `0 / 0` | `0` | change lod bias on mipmapped images |
| Force highest-resolution mip | `r_forceFirstMipMap` | `0x3937860` | `0 / 0` | `0` | force every texture sampler mipmap to 0 |
| Enable all texture mips | `r_addAllMips` | `0x32EFD90` | `0 / 0` | `0` | Enables all mips (use with caution, e.g. for HD screenshots |
| Texture streaming budget (MB) | `r_textureStreamerMemoryBudgetMB` | `0x32F0090` | `4096 / 4096` | `-1` | threshold of coarse VRAM texture amount to decide when to unload texture mips (in MBytes !) |

`r_addAllMips` explicitly warns that it should be used with caution. Its control
is opt-in and the panel notes the video-memory cost. The streaming budget's live
value (4096 MB) differs from its default string (-1), so restore returns to 4096.

## Validation limits

This scan verifies retail presence, object correspondence, current values and
native descriptions; it does not establish visual response to changing the
values. The setting names and help indicate that the distance thresholds govern
entity visibility and mesh levels, but the visual transitions need confirmation
in Void Camera 1.1. Presets and mips may also have game-side update timing that
cannot be confirmed by reading CVar memory alone.

## Arkane setting identifiers and shadow controls

The retail image contains uppercase `ARK_VISUALSETTING_*` identifiers such as
`ARK_VISUALSETTING_VIEW_DISTANCE`, `...MODEL_DETAILS`, `...SHADOW_QUALITY` and
`...TEXTURE_DETAILS`. These are setting identifiers, not CVar objects. No live
object was found for `ark_visualsetting_view_distance`; the corresponding live
quality controls are the lowercase `r_*Preset` CVars. No exact lowercase `ar_*`
CVar name was found in the retail image. Lowercase `ark_*` names found for UI
and gameplay do not expose a verified LOD control.

The active retail process was also checked after the user had experimented with
the tool values. This is a transient state, not a recommended profile or game
default:

| CVar | Current value during inspection |
|---|---:|
| `r_shadowCullDistance` | `1000` |
| `r_viewDistPreset` / `r_modelsQualityPreset` / `r_shadowQualityPreset` / `r_textureDetailsPreset` | `5 / 5 / 5 / 5` (Ultra) |
| `r_lodZoomMultiplicator` | `0.6` (default `1.0`) |
| `lod_forcevisibledistance` / `lod_dist_level0` | `0 / 200` |
| `lod_dist_level1` / `lod_dist_level2` / `lod_dist_level3` / `lod_dist_far` | `300 / 400 / 500 / 50` |
| `image_lodbias` / `r_forceFirstMipMap` / `r_addAllMips` | `0 / 1 / 1` |
| `r_textureStreamerMemoryBudgetMB` | `16384` MB |

These readings confirm that the active CVar caches changed, but reading them
does not prove the renderer's visible response. The entity LOD thresholds are
not a control for static buildings; use `r_lodZoomMultiplicator` and the model
quality preset for those. `r_shadowCullDistance=1000`
allows **model** shadows to remain visible much farther than the earlier live
value of 75. The current LOD thresholds are also far from the earlier baseline
of 10/30/40/50/70/50 and are not a useful neutral setup for judging transitions.

## Void Camera 1.7 distance override

The user reports that `r_forceLOD` previously improved image quality by keeping
detail at greater distances. The exact CVar is still not verified in the
supported retail executable, so v1.7 provides an explicit workaround using
verified retail entity thresholds: the **Force highest entity detail to
100,000** button sets visibility/LOD0 to 100,000 and the later LOD transitions
to 200,000/300,000/400,000/500,000. This attempts to keep entity LOD0 selected
to a very large distance. It does not force static-world geometry; that remains
controlled separately through `r_lodZoomMultiplicator`. These larger values
were not visually validated in-game at the time of release and may reduce
performance. The UI also accepts direct threshold entry up to 1,000,000 and
model shadow distance up to 100,000, so users can test beyond the former 1,000
UI cap. Retail cache reads alone do not prove visual response.

`r_shadowCascadeFars` is present at object RVA `0x397A350`; its native help says
it takes four cascade distances and its default string is `4 12 30 75`. Its
integer/float scalar caches both read as zero, so the normal scalar CVar writer
must not be used for it. The adjacent directional-shadow settings
`r_shadowParallelSliceCnt` and `r_shadowParallelSliceSize` are live at
`0x397A2D0` (4) and `0x397A250` (2048). These variables may be relevant to
world-shadow transitions, but their visual effect and safe update path have
not been confirmed.

## Live follow-up, 2026-10-24

After the user's Void Camera 1.7 test, a read-only scan of the running retail
process confirmed the tool's writes reached the CVar caches:

| Control / CVar | Live value | Interpretation |
|---|---:|---|
| `r_lodZoomMultiplicator` | `0.01` | User confirms this lowest tested multiplier visibly improves static-world detail. |
| `lod_forcevisibledistance` / `lod_dist_level0..3` / `lod_dist_far` | `100000 / 100000 / 200000 / 300000 / 400000 / 500000` | The distance-based highest-detail preset was written successfully; renderer response still needs visual comparison. |
| `r_forceFirstMipMap` / `r_addAllMips` | `1 / 1` | Both mip controls read as enabled in the running process; this alone does not prove sharper pixels or that the streamer retained them. |
| `r_textureStreamerMemoryBudgetMB` | `4096` MB | Current readback; memory availability and visible texture response were not checked. |
| `r_shadowCullDistance` | `10000` | Model-shadow range already very high; remaining distant shadows are likely directional sun cascades, not this setting. |
| `r_shadowCascadeFars` | `4 12 30 75` | Native help: four directional cascade far distances; scalar caches are zero and must not be written as floats. |
| `r_shadowParallelPartialSliceUpdate` | `1` | Native help says cascades are time-sliced except slice 0 to save performance; disabling is a test for delayed/pop-in updates. |
| `r_shadowParallelPartialDynaUpdate` | `0` | Native help says 1 skips dynamic updates on far slices; current 0 already keeps these updates enabled. |
| `r_shadowParallelSliceSize` | `2048` | Pixels per directional-shadow slice; higher resolution may improve detail at a GPU cost. |
| `r_shadowParallelResolutionDividerLevel` | `0` | Native help identifies 0 as highest quality; already optimal. |

Void Camera 1.8 added controls for time-sliced sun-shadow cascade updates,
directional slice resolution, and far-cascade distance. The user's test reported
no visual change. A follow-up read-only inspection of the live 1.8 process found
why: the UI changed the scalar integer/float caches but left each scalar CVar's
native current-value string unchanged. For example, the slice-size cache read
4096 while its current string remained `2048`; the partial-update cache read 0
while its current string remained `1`. This confirms the old generic writer did
not update the value the renderer consumes for those controls. The cascade
string itself changed to `4 12 30 999`, but the renderer did not update it live.

Void Camera 1.9 updates and restores the verified inline CVar value string as
well as the numeric caches for scalar controls. Shadow-buffer settings are now
instructed to be set from the main menu before loading the save, giving the
renderer a chance to create the level's shadow resources with those settings.
This update path needs a new visual comparison; no improvement is claimed until
the user confirms the shadow appearance changes.
