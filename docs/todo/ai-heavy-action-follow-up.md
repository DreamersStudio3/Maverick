# AI Heavy 액션 후속 정리

- 목표: 공용 Heavy 정의 제거 이후 AI의 진입 조건·액션 선택·위협 판정 복구
- 상태: 플레이어 Light·Charge 통합 완료. 에디터 빌드에서 확인된 공용 Heavy 의존 지점만 비활성화
- 결정: AI 대체 공격과 선택 정책은 이 작업에서 확정
- 위키 검토: 변경 불필요: AI 대체 정책 확정 후 검토
- 검증: 플레이어 변경 포함 Editor 빌드 통과. 아래 AI 경로의 런타임 검증 대기

| 위치 | 비활성 동작·후속 검토 |
|---|---|
| `AI/Condition/MVCombatActionEnterCondition.h:41` | 기본 Heavy 진입 조건이 `None`으로 초기화 |
| `AI/Task/MVEnemyCombatActionTask.cpp:48` | 적 태스크의 기본 Heavy 실행 요청이 실패 반환. 태스크 자체의 직렬화 타입과 기본값 검토 |
| `AI/Task/MVExecuteAttackTask.cpp:53` | 일반·회피·질주 Heavy 행 분류에서 빈 태그 반환. `CHT_E1_Attack`의 공용 Heavy 태그 참조 확인 |
| `Private/AI/MVDodgeThreatEvaluatorLibrary.cpp:13,76,94` | Heavy 계열을 기본 위협 목록과 분류에서 제외 |
| `Editor/MaverickBossStateTreeCommandlet.cpp:389,483` | 보스 StateTree의 문자열 패턴명과 Row 연결 확인 |

경로 기준 `Source/Maverick/`
