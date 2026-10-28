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
| Far Z clip plane | `r_zfar` | `0x3937460` | `7000 / 7000` | `7000` | far Z clip plane distance, 0 = infinity |
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

## Zoom-driven static LOD follow-up

The user asked to test the static-world multiplier down to `0.001` and identify
the routine that adds detail during zoom, then make that detail persist without
zoom. `r_lodZoomMultiplicator` is documented by the retail help as the zoom
multiplier for static LOD; the user previously confirmed that `0.01` visibly
improves detail at distance. The camera FOV should remain unchanged while
testing `0.001`, so the CVar effect can be isolated.

Initial static disassembly found three references to the CVar object's RVA
`0x32FBF20` in registration/forwarding code, and did not decode the cached
float's consumer. The targeted follow-up below located a direct read of the
cached float. The retail binary still has no exact string for `r_forceLOD`.
Runtime confirmation that the zoom-related input field changes on spyglass
activation is still needed. The `0.001` value is accepted by Void Camera 1.17,
but its visual effect is not yet tested.

### Retail code follow-up: zoom factor consumer

Ghidra inspection of the supported retail executable found a direct read of
the cached float at VA `0x1432FBF4C` (RVA `0x32FBF4C`) at instruction
`0x1403522D1`: `MULSS XMM4, dword ptr [0x1432FBF4C]`. The surrounding code
loads a per-object float from `[RAX + 0x3A6C]`, normalizes it by a global
factor, clamps it with `MINSS`, multiplies it by `r_lodZoomMultiplicator`,
then writes the resulting scalar to `[RCX + 0x38]`. This is direct evidence
that the LOD CVar multiplies the normalized scalar read from `[RAX + 0x3A6C]`;
it is not itself a zoom-state bool or the key handler. Runtime inspection by
the user identified `[RAX + 0x3A6C]` as the aspect-ratio value, not the
spyglass zoom state. This consumer therefore shows an aspect-ratio-dependent
input to the LOD scalar, not the effect of pressing W. Do not use that field
as the target for a write breakpoint when tracing spyglass activation. The
input action/key event path and the zoom-dependent field remain unidentified.
Ghidra auto-analysis of the full executable hit its five-minute limit, but
targeted disassembly of this consumer succeeded.

### Zoom action binding and engine identifiers

The active user config at
`C:\Users\olivi\Saved Games\Arkane Studios\Dishonored2\base\dishonored2Config.cfg`
maps `W` to `MOVE_FORWARD` and `Z` to `ZOOM` (`arkBind` entries). Therefore
the current saved configuration does not make W the spyglass/zoom action. Raw
retail executable strings include `PLAYER_STATE_SPYGLASS`,
`PLAYER_STATE_SPYGLASS_ZOOM`, and `ARK_ATTRIBUTE_SPYGLASS_ZOOMLEVEL`. These
identify relevant player states and an attribute, but static string presence
alone does not prove which functions transition between them or which state
affects LOD. The next code/runtime trace must follow the `ZOOM` action and
`SPYGLASS_ZOOMLEVEL`, rather than treating W or the aspect-ratio field at
`RAX+0x3A6C` as the LOD trigger.

### Retail entity LOD distance path

Targeted references to the live float caches found the entity distance CVars
being consumed together in `FUN_14064CFF0` (entry VA `0x14064CFF0`, RVA
`0x64CFF0`). At `0x14064D0D2` through `0x14064D11B`, the code reads
`lod_forcevisibledistance`, `lod_dist_level0..3`, and `lod_dist_far`, and
squares each threshold. It also derives and squares a per-object distance
value, then passes the distance and threshold arrays to a helper. This is a
real distance-based entity LOD/visibility path. It explains why advancing
toward a table or prop can make another mesh/LOD become visible: the measured
distance crosses one of these thresholds. It does not prove that `ZOOM`
changes these thresholds or calls this function; the distinction between
movement (W=`MOVE_FORWARD`) and optical zoom (Z=`ZOOM`, gamepad
`PAD_RTHUMB`) remains essential. Static-world LOD is a separate path and
still needs a runtime comparison to determine whether spyglass zoom changes
its selection.

## Void Camera 1.29 diagnostic test plan

The LOD tab now exposes the verified entity thresholds and a reversible preset
that pushes visibility/LOD0 to 100,000 and later transitions to 200,000 through
500,000. The preset is an attempt to hold entity LOD0 at long range, not a
confirmed engine force flag and not a static-world LOD control.

The optional CSV log samples every 250 ms at
`%LOCALAPPDATA%\VoidCamera\lod-spyglass-test.csv`. It records the previously
found keyboard bindings (`Z` = `ZOOM`, `W` = `MOVE_FORWARD`), gamepad button and
axis state, camera position and view dimensions, aspect ratio, static-world LOD
multiplier, and entity LOD distance values. Compare separate runs: approach the
table without using Spyglass, then remain at a fixed distance and toggle
Spyglass. This distinguishes a proximity transition from a zoom-related change.

## Mesh appearance range follow-up

The user's 1.29 run reports that changing the six `lod_*` distances and the
long-range LOD0 preset did not change the distance at which the observed
objects appeared. Treat that as evidence that these controls do not govern
that object's appearance range in the tested scene; it does not invalidate
their entity LOD consumer described above.

A targeted scan of null-terminated retail executable names containing
`dist`, `distance`, `cull`, `view`, `visib`, `range`, `far`, `mesh`, or `lod`
found no separate, clearly named mesh draw-distance CVar. The relevant
verified candidates remain:

* `r_viewDistPreset` (already Ultra in the inspected profile): game view-distance
  quality preset. Its menu mapping is known, but its exact runtime consumers
  and whether changing its CVar alone rebuilds level visibility are not yet
  established.
* `r_zfar` (help: far Z clip plane distance, with 0 meaning infinity): camera
  clipping distance, not evidence of an object/mesh streaming range.
* `r_lodZoomMultiplicator`: a static-world LOD detail multiplier, which has
  visibly changed distant detail in prior user testing; it does not establish
  when an object is first included in the scene.
* `lod_forcevisibledistance` and `lod_dist_level0..3` / `lod_dist_far`: entity
  visibility and entity model-detail thresholds. In 1.29, changing them had
  no reported effect on the observed mesh appearance range.

Next investigation should follow the mesh's actual visibility path: determine
whether it is an entity component or static world geometry, then inspect the
retail code that inserts/filters that class of object into the render view,
including view-distance preset consumers and any cell/sector streaming or
occlusion path. `r_zfar` is not a good substitute: widening the camera clip
plane can reveal distant objects only if clipping is the cause, and cannot
force unloaded or culled meshes to render. The 1.29 CSV contains no manual
marker for the instant a particular mesh becomes visible, so it cannot by
itself correlate a transition with camera position or Spyglass state.

Void Camera 1.30 addresses this measurement gap. Its 100 ms
`%LOCALAPPDATA%\VoidCamera\lod-mesh-test.csv` log has explicit `mesh_absent`
and `mesh_visible` marker events. Each event is sampled with camera position,
key/gamepad state, static and entity LOD values, `r_viewDistPreset`, and
`r_zfar`. The inspected running game read `r_viewDistPreset=5`, `r_zfar=7000`,
and `r_lodZoomMultiplicator=0.001`. These are the starting values for the next
visual test, not evidence that changing the first two alters mesh visibility.
The same mesh must be marked absent/visible while approaching, then tested at
a fixed position while Spyglass is toggled. Compare `r_viewDistPreset` at
levels 5 and 0; test `r_zfar` at 7000 and 0 (infinite clipping), restoring
initial render settings after each experiment. If only proximity changes the
marker position and neither candidate shifts it, inspect the mesh's entity vs
static-world classification and the corresponding streaming/occlusion path.

## View-distance preset setter check (2026-10-27)

The 1.30 overlay writes `r_viewDistPreset` directly into the CVar value cache.
After the user changed it from 5 to 3 through that overlay, a read-only snapshot
of the same 2,048 live CVar objects found only `r_viewDistPreset` changed; the
other quality preset CVars remained at 5. This is expected for a direct cache
write and does **not** describe what the game's Options menu applies when it
selects High.

Static inspection of the retail executable shows the generic CVar setter at
`Dishonored2.exe+0x102400` performs the CVar update and invokes virtual
notification/callback methods. The normal setter wrapper at `+0x1028E0` calls
that path. Void Camera's direct memory write bypasses those calls. Therefore,
the 5-to-3 overlay experiment is useful for checking whether the renderer reads
the CVar live, but cannot enumerate the settings bundled into the native View
Distance preset. The executable contains the six-level label mapping
(`0 Very Low` through `5 Ultra`) and an `OnUpdatePreset` UI callback name, but
the exact per-preset values and the native preset handler's full write set
remain unidentified. Do not claim that Ultra consists of any particular CVar
bundle until that native handler is followed or an actual Options-menu change
is captured with before/after CVar snapshots.

## Overall render-quality preset handler

Following the `OnUpdatePreset` callback exposed a separate master setting,
`r_renderQualityPresets` (CVar object RVA `0x32FB450`). Its native handler at
`Dishonored2.exe+0x5122D0` applies the selected master level across multiple
settings. For master levels 0-5, it sets all of these individual quality
preset CVars to the same level: `r_shadowQualityPreset`,
`r_oceanQualityPreset`, `r_modelsQualityPreset`, `r_viewDistPreset`,
`r_textureDetailsPreset`, and `r_maxDynamicDecalsPreset`.

It also maps anti-aliasing separately (master 0 -> FXAA Low, 1 -> FXAA High,
2-5 -> Temporal AA), maps `r_downsampleQualityPreset` to 1 for levels 0-1 and
3 for levels 2-5, and sets `r_downsampleQuality` to 1. Several skip toggles
(`r_skipBloom`, `r_skipLensFlares`, `r_skipGodRays`,
`r_skipGodRaysUnderWater`, `r_skipDofBck`) are enabled below High (levels 0-2)
and disabled at High/Very High/Ultra (3-5). The same handler also writes
`r_skipCameraMotionBlur` and the rat/bloodfly shadow toggles as part of its
quality profile.

This is the master **Render Quality** preset behavior, not the full internal
meaning of `r_viewDistPreset` in isolation. The latter is one of the six child
quality selectors and its native Options-menu selection updates its own stored
preset value. Its exact renderer-side thresholds or view-distance values have
not yet been recovered from the retail code. The direct Void Camera slider
changes only that child CVar and does not run the master profile handler.

## Native view-distance preset trace build (2026-10-27)

Void Camera 1.32 hooks the retail generic CVar setter at
`Dishonored2.exe+0x1028E0`. A trace sequence begins when the in-game setter is
called for `r_viewDistPreset` (`Dishonored2.exe+0x3982E70`). While that call is
executing, the hook records every nested call through the same setter, including
the CVar name/object RVA, old/requested/new integer and float cache values,
force/accepted flags, caller RVA, and stack RVAs. This should reveal values
applied through the common setter and identify the preset callback path.

Log: `%LOCALAPPDATA%\VoidCamera\r-viewdist-preset-trace.csv`.
Change View Distance in the game's own Options menu to capture it. This first
pass observes synchronous calls through the generic setter; direct memory writes
or work dispatched to another thread are outside this trace and would need a
separate memory-diff or follow-up hook if the log does not account for all
changes.

### Trace result

The user's 1.32 trace contains six successful value transitions:
`5→3`, `3→5`, `5→1`, `1→5`, `5→1`, `1→5`. Four transitions came through
caller RVA `0x3D323C61`; two came through the retail settings path at caller
RVA `0x512C6C` (the call site at `Dishonored2.exe+0x512C67`). During none of
the six setter calls did the same-thread hook observe a nested CVar setter.
Thus the View Distance option does not apply a sibling-CVar bundle through
the generic setter. Its new cache value is written successfully; the CSV's
`accepted` byte is not a reliable success result because the wrapper at
`+0x1028E0` returns through cleanup code without setting a documented boolean.

Next investigation target: follow the `r_viewDistPreset` notification/render
consumer or locate the renderer's lookup of this enum. The current setter trace
does not identify the object streaming threshold itself. If a future cache
snapshot shows additional CVar changes, those may be direct writes or async
work and require another capture method.

### Static CVar cache-diff capture build (1.33)

Because the 1.32 setter trace showed no nested setters, 1.33 also scans the
retail module's data section for objects matching the validated CVar vtables
and name-pointer layout, then snapshots each object's text, integer, and float
fields around a `r_viewDistPreset` setter call. Changed objects are written to
`%LOCALAPPDATA%\VoidCamera\r-viewdist-cvar-diff.csv`; the call trace remains in
`r-viewdist-preset-trace.csv`. The scan is limited to static CVar objects in the
main executable image and synchronous changes during the preset setter call.
This will show whether the game directly edits other CVar caches without using
the generic setter; renderer-internal state and later async writes need separate
runtime tracing.

### 1.33 trace review and registry traversal correction

The supplied 1.33 setter trace records the in-game sequence `5→4→3→2→1→0`;
each changed value comes through the settings path at caller RVA `0x51252B`.
A final `0→0` setter call is also present. No nested generic CVar setter calls
occur in these sequences.

The supplied `r-viewdist-cvar-diff.csv` contains only its header. That is not
evidence that no direct CVar values changed: 1.33's cache scanner incorrectly
assumed CVar objects had a shared vtable and returned before discovering the
registry. Static analysis of the retail registration function shows the CVar
registry is a linked list: head pointer at `Dishonored2.exe+0x31C04B0`, next
pointer at each object `+0x78`, with the name pointer at `+0x30`. Version 1.34
uses that list to enumerate registered CVars before and after the preset update.
The direct-cache question remains open until the corrected diff is captured.

### 1.34 registry-diff result and 1.35 notification trace

The supplied 1.34 CSVs cover the in-game sequence `0→1→2→3→4→5` and the
reverse `5→4→3→2→1→0`. The corrected linked-list walk found 1,616 CVar
objects. In both directions, the only changed registered CVar cache was
`r_viewDistPreset` at object RVA `0x3982E70`; the changed-value calls came from
the Options-menu path at caller RVA `0x51252B`. Same-value `5→5` and `0→0`
calls also occurred through `0x512C6C`. This rules out a synchronous bundle of
other registered CVar cache writes during the setter, but does not rule out
renderer state changes that are not represented as CVars or work scheduled
after the setter returns.

Void Camera 1.35 additionally resolves the live virtual method at
`[*(Dishonored2.exe+0x228BAE8) vtable+0x88]` and, when the address is a valid
executable target in the supported game image, hooks it conditionally. While
the preset setter is active, the trace records this method's entry/exit,
manager-method RVA, flags argument, return value, thread, and stack. If the
runtime target cannot be validated, camera injection continues without this
optional diagnostic hook and the trace records RVA `0`. The method's renderer
meaning is still a hypothesis until a 1.35 runtime trace establishes its call
sequence and the target can be mapped in the game code.

To capture: inject 1.35, change View Distance from 0 to 5 and back to 0 using
the game's Options menu, then share `r-viewdist-preset-trace.csv` and
`r-viewdist-cvar-diff.csv` from `%LOCALAPPDATA%\VoidCamera`. The trace CSV has
`# notify_method_rva,<RVA>` metadata; `notify_begin` / `notify_end` rows identify
calls made synchronously within the preset update. A target RVA of `0` or no
notification rows means this hypothesis was not confirmed and the next step is
to trace the settings path's direct renderer update or queued work.

The supplied 1.35 trace confirms a live target at RVA `0x102940`. Disassembly
shows the whole method is `or dword ptr [rcx+40h],edx; ret`: it accumulates
notification bits in the manager object. The native setter calls this at
`0x1024DF` via the manager vtable slot `+0x88`; the return address logged as
`0x1024E5` is the next instruction. That instruction then sets bit `0x80000`
in the CVar's own flags at `[CVar+0x48]`. This accounts for the observed
callback argument change: the first `0→1` passes `0x50012`, and later updates
pass `0xD0012` (`0x50012 | 0x80000`). The same-value `5→5` and `0→0` setter
calls do not enter the notification. Because `0x102940` does not write RAX,
the logged return value is incidental and must be ignored.

The cvar-diff still reports only `r_viewDistPreset`, so the preset's other
visual effects are not synchronous writes to another registered CVar cache.
The notification itself only marks accumulated change bits; it does not apply
LOD/render settings. The next target is the manager's consumer of its `+0x40`
pending-flags field (and any deferred settings/update pass) to find which
renderer or streaming state those bits trigger.

### 1.36 manager-method map and callback-chain finding

Void Camera 1.36 adds `r-viewdist-notify-vtable.csv`. On the first preset
notification it records the manager pointer, its vtable pointer, all 64 vtable
targets, and the manager's pending-flags value before and after each notification.
The manager is runtime-initialized, so its vtable could not be recovered from
the on-disk BSS object alone. This one capture supplies the exact method map;
then inspect those method RVAs for reads/clears of `manager+0x40` and follow the
consumer's call chain to the renderer settings affected by `r_viewDistPreset`.

Ghidra analysis of the retail image identifies the object at
`Dishonored2.exe+0x31C0640` as the `idCVarSystem` singleton. Its pending-flags
methods are exactly the observed vtable entries: slot `+0x88` ORs bits into
`[this+0x40]`, slot `+0x90` returns that field, and slot `+0x98` clears the
requested bits. The only manager-global read/clear path found in this pass is
`FUN_1400D2FC0` (RVA `0xD2FC0`): it sets bit `0x10000`, checks it, clears it,
then saves `dishonored2Config.cfg`. This is config persistence, not an identified
renderer or mesh-distance consumer. The CVar manager's pending mask therefore
does not yet explain the visible-range change.

The setter's important next call is `FUN_140102CB0(CVar)` at RVA `0x1024CD`,
before the manager notification. `FUN_140102CB0` normalizes/parses the value,
updates the CVar cache, then walks the linked callback list at `CVar+0x70`;
each node has a callback object at `node+0` and a next pointer at `node+8`, and
the engine invokes the callback object's vtable method at `+0x10`. The CVar
constructor initializes this list to null. `r_viewDistPreset` is registered
with a null constructor callback argument, so any listener must be attached
dynamically during startup. Static references to the CVar object/value lead to
registration and options-menu code; no direct renderer reference was found.

This establishes the next diagnostic without another broad scan. Void Camera
1.37 writes `%LOCALAPPDATA%\VoidCamera\r-viewdist-cvar-callbacks.csv` when the
user changes the preset. It records the live callback-list head and each node,
callback object/vtable, and the resolved `vtable+0x10` method before and after
the native setter. If a callback is present, its method RVA is the next static
target to decompile. If the list is empty, investigate where the renderer reads
the changed CVar value or whether the observed range change is caused by a
separate options-apply path; the current traces show no sibling CVar cache
changes during the setter.

The supplied 1.37 logs record both full transitions, `0→1→2→3→4→5` and
`5→4→3→2→1→0`, all through the Options-menu caller at RVA `0x51252B`. A same-
value `5→5` and `0→0` also occur through `0x512C6C` and do not enter the notify
method. For every changed value, the callback-list head at `CVar+0x70` is zero
both before and after the native setter; there is no CVar-specific listener to
follow. The CVar diff again contains only `r_viewDistPreset`, so no sibling
registered CVar cache is synchronously updated. The manager pending mask stays
`0xF5C9F` across notifications; because those bits were already set before the
first transition, this capture cannot attribute the accumulated mask to this
preset change. The callback branch is closed; the remaining path is a direct
consumer of the CVar value or non-CVar state updated by the Options caller.
