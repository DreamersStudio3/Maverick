# Enemy Dodge Token Component

## Goal

- [x] Add an enemy-only dodge token component.
- [x] Grant one dodge token from combat flow events:
  - groggy ends by recovery, not finisher
  - enemy receives enough hits
  - enemy lands enough hits on player
- [x] Consume one token only after dodge threat evaluation passes.
- [x] Keep `MVEnemyDodgeActionTask` as execution-only; do not move tactical filtering there.
- [x] Document Blueprint/StateTree node wiring for `STT_BindCombatActionStarted`.
- [x] Verify C++ build reached UHT before Live Coding blocked full compile.
- [x] Remove temporary ActionTrace logs added during diagnosis.
- [x] Show token grants with an on-screen debug message.
- [x] Bind enemy StatComponent groggy events into enemy dodge token flow.
- [x] Add a PIE action test button that grants one enemy dodge token to the current target.

## Notes

- The component observes existing damage/groggy events where possible.
- Blueprint wiring should call one compact API that returns `FMVAIDodgeRequest` for the StateTree event payload.
- Full compile is currently blocked while Live Coding is active; use Ctrl+Alt+F11 in the editor or close the editor before building.
