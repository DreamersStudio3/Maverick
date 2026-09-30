# 보스 공격 Task 분리

- 목표: `Maverick/AI/AITask`에 최소 보스 공격 실행 Task 추가
- 상태: 두 보스 공격 StateTree에 임시 5종 공격 State와 새 Task 배치·저장 완료
- 결정: 이름 `FMVBossExecuteAttackTask`, 공격 row 직접 실행 방식
- 위키 검토: 변경 불필요: 구현·검증 후 공격 StateTree 구조 문서 영향 재검토
- 검증: 기존 Row와 몽타주 매핑 확인; `MaverickEditor` 빌드 성공; Commandlet로 두 StateTree compile·저장 성공; PIE 실전 검증 미실행
