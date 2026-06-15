# World 1: Scenario, Playability and Walkthrough

Updated: 2026-06-12

## Scope and Evidence

- Source of truth: `levels/world1/level_01.tmx` through `level_20.tmx`.
- Player uses the standard World 1 movement set.
- Standard jump still reaches the validated gaps used in the redesigned routes.
- Technical acceptance means the room has a legal route, traps function, death/restart works, and the exit can be reached without an exploit.

## Trap Rules Used in World 1

| Trap | Rule and safe response |
|---|---|
| Vanish Floor | Keep moving; jump before the strip disappears. |
| Pop Spike | Bait it, wait for retraction, or cross airborne. |
| Sand Sinkhole | Do not stand on it; move or hop through. |
| Falling Block | Trigger it, leave its column, and use the landed state only when the route asks for it. |
| Boulder | Read the trigger and jump or outrun it with height. |

## Overall Verdict

| Room | Difficulty | Technical verdict | Main note |
|---:|---:|---|---|
| 01 | 2/10 | PASS | Vanish Floor with visible spike punishment |
| 02 | 2/10 | PASS | Pop Spike introduction |
| 03 | 3/10 | PASS | Sand rhythm with safe gaps |
| 04 | 3/10 | PASS | Boulder trigger and jump timing |
| 05 | 4/10 | PASS | Falling Block gap commitment |
| 06 | 4/10 | PASS | Vanish runway into Pop Spike |
| 07 | 5/10 | PASS | Sand rhythm under Boulder pressure |
| 08 | 5/10 | PASS | Falling gaps and landing Pop Spike |
| 09 | 6/10 | PASS | Two Vanish strips under chase pressure |
| 10 | 6/10 | PASS | Order break between Sand and Falling |
| 11 | 6/10 | PASS | Chase remix with separated decisions |
| 12 | 6/10 | PASS | Short pockets between Falling, Sand, and Pop |
| 13 | 7/10 | PASS | Alternating terrain chase relay |
| 14 | 7/10 | PASS | Falling-step setup under pressure |
| 15 | 7/10 | PASS | Vertical terrain decay with no easy bypass |
| 16 | 8/10 | PASS | Vertical pursuit with Pop Spike commits |
| 17 | 8/10 | PASS | Three rhythm betrayal beats |
| 18 | 8/10 | PASS | Falling-step route construction |
| 19 | 8/10 | PASS | Tight recap with recovery pockets |
| 20 | 9/10 | PASS | Boss trial with chase, climb, and payoff |

## Walkthrough Notes

1. Room 05: shake the blocks, leave the column, then commit to the gap jump.
2. Room 08: the Pop Spike sits on the landing side, so the room teaches “land, read, continue.”
3. Room 09: the Boulder pressure means waiting is worse than committing to the cracked routes.
4. Room 14: the Falling Blocks are part of the lesson, not just decoration.
5. Room 16 and 20: climb routes are legal at standard jump height, but the player must keep moving through Pop Spike commit points.

## Remaining Human Playtest Checklist

- Attempts to first clear for every room.
- Learned clear time under 30 seconds.
- Whether each death cause is correctly identified after the first encounter.
- Room 14 understanding of the Falling Block step logic.
- Room 18 understanding of the constructed route.
- Room 20 deaths by phase and whether the Boulder grace feels fair.
