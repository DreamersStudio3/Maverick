# Stat Recovery Refactor TODO

스탯 회복 쿨다운과 액션 회복 일시정지 책임을 `AMVCharacterBase`에서 `UMVStatComponent`로 옮기는 작업을 관리한다.

- [x] `POLICY.md`와 현재 컴포넌트 결합 지점을 확인한다.
- [x] `UMVStatComponent`가 회복 쿨다운, 회복 일시정지, 최근 감소 홀드 이벤트를 소유하게 한다.
- [x] `AMVCharacterBase`에서 액션 델리게이트 중계와 회복 쿨다운 상태를 제거한다.
- [x] `UMVActionComponent`가 액션 회복 일시정지를 `UMVStatComponent`에 직접 반영하게 한다.
- [x] HUD 바인딩을 캐릭터 이벤트가 아니라 스탯 컴포넌트 이벤트로 옮긴다.
- [x] 변경 후 참조 검색과 빌드로 확인한다.
- [x] `MVStatComponent.cpp` 로컬 스탯 보정 헬퍼를 복구한다.
