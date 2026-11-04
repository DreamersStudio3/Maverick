# Enemy Death Boss HUD Hide

## Goal

- [x] Keep boss HUD hide ownership on `AMVEnemy`, which owns boss HUD binding.
- [x] Stop pending boss HUD bind retries when the enemy dies.
- [x] Hide the bound boss HP widget from `EnemyDeadTask` through the enemy API.
- [x] Build MaverickEditor and verify compile.

## Notes

- `UMVMainHUDWidget::HideBossHPBar()` resets the boss widget and unbinds stat delegates through `UMVBossHPBarWidget::ResetBossBar()`.
- `AMVEnemy::HideBoundBossHUD()` clears pending bind retries so the dead enemy cannot rebind the boss HUD after entering dead state.
