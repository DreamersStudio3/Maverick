# Enemy Ignore Dead Target

## Goal

- [x] Add dead target filtering to `MVGlobalSensingTask`.
- [x] Clear AIController target when the sensed target is dead.
- [x] Build MaverickEditor and verify compile.

## Notes

- Dead targets are actors with `UMVStatComponent::IsDead() == true`.
- The sensing snapshot clears target, line of sight, movement path, and attack context outputs so StateTree conditions can fall back to idle/no-target behavior.
