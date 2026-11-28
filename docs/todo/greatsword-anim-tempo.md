# 아트록스형 대검 평타·스킬 템포 대비 조정

- 목표: 평타는 빠르고 가볍게, 스킬은 선딜-임팩트 대비가 뚜렷하게 PlayRate·몽타주·Notify 타이밍 조정
- 대상: `DT_Weapons_Player.GreatSword_TestGreatSword` → `PC_GreatSword` → `Player_GreatSword`
- 상태: A·B 에셋 적용·저장 완료, PIE 체감 확인 대기, 미커밋
- 메쉬: `SM_GreatHammer`(Translation Z -20) → `/Game/MassiveGreatSwordAnimset/demo/mesh/weapon`(Translation 0), 그립 정렬 미확인
- A 평타: LightAttack·LightAttack2·Dodge_LightAttack·Sprint_LightAttack PlayRate 1.15, LA1/LA2/DA/SA BlendIn 0.1, 탈출창 시작 0.85/1.15/1.00/0.90(끝 유지)
- B 스킬: Q1~Q3·R 세그먼트 3분할(판정 전 0.9x, 판정+0.1s 1.4x, 이후 1.0x), 전 Notify 새 타임라인 재배치, Q2 탈출창 시작 2.92 → 2.30
- 결과(실제 초): 평타 판정 시작 0.19~0.60·탈출창 0.74~1.00, Q/R 판정 시작 0.91~1.95·판정 폭 0.20~0.30
- 결정: 범위는 애니메이션 속도감만, 타격 피드백(이펙트·셰이크·사운드·피격 반응) 제외, 차지·W·E 미변경
- 결정: Python에서 `AnimSegment.StartPos`·몽타주 길이 읽기 전용 → 세그먼트 저장 후 재로드 PostLoad 재계산, Notify는 기존 인스턴스 재부착
- 위키 검토: 변경 불필요 예상: 에셋 수치 튜닝만, 코드·구조 계약 변경 없음
- 검증: 커맨드렛 재덤프로 PlayRate·BlendIn·세그먼트·Notify 시각·몽타주 길이 확인, PIE 미실시
