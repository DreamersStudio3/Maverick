# HitReaction Launch/Enemy 재생 회귀 확인

## 증상

- [ ] WideAttack KnockDown에서 플레이어가 적을 바라보는 상황인데 `Front` 대신 `Back` 방향 몽타주가 선택된다.
- [ ] HitReaction Launch 재생 중 RecoveryEscapeWindow가 열리기 전 입력으로 Launch 액션이 끊길 수 있다.
- [ ] `DT_HR_E1`/`CHT_HR_E1` 및 적 애니메이션 에셋 수정 후 적 피격 애니메이션이 재생되지 않고 다음 State로 넘어간다.
- [ ] Enemy Flinch에서 몽타주 방향이 피격 지점 반대 방향과 맞지 않고, `LaunchDistance=800cm`, `LaunchDuration=1s` 대비 실제 이동량이 너무 짧다.

## 확인 계획

- [x] `HitDirectionTrace`로 request 방향, resolved 방향, 피격자 forward 기준 dot, 선택 row를 한 흐름에서 확인한다.
- [x] RecoveryEscapeWindow 밖 HitReaction 본 액션이 입력/전환으로 끊기는 경로를 확인하고 차단한다.
- [x] Enemy StateTree HitReaction task의 시작 실패/즉시 종료 조건을 로그로 확인한다.
- [x] Enemy Flinch 영상에서 실제 밀림이 짧게 감쇠되는 것을 확인하고, Flinch는 `HitLocation` 반대 방향 우선으로 방향을 계산하게 한다.
- [x] `LaunchDuration` 동안 가로 속도를 유지하고 만료 시 XY 속도를 정리하는 제어 타이머를 추가한다.
- [x] `FMVHitResolveRequest`에 `ImpactNormal`을 추가하고, HitResolver가 `HitLocation`/`ImpactNormal`으로 최종 `HitDirection`을 계산하게 한다.
- [x] HitReactionComponent는 Resolver가 확정한 `ResolvedHitData.HitDirection`만 사용하게 정리한다.
- [x] `FMVHitResolveRequest`/`FMVHitResolveContext`의 `HitDirection` 입력 노드를 제거하고, Ability 쪽은 Impact 값만 넘기는 구조로 정리한다.
- [x] Ability별 Launch 설정과 HitReaction Launch 계산부에 Distance/Duration/VerticalSpeed 적용 흐름을 주석으로 남긴다.
- [x] `bUseLaunch` HitReaction은 몽타주 재생 직전에 Owner yaw를 피격 연출 기준 방향으로 맞춘다.
- [x] `bUseLaunch` HitReaction의 Owner yaw 정렬 기준을 `AttackerForward`로 바꿔 victim forward와 attacker forward가 겹치게 한다.
- [x] HitReaction yaw 정렬 중 바꾼 MovementComponent 회전 정책과 락온 회전 억제를 HitReaction 종료/실패 시 복구한다.
- [x] 기존 LandDetector Notify를 KnockDown에서도 쓰게 하고, KnockDown은 MovementMode 대신 LaunchDuration 완료 후 Land 섹션으로 점프하게 한다.
- [x] `Build.bat MaverickEditor Win64 Development -Project=... -NoHotReload` 빌드 성공으로 컴파일을 확인한다.
- [x] 정면 공격 중 Enemy가 옆으로 밀리는 케이스는 `ImpactPoint`/`ImpactNormal` 방향 보정을 제거하고 공격자-피격자 중심 방향만 쓰는 방식으로 정리했다.
- [x] Launch 정책을 새로 단순화했다. `bUseLaunch` row만 Launch를 쓰고, `HitDirection=VictimLocation-AttackerLocation`으로 고정하며, yaw snap/input lock/vertical cutoff만 남겼다.
- [ ] 공격/몽타주 시작 중 Player가 회전하며 빠져나가는 케이스는 Turn In Place의 `DoTurnInPlace` 잔류 여부와 함께 분리 확인한다. AnimBP에서 쓸 `bActionRunning` 값은 C++에 노출했다.
