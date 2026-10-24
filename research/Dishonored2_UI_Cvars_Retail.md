# Dishonored 2 retail UI CVar correspondences

Build: Dishonored 2 retail 1.77.9.0, image SHA256 `b671f8249422862bf557d00aab2f4bb671ed4515cc10ad8a8a9decad84516180`.

These are runtime correspondences resolved from CVar name pointers in the live process image. They were not taken from the debug-build CVar dump. At each object RVA, the name pointer is at `+0x30`, integer cache at `+0x28`, and float cache at `+0x2C`. The runtime scan validated the name string at each object.

| CVar | Object RVA | Runtime int / float observed in process scan |
|---|---:|---:|
| `g_showHud` | `0x39A3610` | `1 / 1` |
| `g_showGuis` | `0x39A3810` | `1 / 1` |
| `ark_show_ui` | `0x398EBB0` | `0 / 0` |
| `ark_hud_enable_keyhole` | `0x397F2A0` | `1 / 1` |
| `ark_ui_crosshair_opacity` | `0x398A000` | `1 / 1` |
| `ark_ui_show_awareness_markers` | `0x3985BC0` | `1 / 1` |
| `ark_ui_show_contextual_icons` | `0x397D380` | `1 / 1` |
| `ark_ui_show_firelock_markers` | `0x3995FC0` | `1 / 1` |
| `ark_ui_show_grenade_cooking_gauge` | `0x397F220` | `1 / 1` |
| `ark_ui_show_grenade_markers` | `0x397F1A0` | `1 / 1` |
| `ark_ui_show_heart_markers` | `0x3989C20` | `1 / 1` |
| `ark_ui_show_information_messages` | `0x397D0A0` | `1 / 1` |
| `ark_ui_show_interactions` | `0x397D300` | `1 / 1` |
| `ark_ui_show_location_discovery` | `0x3989840` | `1 / 1` |
| `ark_ui_show_master_hud` | `0x398EC30` | `2 / 2` |
| `ark_ui_show_notifications` | `0x397F410` | `1 / 1` |
| `ark_ui_show_objective_markers` | `0x3989BA0` | `1 / 1` |
| `ark_ui_show_objective_notifications` | `0x3989AA0` | `1 / 1` |
| `ark_ui_show_pickup_log` | `0x398A080` | `1 / 1` |
| `ark_ui_show_shortcuts` | `0x397F680` | `1 / 1` |
| `ark_ui_show_snap_reaction_gauge` | `0x397D410` | `1 / 1` |
| `ark_ui_show_target_notifications` | `0x3989B20` | `1 / 1` |
| `ark_ui_show_tutorial_messages` | `0x3981ED0` | `1 / 1` |

The screenshot with the fixed HUD hidden still showed floating objective and awareness markers. The injected 0.8 process had `ark_show_ui=0`, but the Arkane marker CVar values remained enabled. Version 0.9 resolves the table above by exact name, saves each integer/float value, sets the relevant UI visibility values to zero, and restores the saved values when the toggle is released. `ark_ui_show_master_hud` has runtime value 2, so the zero/restore behavior still needs visual confirmation for that entry.

Other retail strings under `ark_ui_*` were found but did not resolve to a live static CVar object in this scan, or are debug/update settings rather than visibility controls. They are not written by the tool.
