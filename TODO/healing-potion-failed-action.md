# Healing Potion Failed Action

## 목표

- 회복물약 보유 개수가 0일 때 입력을 무시하지 않고 Item Failed 액션을 재생한다.

## 진행

- [x] `FMVHealingPotionTableRow`에 실패 액션용 row handle과 start section 추가.
- [x] 회복약 컴포넌트에서 성공/실패 액션 모두 ActionComponent row handle 경로로 실행하도록 정리.
- [x] 기본 실패 액션 row는 `DT_Props_P1`의 `Items_Failed`를 사용하도록 fallback 지정.
- [x] `DT_Props_P1`에 `Items_Failed` row와 `AM_Props_Items_Failed` 몽타주 참조가 저장된 것 확인.

## 확인 필요

- 빌드 후 에디터에서 회복물약 카운트 0 상태로 입력했을 때 실패 몽타주가 재생되는지 확인한다.
