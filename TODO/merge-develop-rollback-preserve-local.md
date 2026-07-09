# Merge develop rollback while preserving local work

## Goal

- `origin/develop`의 rollback 변경을 현재 브랜치에 병합하되, 로컬에서 추가/수정한 적 전투 데이터와 C++ 확장은 최대한 보존한다.

## Checklist

- [x] 정책과 현재 워크트리 상태 확인
- [x] `origin/develop` 최신 fetch
- [x] incoming 변경과 로컬 변경 교집합 확인
- [x] 로컬 변경 백업
- [x] 로컬 변경 보존 커밋 생성
- [x] develop 병합 및 충돌 해결
- [x] 검증

## Decisions

- `Config/DefaultEngine.ini`: 최초 병합에서는 로컬 `MVAttackTarget` 채널을 보존했지만, 후속 요청에 따라 develop rollback 버전으로 되돌려 채널 삭제를 반영.
- `BP_ThirdPersonCharacter.uasset`: 최초 병합에서는 로컬 변경을 보존했지만, 후속 요청에 따라 develop rollback 버전으로 되돌림.
- `BP_MeleeAttack.uasset`, `BP_NameLessPuppet.uasset`: 로컬 변경이 없던 rollback 대상이라 develop(theirs) 선택.
- `Yone_Attack.uasset` 등 나머지 develop rollback 변경은 병합 결과 그대로 반영.
- 애니메이션, 신규 어빌리티 BP, NamelessPuppet StateTree 변경은 로컬 작업으로 유지.

## Result

- 로컬 변경 보존 커밋: `b0bf0a5`
- develop 병합 커밋: `c400695`
- `MaverickEditor Win64 Development` 빌드 성공.
- `stash@{0}`와 `backup/pre-develop-rollback-merge-20260709`는 병합 전 백업으로 남겨둔다.
