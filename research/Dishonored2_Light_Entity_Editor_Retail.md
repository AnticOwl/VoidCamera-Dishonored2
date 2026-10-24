# Dishonored 2 retail light entity editor investigation

Target: Dishonored 2 retail 1.77.9.0, SHA-256 `b671f8249422862bf557d00aab2f4bb671ed4515cc10ad8a8a9decad84516180`.

## Findings

- The retail executable has no exact `addLight`, `pushLight`, `spawnLight`,
  `createLight`, `cloneEntity`, or `g_showEditEntityDialog` command/CVar string.
  `popLight` and `clearLights` are registered with removal/reset descriptions;
  neither creates a light.
- The retail executable does register `g_editEntityMode`. Its help text maps
  `1` to lights. The name pointer resolves to the live CVar object at RVA
  `0x39A1A90`; its initial integer and float caches were both `0`.
- `g_editEntityCloneDist` is also registered in retail. Its exact live object is
  RVA `0x39A1B90`, initially `2 / 2.0`. Retail help says this controls how far
  from the camera a newly cloned entity is placed. This is evidence that the
  game contains a clone placement path, but does not yet prove its activation
  input or that cloning a light produces a functional spotlight.
- Additional runtime-resolved editor CVars:

  | CVar | Object RVA | Initial int / float | Retail help |
  |---|---:|---:|---|
  | `g_editEntityGridSize` | `0x39A1B10` | `0 / 0` | Snap to grid while dragging entities. |
  | `g_editEntityMouseDrag` | `0x39A1C10` | `0 / 0` | `0` uses bound keys plus LMB to drag; `1` uses LMB plus drag. |
  | `g_showLightEntities` | `0x39A1C90` | `0 / 0` | Includes lights while editing sounds (`g_editEntityMode == 2`). |
  | `g_showEditEntityInfo` | `0x39A1D10` | `0 / 0` | Draws debug info while entity edit mode is active. |

  Name pointers and cache addresses were checked in the live process, not taken
  from the debug-build CVar list.

## Live check

The live process had a non-null `gameLocal` pointer. First, only the integer and
float caches for `g_editEntityMode` and `g_showEditEntityInfo` were set to `1`;
the user saw no change. The test was repeated using the same three-field write
used by VoidCamera (16-byte CVar text buffer, integer cache, and float cache),
and the user again confirmed no visual change. Both CVars were then restored to
`0` in all three fields and read back as `0`. The strings and live CVar objects
are present in the retail executable, but changing them directly does not
activate visible entity-editor markers in this session.

## Interpretation and next step

There is no direct retail `addLight` command. The retail image contains RTTI for
`idEditEntities`, `idEditorLightInterface`, and game-editor interfaces, plus
editor CVar strings. However, changing the editor CVars in the live retail
process did not activate markers. The Doom 3 reference implementation is not
enough to establish that Dishonored 2 exposes the same input path. Further work
must trace the retail editor's activation/selection code or identify an actual
retail command before this can be offered as a usable light-spawn method.

Do not label arbitrary spot creation as implemented until the editor can clone
an existing spotlight and the result remains movable and renders correctly.
