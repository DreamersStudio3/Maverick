# Enemy Death Lock-On Clear

## Goal

- [x] Inspect LockOnTarget plugin APIs used by the player.
- [x] Add Enemy Dead Task logic to clear player lock-on when the dead enemy is the current target.
- [x] Disable the dead enemy target component capture state on dead state enter.
- [x] Build MaverickEditor and verify compile.

## Notes

- `ULockOnTargetComponent::ClearTargetManual()` releases the active target through the plugin API.
- `UTargetComponent::SetCanBeCaptured(false)` prevents the dead enemy from being captured again and dispatches invalidation to remaining invaders.
