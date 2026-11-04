# Enemy Respawn Reset

## Goal

- [x] Confirm player death respawn uses `UMVFieldTransitionSubsystem` field actor reset.
- [x] Implement `IMVFieldTransitionResettableInterface` on `AMVEnemy`.
- [x] Reset enemy runtime state without changing actor transform.
- [x] Restore enemy weapon actor when `WeaponClass` is configured.
- [x] Restart enemy StateTree logic during respawn reset.
- [x] Build MaverickEditor and verify compile.

## Notes

- Position reset is intentionally excluded for a later change.
- Reset restores death/stat/action/capture/visibility/collision/tick/movement state.
- Weapon actor restore uses existing `WeaponClass`, `bUseDualWeapon`, and `AMVEnemyWeapon` attach helpers.
- StateTree reset clears AI target/focus/movement, then restarts StateTree components on the AIController or pawn.
