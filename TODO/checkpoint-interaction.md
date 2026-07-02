# Checkpoint / Interaction Framework Review

체크포인트 BP 구현 전 현재 Interactable 기반 구조가 액터별 커스터마이즈를 지원하는지 확인한다.

- [x] `POLICY.md` 확인
- [x] `UMVInteractableComponent`와 `IMVInteractableInterface` 책임 확인
- [x] 플레이어 `UMVPlayerInteractionDetector` 감지/프롬프트/상호작용 실행 흐름 확인
- [x] PIE 피격 테스트와 `BP_Carcass` 임시 연결 지점 확인
- [x] UI 레이어/윈도우/팝업 구조와 메뉴 트리 구현 여부 확인
- [x] 체크포인트/상호작용 일반화 구현 방향 확정
- [x] 정의 기반 `UMVInteractableComponent` 실행 모델 추가
- [x] 공통 상호작용 세션으로 prompt 억제 상태 일반화
- [x] CommonUI 기반 상호작용 메뉴 윈도우 추가
- [x] 비활성/활성/메뉴 흐름을 가진 네이티브 체크포인트 액터 추가
- [x] `/Game/Interaction/BP_Checkpoint` 블루프린트 에셋 생성
- [x] detector의 `BP_Carcass`/PIE 피격 테스트 특수 처리 제거
- [x] legacy PIE 피격 테스트 패널을 기본 비활성 CVar 뒤로 이동
- [x] `MaverickEditor Win64 Development` 빌드 통과
- [x] 활성화된 체크포인트 목록을 월드 저장 데이터에 추가
- [x] 기존 마지막 체크포인트 저장 데이터를 활성 목록으로 정규화
- [x] 체크포인트 이동 메뉴를 하위 메뉴 트리로 구성
- [x] 체크포인트 이동 항목을 필드 전환 흐름에 연결
- [x] 상호작용 액션 완료 모델에서 `Instant`/`Delay` 자동 완료 제거
- [x] 상호작용 액션과 체크포인트 활성화를 notify/콜백 기반 수동 완료로 단순화
- [x] BP 호환용 deprecated 완료 alias 제거
- [x] `Entry + FollowUp` 고정 길이 모델을 step 기반 interaction flow로 확장
- [x] 메뉴/선택지 항목별 `TriggerName -> NextStepId` 분기 연결
- [x] 액션 step에 `StepId`/`ActionName` 기반 시작/완료 delegate 추가
- [x] 재사용 가능한 `UMVInteractionFlowDataAsset` 추가
- [x] InteractableComponent가 flow asset 또는 inline definition을 실행하도록 확장
