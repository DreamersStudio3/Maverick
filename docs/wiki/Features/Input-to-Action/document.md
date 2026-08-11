---
제목: 입력에서 Action 실행까지
부제목: 입력 수집부터 Action·Ability 활성 구간까지의 실행 경계
최근수정일: 2026-08-11
최근수정자: 곽민규
관련문서:
  - "[[Architecture/document|Maverick Architecture]]"
  - "[[Features/Combat/S1-Tempo-Charge/document|S1 Tempo 공통 Charge·연속기 구현안]]"
  - "[[Research/Combat-Design-MDA/document|Maverick 전투 MDA와 목표 전투 계약]]"
---
# 입력에서 Action 실행까지

1. Blueprint/Enhanced Input이 `UMVInputManagerComponent::SubmitActionInput` 또는 `SubmitHoldActionInput`을 호출한다. `에셋 확인 필요`.
2. InputManager가 controller-space 이동 입력을 snapshot하고 `IMVActionInputHandlerInterface` 구현자를 우선순위 순으로 호출한다.
3. 첫 번째 성공한 handler가 입력을 소비한다. Combat, Dodge, Consumable, HitReaction recovery 등이 같은 문맥을 공유한다.
4. CombatComponent는 공격 태그와 Chooser/fallback 데이터로 Action Row를 고르고 chain/Ability 상태를 준비한다.
5. ActionComponent는 선택된 Row의 Montage, active action, interruptibility와 종료 이벤트를 소유한다.
6. Ability NotifyState가 Montage의 실제 공격 활성 구간에서 Ability를 열고 닫는다.

경계 원칙: CombatComponent는 선택기이고 ActionComponent는 실행기다. 입력 버퍼에 개별 도메인 규칙을 추가하지 않는다.
