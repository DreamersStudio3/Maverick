# 원거리 공격 병합 복구와 현재 전투 구조 연결

- 목표: 복구한 원거리 발사를 현재 스킬 슬롯·연계·단일 선택 패시브·사망 처리에 연결
- 상태: 1~3단계 완료, 4단계 핵심 코드와 사용자 PIE 확인, 5단계 자동화 시험 성공·임시 로그 제거 완료
- 결정: 기존 발사체와 공격별 실행 기록 재사용. 무기·패시브 교체 및 발사자 사망 뒤에도 이미 발사한 탄의 기본 피해 유지. 생존 회차가 바뀌면 후속 보상 차단
- 시험 자산: `BP_TestProjectileAbility` → `BP_TestStraightProjectile`; `Test_OneHand` Q 첫 행 → `Yone_Attack.SkillQ` → 시험 Ability. 현재 몽타주는 옛 Blueprint `ANS_ActiveAbility` Notify 사용. 사용자 Editor에서 `DA_Passive_PredatorCycle.RefundChance=0.1`, 발사체 수명 5초·초기/최대 속도 1200 복원 확인
- 위키 검토: 갱신: `docs/wiki/Features/Combat/Ranged-Projectile/document.md` 초안에 실행 기록·지연 적중·환원·사망 계약과 검증 범위 기록
- 검증: 사용자 Windows Editor에서 `MergedDamage` 1건 성공, `ExecutionLifetime` 재실행 1건 성공·실패 0건. 사용자 PIE에서 Q 발사·적중, 동작 종료 뒤 `RefundAccepted`, 사망 뒤 `HitRewardsSkipped`와 기록 제거 확인. 새 시험은 한 공격의 두 탄, 늦은 적중의 기본 피해·쿨타임 환원 1회, 중복/오래된 사용 회차 거절, 사망 뒤 적 HP 감소·보상 차단을 확인. 로그 제거 후 최신 빌드·`ExecutionLifetime`·Q 발사/적중도 사용자 Windows Editor에서 성공. Agent 빌드·시험·PIE는 사용자 요청에 따라 미실행
- 로그 정리: 임시 `ProjectileCombatTrace` 58개 제거, 정적 공백 검사와 사용자 최신 빌드·핵심 시험 통과

## 남은 확인

- [ ] 실제 패시브의 MP 잔량·화면 쿨타임, 한 공격의 다발 적중 때 환원 횟수
- [ ] 투사체 비행 중 무기·패시브 교체와 부활 뒤 적중의 기본 피해·후속 보상
- [ ] 발사 실패·직접 행 실행·복수 Notify·선택 패시브의 실제 피해 보정

Editor 자산 작업·빌드·실행은 사용자 담당
