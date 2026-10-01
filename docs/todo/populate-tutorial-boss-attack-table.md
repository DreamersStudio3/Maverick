# TutorialBoss 공격 데이터 표 연결

- 목표: `DT_NewDataTable`에 `FMVActionRow` 기반 공격 몽타주 행을 채우고 `ST_TutorialBoss_Attack`의 실행 행 연결
- 상태: `DT_NewDataTable` 값 입력·저장 완료
- 결정: 기존 검증된 `NamlessPuppet` 공격 몽타주를 `BasicAttack1~3`, `SkillQ`, `SkillW`, `Resonance`에 연결
- 위키 검토: `변경 불필요: 기존 AI StateTree·ActionComponent 문서 범위에 포함`
- 검증: UE 5.8 명령let에서 표 저장 로그 확인, 파일 크기 2657→4260 바이트 확인; 전체 명령let은 기존 `ST_BaseAIStateTree` 오류로 종료 코드 1
