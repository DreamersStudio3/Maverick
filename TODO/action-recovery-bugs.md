# Action Recovery Bugs

Action recovery 관련 버그와 회피 입력 흐름을 추적한다.

## 진행 상태

- [x] `POLICY.md` 확인
- [x] LightAttack 체인 stage/ability 갱신 순서 확인
- [x] LightAttack trace stage 수정은 로컬 환경 이슈로 확인되어 코드 변경 제거
- [x] Dodge recovery escape 전환 조건 확인
- [x] Dodge recovery 이슈는 1H step/roll ANS 누락으로 확인되어 코드 수정 범위에서 제외
- [x] Backstep fallback row name이 실제 Dodge DataTable row를 가리키도록 수정
- [x] Dodge 입력 제출 시 이동 입력이 없으면 Backstep row와 기본 StartSection을 사용하도록 수정
- [x] InputManager의 Dodge 태그 분기를 제거하고 Dodge 입력 문맥 생성을 Dodge 오브젝트로 정리
- [x] 빌드 검증

## 원인

- LightAttack trace stage mismatch 대응으로 추가했던 Combat/Ability 코드 변경은 다른 로컬 검증에서 불필요한 것으로 확인되어 제거했다.
- Dodge 연속 escape는 코드 조건이 아니라 1H step/roll 애셋의 ANS 설정 누락으로 확인했다.
- Backstep fallback은 기존 `Dodge_P1_01` 형태를 만들지만 실제 row는 `Dodge_P1_Backstep_1H_01`/`BH_01` 형태라 chooser fallback 경로에서 시작할 수 없다.
- Dodge 입력 시점에 이동 입력이 없어도 기존 `PendingMovementInputVector` 재조회나 chooser의 기존 `StartSection`이 남으면 Backstep 대신 이동 회피 문맥으로 처리될 수 있다.

## 검증

- `git diff --check` 통과
- `MaverickEditor Win64 Development -NoHotReloadFromIDE` 빌드 성공
