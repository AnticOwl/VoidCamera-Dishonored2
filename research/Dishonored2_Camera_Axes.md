# Dishonored 2 camera axes

The user reports that the supplied axis formula still lets pitch/yaw and
left/right versus up/down appear to exchange at 90 degrees of camera roll.
The UI terms identify pose angles, so update them independently rather than
composing incremental rotations in the already rolled frame.

## Current 1.44 control model

- `Pitch` updates `pose.pyr[0]`; `Yaw` updates `pose.pyr[1]`; `Roll` updates
  `pose.pyr[2]`. Pitch is clamped below the Euler singularity; yaw and roll
  wrap at 360 degrees.
- The rendered orientation is rebuilt using the existing `fromEuler()` mapping.
- Translation builds a movement matrix from current pitch and yaw with roll
  set to zero. Forward and left/right use its first two rows; up/down remains
  world Z. Thus changing only roll does not turn side movement into height or
  vice versa. Keyboard and XInput feed this same path.
- IGCS multishot retains its separate Connector camera-plane basis.

The code compiles, but this mapping still needs directional confirmation in
game, especially after applying a 90-degree roll.
