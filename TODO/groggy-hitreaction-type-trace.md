# Groggy HitReaction Type Trace

## Context

- Branch: `fix/enemy-hitreaction`
- Report: After moving groggy animation start from direct `ActionComponent::TryStartActionFromRowHandle` to `HitReactionComponent::HandleDamaged`, full groggy gauge appears to enter with the original hit data type instead of `Groggy`.
- Trace prefix: `GroggyTrace`

## Log Locations

- [x] `Source/Maverick/Combat/MVHitResolverSubsystem.cpp`
  - Check whether `BuildResolvedHitData` predicts full groggy and converts `OutHitData.HitReactionType` to `Groggy`.
- [x] `Source/Maverick/Components/MVStatComponent.cpp`
  - Check which `HitReactionType` stat damage handling receives, and whether `TryStartGroggy` is attempted.
- [x] `Source/Maverick/Components/MVHitReactionComponent.cpp`
  - Check which `HitReactionType` `HandleDamaged` receives, which action row is selected, and whether `ActionComponent` starts it.
  - Added rejection-stage logs after `HandleDamagedReceived` to identify early return reasons.

## Cleanup

- [x] Reproduced groggy conversion and action start with `GroggyTrace`.
- [x] Removed temporary `GroggyTrace` logs from `MVHitResolverSubsystem`, `MVStatComponent`, and `MVHitReactionComponent`.
