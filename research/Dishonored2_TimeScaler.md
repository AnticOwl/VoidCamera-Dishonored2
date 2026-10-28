# Dishonored 2 time scale CVar — unsuccessful UI test

Retail executable: Dishonored 2 1.77.9.0 (the executable identity verified by VoidCamera).

Ghidra's CVar registration identifies `com_timeScaler` at object RVA `0x25B1E00`, with the description `scales the time`. Its default string points to `2`; the live retail process also showed text and float caches both equal to `2`. There are no registered `ark_` CVars matching time, slow motion, scale, or pause. A game call site applies time-scale values through `FUN_140102880(&DAT_1425b1e00, value, 1)`, so the supported build uses the native float CVar setter at RVA `0x102880`.

The attempted VoidCamera control was removed in 1.40. During the live test, the UI showed `0.1` while the game's CVar text, integer cache, and float cache still read `2`. Thus the native setter call at `0x102880` did not apply the change as called by the prototype. No `ark_` time/slow-motion CVar was found in the available retail registration output. Keep the feature out of the UI unless a correct native command or setter path is found.
