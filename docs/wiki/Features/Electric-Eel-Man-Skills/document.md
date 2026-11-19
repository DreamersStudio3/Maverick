---
제목: "전기뱀장어맨 W E R"
부제목: "도약 타격·돌진 포획·누적 방출"
최근수정일: 2026-11-19
최근수정자: "Codex"
관련문서:
  - "[[Features/Electric-Eel-Man-Q/document|전기뱀장어맨 Q]]"
  - "[[Features/Combat/Combat-System/document|Maverick 전투 시스템]]"
---

# 전기뱀장어맨 W E R

## 구성

`Source/Maverick/Combat/Boss/ElectricEelMan/`

| 클래스 | 책임 |
|---|---|
| `MVElectricEelManWAbility` | 목표 방향 도약·착지의 직접 타격과 충격파 |
| `MVElectricEelManEAbility` | 돌진·포획·전기 피해·입력 잠금 해제 |
| `MVElectricEelManRAbility` | 사용 조건·3초 충전·1초 간격 3회 방출 |
| `MVElectricEelManAbility` | W/E/R 타이머·이동 보관/복구·플레이어 판정·공용 피해 전달 |
| `MVElectricEelManSkillComponent` | 보스별 Q/W/E 발동 횟수, R 발동·사망 시 초기화 |

## 발동 횟수

- Q/W/E의 첫 Ability 활성화와 비용 지불 성공 시 1회 누적, 9회까지 보관
- 공격의 추가 Notify 구간은 같은 발동으로 취급, 적중 실패·발동 후 취소도 이미 시작한 스킬로 집계
- 대상 부재·E 사거리 초과·비용 부족에 의한 발동 실패는 집계 제외
- 최초 발동 시 보스에 컴포넌트 자동 등록, Ability 인스턴스가 교체돼도 같은 보스의 횟수 유지
- R은 `CanActivateAbility()`를 공용 전투 컴포넌트가 몽타주 시작 전에 확인
- R Notify 시작에서도 재검사, 비용 지불 성공 시 횟수 0으로 초기화, 충전 취소 시 소비 유지
- 보스 사망 시 초기화, AI 조건에서는 컴포넌트의 `CanUseR()` 조회 가능
- 공격 시험 UI의 실행은 Q/W/E 횟수를 바꾸지 않으며, R은 횟수 확인·소비 없이 시작

## W

- 기본 0.5초·높이 100cm·최대 300cm의 짧은 도약, 시작 시 대상 방향과 종점 고정
- 보스 캡슐 이동은 Sweep 사용, 이동 중 충돌하거나 도약 시간이 끝나면 현재 위치에서 타격
- 판정 중심은 보스 기준 `ImpactOffset = (120, 0, -40)`
- 주먹 반경 120cm, 공격 행 피해 배율의 1배와 `Stagger` 요청
- 충격파 반경 450cm, 공격 행 피해 배율의 0.5배와 `Flinch` 요청
- 주먹 범위를 먼저 처리하고 적중 집합 공유, 직접 피격자의 충격파 중복 피해 방지
- 100%·50%는 방어력 등 공용 보정 전 배율, 실제 경직 모션은 플레이어 피격 설정에서 결정

## E

- 기본 최대 450cm를 0.35초에 돌진, 포획 거리는 액터 원점 사이 150cm
- 시작 시 거리 600cm 초과·무적·사망·기존 입력 잠금 대상은 발동 거절
- 대상 방향 고정 후 이동 Sweep, 근접 시 월드 장애물 시선 검사로 벽 너머 포획 차단
- 포획 시 대상 액션 취소, 입력 차단 카운트 획득·이동 모드 보관 후 정지
- 포획 성공 시 `UE_LOG(LogTemp, Log, TEXT("시퀀스 재생"))` 한 번 출력
- 기본 포획 0.5초 후 공격 행 피해 배율 1배의 전기 피해, 1.5초 후 해제
- 종료·Notify 중단·소유자 사망·대상 무효·소멸에서 타이머와 자신의 입력 잠금 정리
- 시퀀스 재생·촉수 변형·전기 시각 효과는 후속 연출 범위

## R

- 발동 이후 3·4·5초에 보스 중심 반경 900cm 피해, 각 방출의 피해 배율은 공격 행의 1배
- 각 방출은 새 적중 집합 사용, 동일 플레이어 최대 3회 피격
- 중간에 범위 진입·이탈한 대상은 각 방출 시점 위치로 판정
- 느린 프레임에서는 경과한 방출 시점을 순서대로 처리, 3회 후 자동 종료
- 충전·방출 중 Notify 종료나 공격 취소 시 남은 방출 제거

## 몽타주 연결

1. `FMVSkillDataTableColumn` 기반 공격 행의 `AbilityReference`에 W/E/R 클래스 또는 자식 Blueprint 지정
2. 각 행의 `Montage`에 제작한 공격 몽타주 연결
3. 몽타주에 `MV Activate Ability` Notify State 배치, `AbilityClass` 지정 시 해당 클래스와 일치
4. W는 도약 시작부터 착지까지, E는 돌진부터 포획 종료까지, R은 충전 시작부터 5초 이후까지 활성 구간 확보
5. 이동은 Ability에서 담당하므로 몽타주 루트 이동과의 합성은 후속 연출 검증 대상
6. StateTree의 공격 실행이 공용 `CombatComponent`의 행 실행 경로를 사용하도록 연결

## Eelman 적용 에셋

루트 경로는 `/Game/Characters/NPC/Boss/Eelman`. `BP_Eelman`은 `SK_Eelman`과 `ABP_Eelman`을 사용하며, `ABP_Eelman`의 `DefaultSlot`으로 공격 몽타주를 재생

| 호출 이벤트 | 공격 행 | 몽타주 | Notify 활성 구간 |
|---|---|---|---:|
| 시험 UI 기본 공격 | `BasicAttack1` | `AM_Eelman_BasicAttack` | 오른손 0.43초, 왼손 1.10초, 양손 1.88초부터 각각 0.15초 |
| `UseSkillQ` | `SkillQ` | `AM_Eelman_SkillQ` | 0.70–0.92초 |
| `UseSkillW` | `SkillW` | `AM_Eelman_SkillW` | 0.35–1.10초 |
| `UseSkillE` | `SkillE` | `AM_Eelman_SkillE` | 0.20–2.25초 |
| `UseSkillR` | `SkillR` | `AM_Eelman_SkillR` | 0.15–5.55초 |

- 모든 공격은 `DT_EelmanCombatAttack`의 행으로 공용 전투 컴포넌트를 호출
- 기본 공격 한 번에 2.4초 3타 실행. 각 Notify 구간은 `MVBoxDamageAbility`의 전방 박스로 대상당 한 번 피해 적용
- 배치된 보스와 Blueprint의 메시 방향을 액터 전방에 정렬. 맨손 공격력 시험값 20 기준으로 Windows PIE에서 체력 100 → 80.5 → 61 → 41.5 확인
- `Animations/AS_Eelman_Skill*_Temp`는 Eelman 참조 자세로 만든 임시 동작. 최종 동작 교체 시 몽타주 슬롯과 Notify 구간 유지
- 대기 자세는 스켈레톤 기본 자세 사용. 자동 공격 선택 StateTree와 E 포획 시퀀스·VFX는 후속 연결
- 2026-11-16 Windows PIE에서 테스트 인스턴스의 AI를 끄고 네 이벤트 직접 호출. Q/W/E 몽타주와 누적, E 포획 로그, R의 0회 거절 및 9회 재생·초기화 확인

## Manny 대기 동작 리타겟

- 리타겟 전용 에셋은 `Animations/RetargetAnimations/Normalized`. `RTG_MannyToEelman_Normalized`의 대상은 별도 Skeleton을 사용하는 `SK_Eelman_Retarget`
- 기존 메시를 FBX로 내보낸 후 Blender에서 Armature 스케일 적용. Unreal 가져오기에서 cm 단위를 보정해 원래 크기 유지, 기준 `root` 스케일은 100에서 1로 정규화
- 포즈 초기화와 대상 자동 정렬 후 대기 동작용 `Run IK Rig`·`Root Motion` 비활성화. 이동 동작은 발 접촉과 루트 이동을 따로 검증해야 함
- Windows UE 5.8.4 에디터에서 `AS_Eelman_1H_Idle`의 저장 전 원시 자세 5개 검증. 머리·골반·발 높이와 루트 스케일 검사 통과, 기존 `BP_Eelman`에는 미적용

## Eelman 로코모션

- `Animations/Locomotion/BS_Eelman_Locomotion`: 방향 -180~180°, 속도 0~450cm/s. 대기와 4방향 걷기 180cm/s·달리기 450cm/s, 후진 경계 포함 15개 표본
- `ABP_Eelman_Locomotion`: 공용 `MVAnimInstanceBase`의 GroundSpeed 및 Velocity·ActorRotation으로 계산한 방향 → Blend Space → DefaultSlot → 최종 자세
- `BP_Eelman`과 배치 인스턴스의 Mesh·AnimClass에 정규화 메시와 전용 AnimBP 적용. 메시 Yaw -90°로 원본 Manny 동작 방향과 액터 전방 일치
- `DT_EelmanCombatAttack`은 같은 폴더의 정규화 공격 몽타주 참조. 이전 시퀀스의 root 스케일 100을 1로, 하위 본 로컬 이동은 100배로 변환해 모델 공간 자세 보존. 기존 공격 Notify 복사
- Windows UE 5.8.4 PIE에서 속도 입력별 자세 평가·메시 스케일과 기본 공격 몽타주·3개 Notify 확인. 이동 속도는 시험 중 직접 주입, 실제 AI 추격·발 접촉·전투 전체 회귀는 별도 검증 범위
- 이동 시작·정지 및 제자리 회전 전용 동작은 후속 범위. 원본 공격과 적용 전 BP·테이블 백업은 `Saved/EelmanLocomotionBackup` 보관

## 검증

2026-11-11 · Codex · Windows · Unreal Engine 5.8.4 Development Editor

- Editor 빌드 성공, W/E/R·공용 부모·집계 컴포넌트·몽타주 전 사용 조건의 컴파일·링크 확인
- `Maverick.Combat.ElectricEelMan.Skills` 성공, 공용 Notify 경로에서 W 우선순위·배율, E 포획 로그·지연 피해·취소 해제, R 조건·소비·3/4/5초 방출·취소 검증
- 기존 `BoxDamageAbility`·`DuelistResonance` 성공, Q·일반 박스와 공명 폭주 회귀 확인
- 초기 검사 캐릭터의 BeginPlay 누락으로 무기 초기화 전 피해값 사용, 명시적 초기화와 공격력 단언 추가 후 최종 3개 검사 성공
- 격리 게임 월드·NullRHI 실행, 테스트 캐릭터의 무기/피격/사망 행 미지정 경고 포함
- 실제 보스 애니메이션·플레이어 경직 연출·시퀀스·VFX·멀티플레이·패키징은 이번 검사 범위 밖
- 빌드의 기존 PropertyBindingUtils 의존성·XGE 라이선스 경고 유지

결과 보관은 `Saved/ElectricEelManSkills/AutomationFinal/index.json`, `automation-final.log`, `build-final.log`, Git 제외
