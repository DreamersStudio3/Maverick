# 전투 로그 버그 수정

- [x] `POLICY.md` 확인
- [x] 첨부 로그와 현재 에셋 경로 대조
- [x] CombatComponent 초기화 경로 추적 로그 수집
- [x] 이동된 Yone 공격 테이블 fallback 경로 갱신
- [x] NPC에 플레이어 전용 Chooser/스킬이 등록되는 초기화 경로 수정
- [x] Skill DataTable 비용 필드 기본값 초기화
- [x] BP_Carcass의 삭제된 DodgeComponent 참조 정리
- [x] TestMap에 남은 MVDodgeComponent import 정리 후 맵 재저장
- [x] CommonUI GameViewportClient 설정 섹션 수정
- [x] 에디터 타깃 빌드 검증
- [x] TestMap 헤드리스 실행 로그에서 오류 제거 확인
- [x] 임시 추적 로그 제거

## 검증 메모

- `MaverickEditor Win64 Development` UHT/컴파일/링크 성공
- `TestMap` 헤드리스 게임 실행에서 fallback 실패, 구 Yone 경로, 미초기화 struct 멤버, MVDodgeComponent linker warning, CommonUI viewport 오류가 모두 0건
- 플레이어 LightAttack과 SkillQ 등록 확인
- `TestMap.umap` 재저장 후 Content 전체에서 `MVDodgeComponent` import 문자열이 더 이상 검출되지 않음
- 맵 재저장 후 새 프로세스로 실행한 TestMap 로그에서도 legacy dodge, fallback, CommonUI, 미초기화 오류가 모두 0건
