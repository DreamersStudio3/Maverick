# Groggy HitReaction Policy

## Context

- Branch: `fix/enemy-hitreaction`
- Goal: Let the groggy enter animation play through `HitReactionComponent`, then continue broader groggy behavior after the animation is verified.

## Policy

- `StatComponent` owns groggy state and gauge values.
- `HitReactionComponent` owns hit reaction animation selection and blocking policy.
- `Groggy` hit reaction must pass the invincibility gate so the groggy enter animation can interrupt prior hit reaction invincibility windows.
- While `StatComponent::IsGroggy()` is true, regular hit reaction animations must not steal playback from the groggy state.
- Finisher/front-grab skill animations must remain possible because they are not regular `HitReactionComponent::HandleDamaged` hit reaction playback.

## Tasks

- [x] Record groggy hit reaction ownership policy.
- [x] Allow `Groggy` hit reaction through the invincibility gate.
- [x] Block non-groggy hit reaction animation while the target is already groggy.
- [x] Route groggy hit reaction to `Groggy_E1 / Groggy_E1_Start_Loop` instead of missing `HR_E1 / Groggy_F`.
- [x] Treat `Groggy_*` action tables as hit reaction actions inside `MVHitReactionTask`, so StateTree waits for the groggy action instead of falling through to movement.
- [x] Avoid adding fields to StateTree-bound context structs for groggy blocking; it can break existing StateTree asset links.
- [x] Verify `MaverickEditor` build.
- [x] Reproduce in PIE and confirm groggy animation plays.
- [x] Remove temporary `GroggyTrace` logs after verification.
