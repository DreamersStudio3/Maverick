# Enemy Skill Action Trace

## Current Goal

Enemy skill attack states are entered, but the action task or action playback
does not run. Add temporary tracing first, then use the PIE log to identify the
failing branch.

## Added Temporary Logs

- [x] `Enemy Combat Action Task`
  - EnterState input values.
  - Owner/component resolution failures.
  - `TryStartAction` failure.
  - Cooldown start failure.
  - Started table/row when action starts.
- [x] `AMVEnemy`
  - `TryHeavyAttack` and `TrySkillAttack` bridge result.
- [x] `UMVCombatComponent`
  - `TryCombatAction` gate values.
  - `TrySkill` skill map, cooldown, cost, and action-start failures.
  - `TryStartActionWithAbility` owner/component/interrupt/start failures.
- [x] `UMVActionComponent`
  - Final resolved action playback failures.

## Next Check

- [ ] Compile after Live Coding is disabled or with `Ctrl+Alt+F11`.
- [ ] Run PIE and filter logs by `[ActionTrace]`.
- [ ] Remove or gate temporary logs after the failing branch is identified and
      fixed.

## Verification

- [ ] `MaverickEditor Win64 Development` build.
  - Blocked on 2026-07-12 because Unreal Live Coding is active.
