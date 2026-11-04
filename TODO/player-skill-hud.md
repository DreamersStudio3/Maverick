# Player Skill HUD

## 목표

- BossHPBar 아래 화면 하단 중앙에 동일 크기 Q/R 스킬 슬롯 2개를 표시한다.
- Q 체인 스킬은 Q1/Q2/Q3 아이콘을 현재 입력 가능한 단계에 맞춰 순서대로 표시한다.
- R 스킬은 단일 아이콘을 표시한다.
- 스킬 메인 쿨다운 동안 어두운 방사형 오버레이가 시계 방향으로 감소한다.
- 슬롯 외형은 동일한 크기의 rounded box와 얇은 외곽선을 사용한다.

## 작업 목록

- [x] 기존 CombatComponent SkillMap/체인/쿨다운 흐름 확인
- [x] UI용 스킬 슬롯 런타임 상태 조회 API 설계
- [x] 방사형 쿨다운 Slate 위젯 구현
- [x] 아이콘 스택을 지원하는 개별 스킬 슬롯 위젯 구현
- [x] Q/R 두 슬롯을 관리하는 PlayerSkillHUD 위젯 구현
- [x] MainHUD 하단 중앙에 PlayerSkillHUD 연결
- [x] 최종 Q1/Q2/Q3/R PNG를 Texture2D 자산으로 가져와 슬롯 기본 아이콘 연결
- [x] CustomVerts용 실제 Slate 텍스처 리소스로 교체해 방사형 쿨다운 렌더 복구
- [x] Q 체인 단계 간 2초 쿨다운도 방사형 오버레이에 연결
- [x] 슬롯 우측 상단의 1/3, 2/3, 3/3 텍스트 제거
- [x] 슬롯 Ability 시전 중에는 사용한 아이콘과 쿨다운 오버레이를 100%로 고정하고 Ability 종료 후 다음 아이콘과 타이머 감소 시작
- [x] Q 슬롯 위 왼쪽에 Q1 아이콘을 사용하는 1/2 크기 방사형 체인 타이머 슬롯 추가
- [x] InterStage 쿨다운 종료 후 Input Duration 타이머가 순차 시작되도록 체인 만료시간 분리
- [x] UHT와 변경된 스킬 UI/Combat/MainHUD 소스 개별 컴파일 확인
- [x] MaverickEditor 전체 UHT/컴파일/링크 검증
- [x] PIE에서 Q 체인 단계 및 메인 쿨다운 방사형 오버레이 표시 검증
- [x] PIE 검증 완료 후 임시 `SkillCooldownTrace` 전환 로그 제거

## 검증 메모

- `MVCombatComponent`, `MVCooldownOverlayWidget`, `MVSkillSlotWidget`, `MVPlayerSkillHUDWidget`, `MVMainHUDWidget` 컴파일 성공
- 최종 Q1/Q2/Q3/R Texture2D 자산 임포트와 기본 SoftObject 경로 연결 확인
- `MaverickEditor Win64 Development` 전체 빌드 성공
- 게임 타깃 전체 빌드는 기존 `MVSheetSpecs.cpp`의 editor-only metadata API 오류에서 중단
