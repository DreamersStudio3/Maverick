# Groggy Gauge Decay Regression Check

## Context

- Branch: `fix/enemy-hitreaction`
- Report: Groggy gauge should decay a few seconds after no further attacks, but it appears not to apply.
- Follow-up rule: Full groggy gauge must not decay automatically.

## Tasks

- [x] Confirm project policy.
- [x] Locate groggy gauge increase and decay runtime path.
- [x] Skip temporary trace logs because the static runtime path exposed the early return condition.
- [x] Identify why inactive groggy gauge decay is not applying.
- [x] Apply focused fix.
- [x] Keep full groggy gauge out of automatic decay.
- [x] Verify with build or targeted test where feasible.
- [x] Fix active groggy recovery so `bIsGroggy` can recover even when `CurrentGroggy >= MaxGroggy`.
- [x] Preserve delayed inactive decay: after recent damage, if no more attacks arrive until `RecentDamageResetDelay` expires, partial groggy gauge decreases.
- [x] Preserve full inactive gauge guard: maxed gauge does not decay unless the stat component has actually entered groggy state.
- [x] Keep HP damage active during groggy, but skip additional groggy gauge gain and recovery cooldown reset while already groggy.
