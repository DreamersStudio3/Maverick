\# Input → Dodge → Action 흐름 (요약)



이 문서는 입력이 `InputManagerComponent`에서 수집되어 `MVDodgeComponent`가 이를 활용하고, 최종적으로 `MVActionComponent`가 액션(몽타주)을 재생하는 전체 흐름을 설명합니다. 파일/함수 목록 표기는 제외하고 도지 컴포넌트 활용 예제에 주요 함수명을 함께 기재합니다.



\## 전체 요약

\- 입력 수집: 입력 매니저는 이동 입력을 컨트롤러 공간으로 변환해 캐시하고, 액션 버튼 입력을 짧은 프레임 기반 버퍼에 저장합니다.

\- 입력 소비: 도지 컴포넌트는 InputManager handler로 등록되어 즉시 실행을 시도하거나, recovery window가 열릴 때 라우터를 통해 버퍼를 소비해 전환(또는 시작)합니다.

\- 액션 재생: 액션 컴포넌트는 데이터테이블에서 액션 row를 찾아 몽타주를 재생하고, 몽타주 종료 시 상태를 정리합니다.



\---



\## 상세: MVDodgeComponent 활용 예제 (함수명 포함, 단계별)



1\. 입력 발생 및 버퍼링

&#x20;  - 액션 버튼 입력 시 입력 매니저의 `SubmitActionInput(FGameplayTag)`가 호출되어 현재(또는 최근)의 컨트롤러-공간 이동 입력 스냅샷을 버퍼(`BufferedAction\*`)에 기록합니다.

&#x20;  - 버퍼는 프레임 기준으로 만료되며, 확인은 `TryGetBufferedActionInput`에서 수행합니다.



2\. 도지 컴포넌트의 handler 수신

&#x20;  - `BeginPlay`에서 도지 컴포넌트는 입력 매니저에 action input handler로 등록합니다.

&#x20;  - 입력 라우팅 시 `TryHandleActionInput`가 실행됩니다.

&#x20;    - 이벤트가 도지라면 `CacheControllerSpaceMovementInput`로 이동 입력을 캐시하고 `TryStartDodgeAction`를 즉시 시도합니다.

&#x20;    - 즉시 시작 실패 시(예: `TryStartDodgeAction`가 false 반환) 버퍼는 남아 recovery에서 소비될 수 있습니다.



3\. 즉시 시도 흐름 (`TryStartDodgeAction`)

&#x20;  - 존재/유효성 검사: 소유자 및 컴포넌트 참조 확인.

&#x20;  - 전환 가능성 검사: 현재 액션이 실행 중이면 `CanTransitionActiveDodgeAction`를 호출해 `IsRecoveryEscapeWindowOpen()` 및 `ActionComponent.CanInterruptActiveAction()` 조건을 확인합니다.

&#x20;  - 준비 단계: `PrepareDodgeAction`이 호출되어

&#x20;    - `CaptureControllerSpaceMovementInput` 또는 내부 캐시에서 입력을 가져와

&#x20;    - 캐릭터 회전 보정(시작 yaw) 및 chooser 문맥을 적용(`ApplyDodgeChooserSnapshot`)합니다.

&#x20;  - chooser/row 결정: `ResolveDodgeActionRowHandle` / `EvaluateDodgeChooserActionRowHandle`로 적절한 `FDataTableRowHandle`를 확보합니다.

&#x20;  - 비용 검사/소비: `CanConsumeDodgeCost`와 `ConsumeDodgeCost`로 스태미나 등 리소스 검증을 수행합니다.

&#x20;  - 액션 요청: 준비가 완료되면 `MVActionComponent`의 `TryStartActionFromRowHandle` 또는 `TryTransitionActionFromRowHandle`를 호출합니다.

&#x20;  - 성공 처리: 시작 성공 시 `InputManager->ClearBufferedActionInput()`를 호출하여 버퍼를 제거합니다.



4\. Recovery Escape Window 처리(전환 허용)

&#x20;  - 몽타주 내 `NotifyState`가 열리면 입력 매니저의 `BeginRecoveryEscapeWindow`가 호출되어 버퍼 입력을 등록된 handler 순서로 다시 라우팅합니다.

&#x20;  - 도지 컴포넌트의 `TryHandleRecoveryWindowOpened`는 도지 입력 처리 가능 여부를 판단합니다.

&#x20;    - InputManager는 `TryGetBufferedActionInput`으로 버퍼를 검증하고, 도지 handler는 `ActionInputTag`가 도지면 `CacheControllerSpaceMovementInput`을 갱신한 뒤 `TryStartDodgeAction`를 시도합니다.

&#x20;    - 성공하면 `ClearBufferedActionInput`를 호출해 소비를 확정합니다.



5\. NotifyState로 인한 입력 제어

&#x20;  - 몽타주 NotifyState의 `NotifyBegin`/`NotifyEnd`는 각각 입력 매니저의 `BeginMovementInputBlock`/`EndMovementInputBlock` 또는 `BeginRecoveryEscapeWindow`/`EndRecoveryEscapeWindow`를 호출합니다.

&#x20;  - movement-block 활성화 시 `IsMovementInputBlocked` 상태가 true가 되어 즉시 처리 경로를 제한할 수 있습니다.



6\. 액션 종료와 정리

&#x20;  - 몽타주가 끝나면 `MVActionComponent::HandleActionMontageEnded` → `FinishActiveAction`가 호출됩니다.

&#x20;  - `FinishActiveAction`은 로컬 상태 초기화 후 소유자 `InputManager`가 있으면 `ResetNotifyState`를 호출해 모든 notify 카운트를 초기화합니다.

&#x20;  - 스탯 관련 일시정지 해제는 `EndRecoverableStatRecoveryPause`에서 처리됩니다.



\---



\## 동작 요지(개념적)

\- 입력 매니저는 "무엇이 눌렸는지"(`SubmitActionInput`)와 "그때의 이동 방향(컨트롤러 공간)"을 함께 기록합니다.

\- 도지 컴포넌트는 해당 스냅샷을 도지 실행에 사용하고, 필요하면 캐릭터 회전/chooser 문맥을 적용해 몽타주 자산을 재활용합니다(`PrepareDodgeAction`, `ApplyDodgeChooserSnapshot`).

\- recovery window는 "후딜 동안 사용자가 입력한 의도를 소비하여 액션을 바꿀 수 있는 시점"을 제공하며 이를 위해 `BeginRecoveryEscapeWindow`/`OnRecoveryEscapeWindowChanged`가 사용됩니다.

\- 액션 종료 시 `FinishActiveAction`에서 `ResetNotifyState`를 호출해 상태 누적을 방지합니다.



\---



\## 디버깅 체크포인트 (도지 관련)

\- `HandleActionInputSubmitted` 호출 시 `CacheControllerSpaceMovementInput`가 올바르게 호출되는지(프레임 비교).

\- `TryStartDodgeAction`가 실패할 때 반환 원인(전환 거부, row 미해결, 스태미나 부족 등)을 로그로 확인.

\- recovery window 발생 시 InputManager가 buffered action input을 handler 순서로 다시 라우팅하는지 확인.

\- 액션 종료 시 `FinishActiveAction`가 `ResetNotifyState`를 호출해 `IsMovementInputBlocked`/`IsRecoveryEscapeWindowOpen`이 false가 되는지 검증.



\---



\## 권장 TODO

\- \[ ] 도지 입력 소비 실패 케이스에 대한 상세 로그 추가(`TryStartDodgeAction` 실패 분기)

\- \[ ] recovery window 경계(프레임 에지) 케이스 자동화 테스트 작성

\- \[ ] `PrepareDodgeAction` / `ApplyDodgeChooserSnapshot` 내부 동작 주석화 및 다이어그램 추가

\- \[ ] 문서화된 흐름을 README(또는 CONTRIBUTING)에 요약 섹션으로 통합



\---

