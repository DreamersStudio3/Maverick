# Attack Opportunity Task

## Goal

- [x] Add a simple StateTree task that can complete while MoveTo is still running.
- [x] Evaluate attack opportunity from `FMVAICombatContext`.
- [x] Support distance, angle, line of sight, action cooldown, and attack cadence checks.
- [x] Expose `bCanAttack` for StateTree transition checks.
- [x] Verify UHT generation.

## Notes

- Put this task beside MoveTo in a StateTree state with task completion mode set to Any.
- Bind `CombatContext` from `GlobalSensingTask`.
- Full compile is blocked while Live Coding is active; use Ctrl+Alt+F11 in the editor or close the editor before building.
