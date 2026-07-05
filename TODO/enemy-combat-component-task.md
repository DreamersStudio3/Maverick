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
  - `TryHeavyAttack()`
  - `TrySkillAttack(int32 SkillIndex)`
- [x] Added default C++ implementation for both functions.
  - `TryHeavyAttack()` calls `UMVCombatComponent::TryCombatAction(HeavyAttack)`.
  - `TrySkillAttack(SkillIndex)` calls `UMVCombatComponent::TryCombatAction(Skill, SkillIndex)`.
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

HeavyAttack must keep the old SkillAttack state behavior:

- Enter one `SkillAttack` state.
- Inside that state, evaluate candidate conditions.
- Select one HeavyAttack candidate automatically.
- Execute the selected HeavyAttack through CombatComponent.

This is different from the fixed `Enemy Combat Action Task`.

The old `Select And Execute Attack Task` already has useful selection logic, but
it starts actions by passing a selected row handle directly to `ActionComponent`.
That bypasses CombatComponent, so it should not be used as-is for HeavyAttack.

## Tomorrow Task: HeavyAttack Select Through CombatComponent

Create a HeavyAttack select path that keeps the old selection behavior but
executes through CombatComponent.

Required behavior:

1. StateTree enters the existing `SkillAttack` state.
2. New select task evaluates HeavyAttack candidates using conditions similar to
   the old `Select And Execute Attack Task`.
3. The selected candidate resolves to a HeavyAttack row or HeavyAttack selection
   key.
4. Execution must go through CombatComponent, not direct ActionComponent row
   execution.
5. Ability/trace setup must still happen through CombatComponent.

## Main Design Problem To Solve

Current `UMVCombatComponent::TryCombatAction(HeavyAttack)` searches for row name:

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

then the current CombatComponent API cannot execute a selected HeavyAttack row
by name.

Tomorrow's implementation must decide how selected HeavyAttack rows are passed
into CombatComponent.

## Recommended Implementation Direction

Prefer adding a CombatComponent API that can execute a selected row name through
the existing CombatComponent data/ability path.

Possible function shape:

```cpp
bool UMVCombatComponent::TryCombatActionByRowName(
    EMVCombatActionTypes InActionType,
    FName RowName);
```

Expected behavior:

- Uses the same chooser table path as current CombatComponent.
- Gets the DataTable for `InActionType`.
- Finds the selected row in the already built map, or resolves it from the table.
- Runs cost checks.
- Sends the selected row to ActionComponent through CombatComponent.
- Keeps ability instance/trace setup valid.

Then add an Enemy bridge:

```cpp
UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Maverick|Enemy|Combat")
bool TryHeavyAttackByRowName(FName RowName);
```

Default implementation:

```cpp
return CombatComponent
    && CombatComponent->TryCombatActionByRowName(
        EMVCombatActionTypes::HeavyAttack,
        RowName);
```

The new HeavyAttack select task can then:

```text
select candidate -> selected row name -> Enemy.TryHeavyAttackByRowName(RowName)
```

## Alternative Implementation

If CombatComponent should not be changed yet, override `AMVEnemy::TryHeavyAttack`
in the E1 Blueprint and do HeavyAttack selection there.

This is faster but less clean:

- StateTree cannot see which HeavyAttack row was selected.
- Selection logic becomes hidden inside BP.
- Reuse for other enemies is weaker.

Use this only if C++ CombatComponent changes are too risky.

## StateTree Setup After Tomorrow Task

Expected usage:

- Skill states:
  - Use `Enemy Combat Action Task`.
  - Set `ActionKind = Skill`.
  - Set `SkillIndex`.
- HeavyAttack/old SkillAttack state:
  - Use new HeavyAttack select task.
  - Candidate list chooses which HeavyAttack row to execute.
  - Selected HeavyAttack must execute through CombatComponent.

## E1 Data Setup Checklist

- [ ] E1 CombatComponent BP chooser returns `DT_E1_HeavyAttack` when
      `ActionType = HeavyAttack`.
- [ ] E1 CombatComponent BP chooser returns `DT_E1_Skill` when
      `ActionType = Skill`.
- [ ] `DT_E1_Skill` row names match CombatComponent SkillIndex rule:
      `Skill0`, `Skill1`, ...
- [ ] `DT_E1_HeavyAttack` row names are documented before implementing select.
- [ ] HeavyAttack candidate conditions define which row to choose.
- [ ] Dead/Hit/Groggy transitions have higher priority than attack transitions.

## Files Already Touched

- `Source/Maverick/Character/NPC/Enemy/MVEnemy.h`
- `Source/Maverick/Character/NPC/Enemy/MVEnemy.cpp`
- `Source/Maverick/AI/Task/MVEnemyCombatActionTask.h`
- `Source/Maverick/AI/Task/MVEnemyCombatActionTask.cpp`
- `TODO/enemy-combat-component-task.md`
