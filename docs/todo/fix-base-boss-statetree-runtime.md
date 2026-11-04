# BaseBoss StateTree 실행 복구

- 목표: `BP_BaseBoss`가 `ST_BaseAIStateTree`를 실행하고 테스트 레벨에서 플레이어 추적 가능한 구성 적용
- 상태: `BP_Test` 추적 이동 복구와 재추적 전이 검증 완료
- 결정: 사용자 제작 BaseBoss 자산 보존, `STT_Move.AIPawn`의 Context 속성을 유지하고 `Cast To Pawn`으로 `AI MoveTo.Pawn` 연결
- 위키 검토: 변경 불필요: 테스트용 Blueprint·레벨 구성 복구이며 장기 공용 설계 확정 범위 아님
- 검증: `BP_BaseBossAIController` 자동 소유와 컨트롤러의 `StateTreeAIComponent -> ST_BaseAIStateTree` 연결 확인; StateTree 스키마의 컨트롤러 타입 교정 후 런타임 `Missing external data requirements` 소멸 확인; `BP_Test`의 `StateTreeComponent` 자동 시작과 `AIController_0` 소유 확인; `STT_Move.AIPawn`을 `Cast To Pawn` 경유로 `AI MoveTo.Pawn`에 연결; `Delay` 완료 시 `Root` 재진입 전이 추가; `BaseBossTestLevel` PIE에서 플레이어 위치 변경 후 `BP_Test` 572.19cm 이동과 목표 거리 188.79cm까지 접근 확인
