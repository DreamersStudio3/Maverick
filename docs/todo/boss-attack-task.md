# 보스 공격 Task 분리

- 목표: `Maverick/AI/AITask`에 최소 보스 공격 실행 Task 추가
- 상태: 기존 `DT_E1_Attack` Row와 보스 패턴 매핑 확인, 새 Task 컴파일 성공, StateTree 배치 대기
- 결정: 이름 `FMVBossExecuteAttackTask`, 공격 row 직접 실행 방식
- 위키 검토: 변경 불필요: 구현·검증 후 공격 StateTree 구조 문서 영향 재검토
- 검증: 기존 Row와 몽타주 매핑 확인; `MaverickEditor` 빌드 성공; StateTree 에셋 배치 미검증
