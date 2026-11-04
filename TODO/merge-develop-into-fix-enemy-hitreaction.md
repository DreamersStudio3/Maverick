# Merge Develop Into Fix Enemy Hitreaction

## Context

- Current branch: `fix/enemy-hitreaction`
- Request: Check conflicts before merging newly uploaded `develop` data into the local branch.

## Tasks

- [x] Confirm project policy.
- [x] Fetch latest remote refs.
- [x] Check committed branch merge conflicts.
- [x] Check incoming changes against local uncommitted files.
- [x] Merge `origin/develop` with develop-side conflict resolution.
- [x] Verify final status.

## Result

- Blocked by binary merge conflicts:
  - `Content/Characters/CharacterLogic/BP_ThirdPersonCharacter.uasset`
  - `Content/Characters/Components/Combat/Abilities/BP_MeleeAttack.uasset`
  - `Content/Characters/Components/Combat/Abilities/BP_ThirdSkill.uasset`
- Local uncommitted overlap with incoming `develop` changes:
  - `Content/Characters/Components/Combat/Abilities/BP_MeleeAttack.uasset`

## Merge

- Created merge commit: `98542a5`
- Resolved binary conflicts by taking `origin/develop` versions.

## Merge 2026-07-13 Follow-up

- Incoming commit: `3b5f760 Feat: Skill R 애니메이션 추가 및 적용 (#80)`
- Local uncommitted overlap with incoming `develop` changes:
  - `Content/Characters/NPC/Enemy/NamelessPuppet/StateTree/ST_NameLessPuppet.uasset`
  - `Content/Table/Attack/Player/Yone_Attack.uasset`
- Resolution policy: take `origin/develop` versions for overlapped binary assets, matching the previous merge instruction.
- Merge attempt blocked because Git could not unlink locked assets:
  - `Content/ArtAssets/Animations/PC/OneHand/Weapon/Yone/Skills/AS_1H_Yone_SkillB.uasset`
  - `Content/Table/Stat/DT_CharacterStat.uasset`
  - `Content/UI/Textures/Skills/Yone/T_Icon_Yone_SkillQ_1.uasset`
  - `Content/UI/Textures/Skills/Yone/T_Icon_Yone_SkillQ_2.uasset`
  - `Content/UI/Textures/Skills/Yone/T_Icon_Yone_SkillQ_3.uasset`
  - `Content/UI/Textures/Skills/Yone/T_Icon_Yone_SkillR.uasset`
- Close Unreal Editor or any asset editor holding those files, then retry the merge.
- Retry after closing the editor succeeded.
- Created merge commit: `691164d`
- Build verification: `MaverickEditor Win64 Development` succeeded.
