# Dodge Threat Evaluator Library

## Goal

- [x] Add a Blueprint-callable C++ evaluator that decides whether `STT_BindCombatActionStarted` should send the dodge StateTree event.
- [x] Keep event binding and `Send StateTree Event` in the existing Blueprint task.
- [x] Return both the decision and the `FMVAIDodgeRequest` payload so BP does not need to duplicate distance, angle, and direction logic.
- [ ] Wire `Evaluate Dodge Threat` in `STT_BindCombatActionStarted`.
- [ ] Verify compile after closing Live Coding.

## Notes

- The dodge State should only execute dodge. It should not be used as the first filtering point because entering it can interrupt the enemy's current AI behavior.
- The filter should run before sending `StateTreeEvent.Enemy.DodgeThreatDetected`.
- Build note: UHT header parsing passed, but full build is blocked while Live Coding is active.
