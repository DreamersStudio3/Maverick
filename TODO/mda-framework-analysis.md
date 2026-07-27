# Maverick MDA 프레임워크 분석 초안

## 목표

코드와 현재 설계 자료에 흩어진 Maverick의 Mechanics, Dynamics, Aesthetics를 명시적인 문서로 정리한다.
특히 플레이어 공격 선택지, 적의 피격/행동 중단, Q 연계 스킬의 보상 구조를 전투 경험의 관점에서 분석한다.

## 진행 상태

- [x] 루트 `POLICY.md` 확인
- [x] 참고 블로그의 MDA 정의와 분석 방향 확인
- [x] 플레이어 공격·입력·자원·연계 규칙 조사
- [x] 피격·대미지·그로기·적 AI 규칙 조사
- [x] 이동·회피·소모품·진행·UI 등 전투 외 규칙 조사
- [x] 현재/의도/추정 상태를 구분한 MDA 초안 작성
- [x] 코드 근거와 Blueprint 기본값/DataTable/Montage 교차검증
- [x] 초안 독립 검토 및 후속 설계 질문 정리

## 결과물

- `MaverickDesign/MDAFrameworkAnalysis.md`

## 범위 원칙

- C++과 텍스트 설정에서 확인되는 내용은 `구현 확인`으로 기록한다.
- Blueprint, Chooser, DataTable, Montage처럼 바이너리 에셋 검증이 필요한 내용은 `에셋 확인 필요`로 기록한다.
- 플레이테스트 관찰과 기획 의도는 코드의 현재 동작과 섞지 않고 별도 표기한다.
- 이번 작업은 분석 초안 작성까지이며 전투 수치나 런타임 코드는 변경하지 않는다.

## 완료 메모

- 현재 작업 트리 기준으로 C++, DataTable, Montage Notify, 주요 Ability Blueprint 기본값을 읽기 전용 분석했다.
- 실제 동작으로 확인된 내용과 플레이테스트 관찰, 원래 의도, 설계 가설을 분리했다.
- 런타임 코드와 에셋은 변경하지 않았다.
- 다음 단계는 초안의 `팀 검토가 필요한 질문`에 답해 전투 북극성과 P0 전투 계약을 확정하는 것이다.

## v0.2 전투 구조 업데이트

- [x] 기본공격을 약공격/차지공격으로 축소하는 목표 문법 정의
- [x] 3+1 스킬 슬롯과 슬롯별 역할·대체 스킬 구성 원칙 정의
- [x] 카타나·석궁·대형 낫의 무기별 Dynamics와 Aesthetics 정의
- [x] 단계형 Impact/Armor와 Hit Transaction 계약 정의
- [x] 자원·적중 확정·그로기·피드백 규칙 제안
- [x] 구현 순서와 플레이테스트 검증 기준 갱신
- [x] 문서 독립 검토 및 TODO 완료 처리

### v0.2 완료 메모

- 기본공격 축소와 3+1 스킬 슬롯을 목표안으로 명시하고, 확정 방향·우선안·미결정 항목을 분리했다.
- 단계형 Impact/Armor, 행동 Phase, 연속 경직 저항, 통합 Hit Transaction 계약을 정의했다.
- 카타나·석궁·대형 낫의 역할과 슬롯별 sidegrade 구성, 자원 상한, Q 적중 확정 규칙을 제안했다.
- 독립 검토에서 확인된 시작 구간 연속 경직, 그로기 발동, 패리 양방향 결과, 같은 프레임 trade, 용어 충돌을 보완했다.
- 런타임 코드와 에셋은 변경하지 않았다.

## v0.3 변경 중심 재작성

- [x] 루트 `POLICY.md`와 develop 작업 트리 확인
- [x] `git pull --ff-only`로 원격 v0.2 및 최신 전투 변경 반영
- [x] v0.2 문서와 현재 전투·스킬·입력·HUD 코드 대조
- [x] 강공격 제거와 문맥형 기본공격 유지 계약 반영
- [x] S1 Tempo / S2 Area / S3 Response / S4 Signature 역할 재정의
- [x] S1 Charge와 S4 적중 게이지의 획득·소비·HUD 계약 정리
- [x] 기존 장문 분석을 변경점·구현 간극·마일스톤 중심으로 압축
- [x] 문서 diff와 근거 경로 최종 검토

### v0.3 완료 메모

- 문서를 1,320줄에서 444줄로 줄이고 v0.1/v0.2의 전체 설명 대신 변경 계약과 다음 마일스톤만 남겼다.
- S1은 연계 단계와 분리된 스킬 Charge, S2는 범위, S3는 기본 회피와 구분되는 반응 스킬, S4는 유효 적중 게이지로 정의했다.
- S1과 S4의 첫 프로토타입 수치, 중복 적중 방지 규칙, HUD 상태, 구현 순서와 플레이테스트 지표를 추가했다.
- 런타임 코드와 에셋은 변경하지 않았다.

## v0.3 입력·회피 계약 보강

- [x] 사용자 제안 키보드·마우스/Xbox 바인딩과 참고 UX 사례 확인
- [x] 현재 Enhanced Input, Q/R chord, Tap/Hold, Dodge Chooser 구조 대조
- [x] 한 버튼 약/차지의 Tap/Hold 지연과 스킬 chord 충돌 원칙 반영
- [x] 게임플레이·아이템·무기·상호작용 전체 입력표 반영
- [x] 락온과 무관한 기본 Roll 및 Perfect Dodge Step 계약 반영
- [x] S3 Response와 Perfect Dodge의 역할 경계 정리
- [x] 관련 TODO 드리프트와 문서 diff 최종 검토

### 입력·회피 보강 완료 메모

- 사용자 후보를 모두 기록하고 게임패드는 `RB Basic / LB Tempo / RT Area / LT Defensive / Y Ultimate` 직접 입력을 우선안으로 정리했다.
- Shift chord의 base action 차단, LB/LT chord 선행 발동, D-Pad item layer와 무기 변경 충돌을 구현 전 검증 항목으로 남겼다.
- 방향 회피는 락온 여부와 무관하게 Roll을 기본으로 하고, 실제 Hit이 Perfect Window와 교차했을 때만 짧은 Step으로 끝나는 계약을 추가했다.
- Perfect Dodge는 공용 스태미나 숙련 보상, S3는 별도 제한과 무기별 결과를 가진 대응 스킬로 역할을 분리했다.
- 기존 `TODO/gamepad-input-mapping.md`에 v0.3 우선안과 과거 Q/R 배치의 드리프트를 표시했다.
- 런타임 코드와 에셋은 변경하지 않았다.

### 기본공격 Hold 해제 계약 정정

- [x] 기본공격 결과를 약공격/차지공격 두 종류로만 제한
- [x] `ChargeCommitTime` 전 Release는 Hold 길이와 관계없이 약공격 재생
- [x] `ChargeCommitTime` 도달 시 차지공격 Commit, 이후 Release로 약공격 복귀 금지
- [x] 부분 차지·중간 배율·무행동 취소 계약 제거
- [x] Commit 경계 같은 프레임의 결정론과 플레이테스트 지표 반영

## S1 공통 Charge·연속기 구현안

- [x] 단일 쿨다운을 `MaxCharges = 1`로 포함하는 공통 Charge 모델 정의
- [x] `AvailableCharges`와 `CurrentChainStage`의 책임 분리
- [x] 순차 회복, 간격 사용, 기본공격 적중 회복 단축 규칙 정리
- [x] S1~S4 공통 슬롯 생명주기와 정책별 gate 정의
- [x] Commit 예약·소비·취소 반환 및 HUD 계약 정리
- [x] 현재 Q/R 구현에서의 마이그레이션 순서와 검증 시나리오 작성
- [x] v0.3 MDA 본문에 별도 구현안 링크 연결

### 구현안 작성 완료 메모

- `MaverickDesign/S1TempoChargeImplementation.md`를 노션에 독립적으로 옮길 수 있는 구현 검토안으로 추가했다.
- 무기별 S1 차이는 코드 분기가 아니라 `MaxCharges`, 회복 시간, Stage row, Chain 정책 데이터로 관리한다.
- 런타임 코드와 에셋은 변경하지 않았다.
