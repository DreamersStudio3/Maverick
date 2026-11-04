# Enemy airborne recovery decision

## Goal

- Reuse the player airborne recovery flow for enemies.
- Keep HitReactionComponent responsible for common recovery execution.
- Let enemy-specific code decide Getup vs EscapeDodge when the recovery window opens.

## Plan

- [x] Check existing player hit reaction and recovery flow.
- [x] Add a C++ recovery decision provider interface.
- [x] Let HitReactionComponent call the provider on recovery window open.
- [x] Implement the provider in AMVEnemy using target distance and direction.
- [x] Verify build.

## Notes

- Continue using `DT_HR_E1`.
- Far target: existing Getup recovery.
- Close target: EscapeDodge away from the target.
