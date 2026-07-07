# Enemy Combat Component Task

## Current Goal

Enemy attacks must use `UMVCombatComponent` because CombatComponent owns the
ability setup that creates traces and checks hit collision.

Do not use the old direct row execution path for attacks that need ability
trace behavior.

## Done

- [x] Checked project naming style.
  - Functions that can fail while trying to start gameplay behavior use `Try*`.
  - Enemy combat bridge functions were named `TryHeavyAttack` and `TrySkillAttack`.
- [x] Added Blueprint-overridable functions to `AMVEnemy`.
  - `TryHeavyAttack(int32 ActionIndex, FName StartSection)`
  - `TrySkillAttack(int32 SkillIndex, FName StartSection)`
- [x] Added default C++ implementation for both functions.
  - `TryHeavyAttack(ActionIndex, StartSection)` calls `UMVCombatComponent::TryCombatAction(HeavyAttack, ActionIndex, StartSection)`.
  - `TrySkillAttack(SkillIndex, StartSection)` calls `UMVCombatComponent::TryCombatAction(Skill, SkillIndex, StartSection)`.
- [x] Added fixed execution StateTree task.
  - File: `Source/Maverick/AI/Task/MVEnemyCombatActionTask.h`
  - File: `Source/Maverick/AI/Task/MVEnemyCombatActionTask.cpp`
  - Display name: `Enemy Combat Action Task`
  - Supports `ActionKind = HeavyAttack` or `Skill`.
  - Supports `SkillIndex` when `ActionKind = Skill`.
  - Waits for the started ActionComponent action to finish by default.
  - Can start an optional `CooldownActionId`.
- [x] Verified build.
  - `MaverickEditor Win64 Development` build succeeded.
- [x] Changed enemy attack execution to support per-action StateTree states.
  - HeavyAttack and Skill both execute indexed rows through CombatComponent.
  - `Enemy Combat Action Task` shows its serialized `SkillIndex` property as `Action Index` for StateTree setup.
  - `MaverickEditor Win64 Development` build succeeded.
- [x] Added StartSection override to `Enemy Combat Action Task`.
  - The task can now pass `StartSection` through enemy bridge and CombatComponent to ActionComponent.
  - Leave `StartSection` empty to use the action row's `DefaultStartSection`.
  - Set `StartSection` on a transition-specific state when reusing an existing action row from a later montage section.
- [x] Added test AnimNotify for cooldown-gated action cuts.
  - File: `Source/Maverick/Animation/Notifies/MVAnimNotify_CooldownReadyActionCut.h`
  - File: `Source/Maverick/Animation/Notifies/MVAnimNotify_CooldownReadyActionCut.cpp`
  - Display name: `MV Cooldown Ready Action Cut`
  - If `CooldownActionId` is ready, the notify cancels the current action with the configured blend out time.
  - It does not choose or start the follow-up state; StateTree remains responsible for the next transition.
- [x] Added unified CombatContext-based StateTree enter condition.
  - File: `Source/Maverick/AI/Condition/MVCombatActionEnterCondition.h`
  - File: `Source/Maverick/AI/Condition/MVCombatActionEnterCondition.cpp`
  - Display name: `Combat Context Enter Condition`
  - Supports `Dead`, `Action`, `MoveToTarget`, `Strafe`, and `Idle` modes.
  - Uses `FMVAICombatContext` and `ReadyActionIds` instead of CombatComponent cooldown state.
  - Keeps exposed inputs minimal: mode, action type, cooldown action id, distance range, and optional combat area.

## Important Decision

`Skill` and `HeavyAttack` will not use the same selection flow.

### Skill

Skill uses the current fixed task flow.

Use `Enemy Combat Action Task` with:

- `ActionKind = Skill`
- `SkillIndex = N`

CombatComponent will look for row name `SkillN`.

Examples:

- `SkillIndex = 0` -> `Skill0`
- `SkillIndex = 1` -> `Skill1`

The Skill DataTable should be returned by the CombatComponent BP chooser when:

- `ChooserInput.ActionType = Skill`

For E1, this should return:

- `DT_E1_Skill`

### HeavyAttack

HeavyAttack uses one StateTree state per indexed attack row. Each state owns its
enter condition and follow-up transitions, while `Enemy Combat Action Task`
executes the configured index through CombatComponent.

## Current Task: Attack State Per Action

HeavyAttack selection will not stay hidden inside one shared select task.
StateTree should own each attack state and follow-up transition explicitly.

Required behavior:

1. StateTree enters a concrete attack state such as `HeavyAttack0` or `Skill1`.
2. `Enemy Combat Action Task` receives `ActionKind` and an index.
3. HeavyAttack and Skill execution both go through CombatComponent.
4. Ability/trace setup must still happen through CombatComponent.
5. Follow-up attacks, retreat, reposition, Dead/Hit/Groggy interrupts are modeled as StateTree transitions.

Use `Combat Context Enter Condition` on each concrete attack state:

- `Mode`: Action.
- `ActionType`: HeavyAttack or Skill.
- `CooldownActionId`: action cooldown id such as `HeavyAttack0`.
- `MinDistance` and `MaxDistance`: valid attack range.
- Optional combat area gate.

## Main Design Problem To Solve

Previous `UMVCombatComponent::TryCombatAction(HeavyAttack)` searched for row name:

```text
HeavyAttack
```

This works only when the HeavyAttack table has a single row named `HeavyAttack`.

If `DT_E1_HeavyAttack` has multiple rows such as:

```text
HeavyAttack1
HeavyAttack2
HeavyAttack3
```

then the old CombatComponent API could not execute a specific HeavyAttack row.

## Recommended Implementation Direction

Use indexed row names consistently:

- `HeavyAttack0`, `HeavyAttack1`, ...
- `Skill0`, `Skill1`, ...

`Enemy Combat Action Task` keeps the serialized `SkillIndex` property for asset
compatibility, but displays and uses it as `Action Index`.

Implementation result:

- `AMVEnemy::TryHeavyAttack(int32 ActionIndex, FName StartSection)` executes indexed HeavyAttack rows.
- `AMVEnemy::TrySkillAttack(int32 SkillIndex, FName StartSection)` executes indexed Skill rows.
- `UMVCombatComponent::TryCombatAction(ActionType, ActionIndex)` uses the index for HeavyAttack/LightAttack/ChargeAttack and Skill.
- Basic attack lookup tries indexed names first and falls back to legacy unindexed row names only for index 0.

## Alternative Implementation

If CombatComponent should not be changed yet, override `AMVEnemy::TryHeavyAttack`
in the E1 Blueprint and do HeavyAttack selection there.

This is faster but less clean:

- StateTree cannot see which HeavyAttack row was selected.
- Selection logic becomes hidden inside BP.
- Reuse for other enemies is weaker.

Use this only if C++ CombatComponent changes are too risky.

## StateTree Setup

Expected usage:

- Skill states:
  - Use `Enemy Combat Action Task`.
  - Set `ActionKind = Skill`.
  - Set `Action Index`.
- HeavyAttack states:
  - Use `Combat Context Enter Condition` to decide whether this concrete attack can start.
  - Set `Mode = Action`.
  - Set `CooldownActionId` to the row/cooldown id, such as `HeavyAttack0`.
  - Use `Enemy Combat Action Task`.
  - Set `ActionKind = HeavyAttack`.
  - Set `Action Index`.
  - Set `CooldownActionId` on the task if the action should start a cooldown when execution succeeds.

## E1 Data Setup Checklist

- [ ] E1 CombatComponent BP chooser returns `DT_E1_HeavyAttack` when
      `ActionType = HeavyAttack`.
- [ ] E1 CombatComponent BP chooser returns `DT_E1_Skill` when
      `ActionType = Skill`.
- [ ] `DT_E1_Skill` row names match CombatComponent SkillIndex rule:
      `Skill0`, `Skill1`, ...
- [x] `DT_E1_HeavyAttack` row names use indexed names such as `HeavyAttack0`.
- [ ] HeavyAttack state enter conditions define which row/state can run.
- [ ] Dead/Hit/Groggy transitions have higher priority than attack transitions.

## Files Already Touched

- `Source/Maverick/Character/NPC/Enemy/MVEnemy.h`
- `Source/Maverick/Character/NPC/Enemy/MVEnemy.cpp`
- `Source/Maverick/AI/Task/MVEnemyCombatActionTask.h`
- `Source/Maverick/AI/Task/MVEnemyCombatActionTask.cpp`
- `TODO/enemy-combat-component-task.md`
