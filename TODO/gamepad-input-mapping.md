# 게임패드 입력 매핑

## 목표

- ProjectBA에서 사용한 게임패드 조작 체계를 Maverick의 현재 Enhanced Input 구조에 맞춰 연결한다.
- 이동/카메라/전투뿐 아니라 상호작용과 CommonUI 입력까지 한 흐름으로 검증한다.
- 메뉴가 활성화되면 키보드 방향키와 게임패드 D-Pad/좌측 스틱으로 포커스를 이동하고, 좌측 입력으로 이전 버튼으로 이동할 수 있게 한다.
- 게임플레이 입력과 메뉴 입력이 동시에 실행되지 않도록 입력 모드와 포커스 소유권을 확인한다.

## 시작 상태

- [x] 루트 `POLICY.md` 확인
- [x] `develop` 워킹트리가 깨끗한지 확인
- [x] 기준 커밋 `ab1f758` 확인
- [x] `codex/gamepad-input-mapping` 브랜치 생성 및 전환
- [x] Maverick의 Input/CommonUI/상호작용 구조 1차 조사
- [x] ProjectBA의 게임패드 런타임 매핑과 UI 내비게이션 구현 조사

## 확인된 현재 구조

- 게임플레이 입력 에셋은 `Content/Input/Actions/IA_*`와 단일 `IMC_DefaultLocomotion`을 사용한다.
- `AMVCharacterBase::SetupPlayerInputComponent`에는 네이티브 바인딩이 없고, 현재 액션 이벤트 전달은 BP/IMC 연결에 의존한다.
- 상호작용 실행용 `IA_Interact`와 `AMVPlayerCharacter::TryInteract()`가 있다.
- 상호작용 후보 전환용 `SelectPreviousInteractable()` / `SelectNextInteractable()` API는 있지만 입력 액션 연결은 아직 없다.
- CommonUI는 `CommonGameViewportClient`, Enhanced Input 지원, `UMVCommonUIInputData`의 Generic Accept/Back을 이미 사용한다.
- `UMVActivatableWidgetBase`는 메뉴 입력 모드와 초기 포커스 대상을 지원하고, 스킬 트리는 명시적 4방향 내비게이션 API를 갖고 있다.
- ProjectBA는 Xbox 표준 키와 DualSense RawInput 키를 함께 런타임 보강했다. Maverick에는 아직 DualSense RawInput 설정이 없다.

## 에디터 적용 기준 게임패드 배치

| 기능 | Xbox 표기 | PlayStation 표기 |
| --- | --- | --- |
| 이동/아날로그 보행 | Left Stick | Left Stick |
| 카메라 | Right Stick | Right Stick |
| 앉기 토글 | L3 | L3 |
| 약공격 | RB | R1 |
| 강/차지 공격 | RT | R2 |
| 질주/회피 | B | Circle |
| 상호작용/결정 | A | Cross |
| 회복약(권장) | D-Pad Up | D-Pad Up |
| 락온 | R3 | R3 |
| Q 스킬 | Y | Triangle |
| R 스킬 | LB + Y | L1 + Triangle |
| 상호작용 후보 이전/다음 | D-Pad Left/Right | D-Pad Left/Right |

게임패드 Walk는 별도 버튼을 사용하지 않고 Left Stick 기울기 크기로 결정한다. `IA_Walk`는 기존 키보드 Z 입력만 유지한다. Crouch는 `L3`, 키보드는 기존 C를 사용한다. Q가 Y를 사용하므로 기존 회복약 Y 배치는 충돌하며, 회복약은 D-Pad Up을 권장한다. 메뉴 열기 버튼은 현재 메뉴 WBP와 열기 IA가 저장소에 없으므로 이번 매핑에서 제외한다.

## 구현 판단

- 게임플레이 IA 이벤트는 `BP_ThirdPersonCharacter`가 소유하고 있으므로 C++ 런타임 매핑을 추가하지 않는다.
- `IMC_DefaultLocomotion`에 게임패드 키를 추가하면 기존 IA 이벤트 그래프가 그대로 실행된다.
- 통합 `IA_SprintDodge`는 사용하지 않고 기존 `IA_Dodge`와 새 `IA_Sprint`로 분리한다.
- 게임패드 B는 두 액션에 함께 매핑한다. Dodge는 Started에서 즉시 실행되고, 같은 입력을 계속 누르면 Sprint가 Hold 임계점 이후 활성화된다.
- 키보드 Dodge와 Sprint는 서로 다른 기본 키를 줄 수 있으며, 별도 Player Mappable 이름을 사용해 이후 개별 리매핑할 수 있게 한다.
- R 스킬의 LB+Y는 별도 `IA_SkillModifier`와 `Chorded Action` 트리거로 구성한다.
- 상호작용 후보 전환은 이미 공개된 `SelectPreviousInteractable` / `SelectNextInteractable`을 새 IA 이벤트에서 호출한다.
- CommonUI Accept/Back과 방향 내비게이션은 현재 C++ 및 프로젝트 설정으로 이미 활성화되어 있다.
- 메뉴 WBP가 아직 저장소에 없으므로 버튼 포커스와 명시적 Navigation은 해당 WBP를 만들거나 편집할 때 설정한다.
- CommonUI의 Menu 입력 모드는 UI 입력 컴포넌트보다 우선순위가 낮은 플레이어 입력을 차단하므로 A/B와 D-Pad를 게임플레이 IA와 함께 사용해도 메뉴 안에서 상호작용·회피·후보 전환이 동시에 실행되지 않는다.

## 구현 계획

### 1. 입력 계약과 에셋 상태 확정

- [x] `IMC_DefaultLocomotion`의 현재 키/Modifier/Trigger를 덤프해 기존 키보드·마우스 매핑을 보존할 기준을 만든다.
- [x] `BP_ThirdPersonCharacter`가 현재 사용하는 IA 에셋을 확인한다.
- [x] Xbox/XInput 경로와 DualSense 직접 연결(RawInput) 경로를 구분한다.
- [x] `Walk`, `Crouch`와 Q/R 스킬의 적용 키를 정리한다.
- [x] 메뉴 열기는 해당 IA/WBP가 없어 이번 범위에서 제외한다.

### 2. 게임플레이 입력 매핑

- [x] C++ 런타임 보강 없이 기존 IMC/BP 구조를 유지하기로 결정한다.
- [ ] 아래 에디터 절차에 따라 이동/카메라/액션 게임패드 매핑을 추가한다.
- [ ] 필요할 때만 RawInput 플러그인과 DualSense 장치 설정을 적용하고 XInput 중복 입력 여부를 검증한다.

### 3. 상호작용 후보 전환

- [ ] 이전/다음 후보 전환용 입력 액션을 추가하거나 기존 축 액션 재사용 가능성을 검토한다.
- [ ] 키보드 Left/Right와 게임패드 D-Pad Left/Right를 후보 전환 API에 연결한다.
- [ ] 후보가 없거나 하나뿐일 때 입력을 소비하지 않는지 확인한다.
- [ ] 대화/팝업/메뉴가 입력을 소유할 때 후보 전환이 동시에 실행되지 않게 한다.

### 4. CommonUI와 메뉴 내비게이션

- [x] CommonUI Generic Accept/Back 설정과 CommonGameViewportClient 적용을 확인한다.
- [x] 엔진 기본 내비게이션에 Left/Right/Up/Down Arrow, D-Pad, 좌측 스틱이 포함되는지 확인한다.
- [x] `UMVWindowBase`의 Menu 입력 모드가 플레이어 입력을 차단하는지 확인한다.
- [ ] 메뉴 활성화 시 첫 번째 유효 버튼에 포커스가 들어오도록 WBP에서 Desired Focus Target을 설정한다.
- [ ] 일반 메뉴 버튼의 Left/Right/Up/Down Navigation 관계를 WBP에서 설정한다.
- [ ] 스킬 트리처럼 비정형 배치인 화면은 `SetExplicitSkillNavigation` 경로를 사용해 네 방향 링크를 검증한다.
- [ ] 게임패드와 마우스 전환 시 포커스/호버 표시가 자연스럽게 갱신되는지 확인한다.

### 5. 검증 및 사용자 수동 작업 정리

- [ ] C++ 변경이 있으면 `MaverickEditor Win64 Development` 빌드로 확인한다.
- [ ] PIE에서 키보드·마우스 회귀와 Xbox 계열 게임패드 입력을 확인한다.
- [ ] 가능하면 DualSense 직접 연결과 XInput 에뮬레이션 경로를 각각 확인한다.
- [ ] 게임플레이, 다중 상호작용 후보, 일반 메뉴, 스킬 트리, 팝업 Accept/Back을 체크리스트로 검증한다.
- [ ] 에디터에서만 가능한 IMC/BP/WBP 수정은 정확한 에셋 경로와 노드/키 설정 순서로 사용자 안내를 작성한다.

## 수동 수정이 필요할 가능성이 높은 지점

- `Content/Input/IMC_DefaultLocomotion.uasset`: 기존 IA에 게임패드 키 추가 및 축 Modifier 확인
- `BP_ThirdPersonCharacter`: 새 상호작용 후보 전환 IA를 C++ API에 연결
- 메뉴/스킬 트리 WBP: `InitialFocusTarget`, `Is Focusable`, 명시적 Left/Right Navigation 설정
- DualSense를 Steam Input/DS4Windows 없이 직접 받을 경우: RawInput 플러그인과 장치별 키 설정

현재 프로젝트의 에디터 중심 입력 구조를 유지하며, 위 에셋은 Unreal Editor에서 수정한다. C++ 런타임 매핑이나 에셋 자동 변경은 추가하지 않는다.

## Unreal Editor 적용 절차

### `IMC_DefaultLocomotion`

Content Browser에서 `/Game/Input/IMC_DefaultLocomotion`을 열고 기존 키보드/마우스 행은 유지한 채 다음 행을 추가한다.

| Input Action | 추가 Key | Modifier / Trigger |
| --- | --- | --- |
| `IA_Move` | Gamepad Left 2D-Axis | 없음 |
| `IA_Look` | Gamepad Right 2D-Axis | 기존 Mouse2D 행을 복제해 현재 `Negate` 유지 |
| `IA_Walk` | 게임패드 매핑 없음 | 기존 키보드 Z 전용 |
| `IA_Crouch` | Gamepad Left Thumbstick Button | 기존 C와 함께 사용 |
| `IA_Dodge` | Gamepad Face Button Right | 없음, Started 즉시 처리 |
| `IA_Sprint` | Gamepad Face Button Right | IA 에셋의 Hold 사용 |
| `IA_Interact` | Gamepad Face Button Bottom | 없음 |
| `IA_LightAttack` | Gamepad Right Shoulder | 없음 |
| `IA_HeavyChargeAttack` | Gamepad Right Trigger | 없음 |
| `IA_SkillQ` | Gamepad Face Button Top | 없음 |
| `IA_SkillModifier` | Gamepad Left Shoulder | Trigger 없음 |
| `IA_SkillR` | Gamepad Face Button Top | 매핑 행에 Chorded Action=`IA_SkillModifier` |
| `IA_LockOnTarget` | Gamepad Right Thumbstick Button | 없음 |
| `IA_UseConsumable` | Gamepad D-Pad Up 권장 | Q 스킬 Y와 충돌 방지 |

Xbox 기준으로 Bottom/Right/Left/Top은 각각 A/B/X/Y이며, PlayStation 표기로는 Cross/Circle/Square/Triangle이다.

`IA_Look`은 현재 Mouse2D 행에 `Negate`가 있으므로 우선 행 복제로 동일하게 적용한다. PIE에서 스틱 상하 또는 좌우가 반대로 느껴지면 새 Gamepad 행의 `Negate X/Y`만 조정하고 Mouse2D 행은 변경하지 않는다.

### Left Stick 기울기 기반 Walk/Run

`IA_Move`의 Gamepad Left 2D-Axis 값을 이동과 보행 판정에 함께 사용한다. 같은 스틱을 Boolean 타입 `IA_Walk`에도 매핑하면 강하게 기울였을 때 두 액션이 동시에 활성화되므로 그렇게 구성하지 않는다.

`BP_ThirdPersonCharacter`에 `bKeyboardWalkToggled`, `bGamepadAnalogWalk` Boolean 변수를 추가하고 기존 `IA_Move` 처리에 다음 흐름을 추가한다. 두 값을 분리해야 입력 장치를 바꿨을 때 게임패드의 마지막 보행 상태가 키보드 토글 상태를 덮어쓰지 않는다.

1. `Action Value`를 `Axis2D`로 가져와 `Vector Length`로 `StickMagnitude`를 계산한다.
2. Common Input Subsystem의 `Current Input Type == Gamepad`인지 확인한다.
3. Gamepad이면 작은 흔들림을 무시하기 위해 `StickMagnitude <= 0.2`에서 `bGamepadAnalogWalk=false`로 정리한다.
4. 현재 `bGamepadAnalogWalk=false`이면 `0.2 < StickMagnitude < 0.5`에서 Walking으로 진입한다.
5. 현재 `bGamepadAnalogWalk=true`이면 `StickMagnitude < 0.65` 동안 Walking을 유지하고, `0.65` 이상에서 Running으로 전환한다.
6. Gamepad에서는 `CharacterInputState.WantsToWalk=bGamepadAnalogWalk`을 적용한다.
7. MouseAndKeyboard에서는 `CharacterInputState.WantsToWalk=bKeyboardWalkToggled`을 적용한다.
8. 기존 `IA_Walk(Z)`의 Started에서는 `bKeyboardWalkToggled`을 반전시키고, 현재 입력 타입이 MouseAndKeyboard일 때만 `WantsToWalk`에 적용한다.
9. `IA_Move`의 Completed/Canceled에서는 Gamepad일 때 `bGamepadAnalogWalk=false`, `WantsToWalk=false`로 초기화한다.

`0.5 / 0.65` 두 임계값은 경계에서 Walking/Running이 빠르게 튀는 것을 막는 히스테리시스다. 실제 패드 감도에 따라 Walking 진입값은 `0.45~0.55`, Running 전환값은 `0.6~0.7` 범위에서 조정한다.

#### Blueprint 노드 구성

먼저 `BP_ThirdPersonCharacter`에 `SetWantsToWalk` 함수를 만든다.

1. Boolean 입력 `NewWantsToWalk`을 추가한다.
2. `CharacterInputState` 변수를 Get으로 그래프에 놓는다.
3. `Set Members in CharacterInputState` 노드의 Struct Ref에 연결한다.
4. 노드 Details에서 `WantsToWalk` 핀만 노출한다.
5. `NewWantsToWalk`을 해당 핀에 연결한다.

`Make CharacterInputState`를 새로 만들어 대입하면 `WantsToStrafe`, `WantsToAim`, `WantsToSprint`가 기본값으로 덮일 수 있으므로 사용하지 않는다.

`IA_Move.Triggered`에는 기존 이동 처리와 보행 판정을 `Sequence`로 나란히 연결한다.

- `Then 0`: 기존 `Add Movement Input` 로직을 그대로 실행한다.
- `Then 1`: `Action Value -> Get 2D Axis Value -> Vector Length -> Clamp(0, 1)`로 `StickMagnitude`를 만든다.
- `Get Controller -> Cast To PlayerController -> Get Local Player Subsystem(CommonInputSubsystem) -> Get Current Input Type`으로 입력 타입을 얻는다.
- `Switch on ECommonInputType` 또는 `Equal(Gamepad) -> Branch`로 분기한다.

Gamepad 분기에서는 다시 `bGamepadAnalogWalk`로 Branch한다.

- False: `(StickMagnitude > 0.2) AND (StickMagnitude < 0.5)` 결과를 `bGamepadAnalogWalk`에 저장한다.
- True: `(StickMagnitude > 0.2) AND (StickMagnitude < 0.65)` 결과를 `bGamepadAnalogWalk`에 저장한다.
- 저장 후 `SetWantsToWalk(bGamepadAnalogWalk)`을 호출한다.

MouseAndKeyboard 분기에서는 `SetWantsToWalk(bKeyboardWalkToggled)`만 호출한다.

`IA_Move.Completed`와 `IA_Move.Canceled`는 둘 다 `ResetGamepadWalk` Custom Event로 연결한다.

1. 현재 Common Input Type이 Gamepad인지 확인한다.
2. Gamepad이면 `bGamepadAnalogWalk=false`로 설정한다.
3. `SetWantsToWalk(false)`를 호출한다.

`IA_Walk.Started`는 다음 순서로 구성한다.

1. `NOT Boolean(bKeyboardWalkToggled)` 결과를 다시 `bKeyboardWalkToggled`에 저장한다.
2. 현재 Common Input Type이 MouseAndKeyboard인지 확인한다.
3. 맞으면 `SetWantsToWalk(bKeyboardWalkToggled)`를 호출한다.

`IA_Move`의 Axis2D는 Vector Length 계산 전에 정규화하지 않는다. 기존 이동 로직에도 원래 X/Y 값을 전달해 스틱 기울기가 보존되게 한다.

### Sprint / Dodge 분리

1. 기존 Digital 타입 `IA_Dodge`를 사용한다. Trigger는 추가하지 않는다.
2. Digital 타입 `IA_Sprint`를 새로 만든다.
3. `IA_Sprint` 에셋의 Action Triggers에 `Hold`를 추가한다.
   - Hold Time Threshold `0.5`
   - Actuation Threshold `0.5`
   - Is One Shot `false`
4. `IMC_DefaultLocomotion`에서 `IA_SprintDodge` 매핑을 제거한다.
5. 게임패드 `Gamepad Face Button Right`를 `IA_Dodge`와 `IA_Sprint` 양쪽에 매핑한다.
6. 키보드는 기본값으로 `IA_Dodge=SpaceBar`, `IA_Sprint=Left Shift`를 권장한다. 같은 키를 원하면 두 액션에 같은 키를 매핑해도 된다.
7. `BP_ThirdPersonCharacter`의 `IA_Dodge.Started`에서 `InputManagerComponent.SubmitActionInput(Action.Input.Dodge)`를 호출한다.
8. `IA_Sprint.Triggered`에서 `CharacterInputState.WantsToSprint=true`로 설정한다.
9. `IA_Sprint.Completed`와 `Canceled`에서 `WantsToSprint=false`로 설정한다.
10. 기존 `IA_SprintDodge` 이벤트 그래프는 연결을 제거한다.

게임패드 B를 길게 눌러도 `IA_Dodge.Started`는 누르는 순간 먼저 발생한다. 따라서 장기 입력의 결과는 즉시 Dodge 후 0.5초가 지나면 Sprint 의도가 활성화되는 흐름이다. 탭 Dodge와 Hold Sprint를 상호 배타적으로 만들려면 Dodge를 Started에서 실행할 수 없고 버튼을 놓을 때 판정해야 하므로 현재 요구와 다르다.

키 리매핑을 실제 옵션 UI로 노출할 계획이라면 `IA_Dodge`와 `IA_Sprint`에 서로 다른 Player Mappable Key Settings 이름(`Dodge`, `Sprint`)을 부여한다. UE 5.6에서는 구형 Player Mappable Input Config보다 Enhanced Input User Settings 경로를 사용한다.

### Crouch

1. 기존 `IA_Crouch`의 C 매핑은 유지한다.
2. 같은 액션에 `Gamepad Left Thumbstick Button`을 추가한다.
3. `BP_ThirdPersonCharacter`의 `IA_Crouch.Started`에서 기존 `AttemptCrouch`를 호출한다.

키보드 C와 게임패드 L3 모두 같은 토글 로직을 사용한다.

### Q / R 스킬 Chord

1. Digital 타입 `IA_SkillModifier`를 새로 만든다. Action Trigger는 비워 둔다.
2. `IMC_DefaultLocomotion`에 `IA_SkillModifier=Gamepad Left Shoulder`를 추가한다.
3. 기존 `IA_SkillQ=Gamepad Face Button Top(Y)` 매핑은 그대로 둔다.
4. `IA_SkillR`에 `Gamepad Face Button Top(Y)` 매핑 행을 하나 추가한다.
5. 이 Y 매핑 행만 펼쳐 Mapping Triggers에 `Chorded Action`을 추가한다.
6. Chord Action을 `IA_SkillModifier`로 지정한다.
7. 기존 키보드 `IA_SkillR=R` 행에는 Chorded Action을 붙이지 않는다.

`IA_SkillModifier`에는 Pressed Trigger를 넣지 않는다. LB를 누르고 있는 동안 계속 Triggered 상태여야 Y를 눌렀을 때 Chord가 성립한다. Enhanced Input은 Chord 매핑을 우선 평가하고 같은 Y의 일반 `IA_SkillQ` 매핑에 Chord Blocker를 적용하므로 `Y=Q`, `LB+Y=R`로 분기된다. 만약 PIE에서 두 스킬이 함께 실행되면 두 Y 매핑이 같은 IMC에 있는지와 Chorded Action이 IA 에셋 전체가 아니라 `IA_SkillR`의 Y 매핑 행에 붙었는지 먼저 확인한다.

### 상호작용 후보 전환

1. `/Game/Input/Actions`에서 Digital(Boolean) 타입 `IA_SelectPreviousInteractable`과 `IA_SelectNextInteractable`을 만든다.
2. `IMC_DefaultLocomotion`에 다음 매핑을 추가한다.
   - `IA_SelectPreviousInteractable`: Left Arrow, Gamepad D-Pad Left
   - `IA_SelectNextInteractable`: Right Arrow, Gamepad D-Pad Right
3. `BP_ThirdPersonCharacter` Event Graph에 두 Input Action 이벤트를 추가한다.
4. 각각 `Started`에서 캐릭터의 `SelectPreviousInteractable` / `SelectNextInteractable`을 호출한다.
5. 반환값은 추가 동작이 필요하지 않으면 사용하지 않아도 된다.

`Triggered` 대신 `Started`를 사용해 한 번 누를 때 후보가 한 칸만 이동하게 한다.

### CommonUI 메뉴 WBP

1. 메뉴 루트 WBP의 부모를 `MVWindowBase` 계열로 둔다.
2. 선택 가능한 버튼은 `MVCommonButtonBase` 계열을 사용하고 `Is Focusable`을 켠다.
3. WBP에서 CommonUI의 `Get Desired Focus Target` 함수를 구현해 처음 선택할 버튼을 반환한다. 이름 기반 바인딩을 사용한다면 첫 버튼의 변수명을 `InitialFocusTarget`으로 둘 수도 있다.
4. 각 버튼의 Details > Navigation에서 Left/Right/Up/Down을 `Explicit`으로 바꾸고 이웃 버튼을 지정한다.
5. 가로 버튼 목록이라면 첫 버튼의 Left를 마지막 버튼으로, 마지막 버튼의 Right를 첫 버튼으로 지정하면 순환 이동한다.
6. 창은 단순 `Add to Viewport`가 아니라 `MVUISubsystem::PushWindowByClass` 또는 현재 Activatable Widget Stack 경로로 열고 활성화한다.

별도의 UI용 IMC에 Arrow/D-Pad를 추가할 필요는 없다. Slate 기본 내비게이션이 Left/Right Arrow, D-Pad Left/Right와 좌측 스틱을 이미 처리하고, `UMVCommonUIInputData`의 Generic Accept/Back이 A/Cross 및 B/Circle 동작을 담당한다.

### DualSense 직접 연결이 필요할 때

Steam Input/DS4Windows 등을 통해 XInput 장치로 인식되면 위의 표준 Gamepad 키만 사용한다. Windows에서 DualSense를 직접 USB 연결하고 표준 Gamepad 키가 들어오지 않을 때만 다음을 적용한다.

1. Plugins에서 `Raw Input`을 활성화하고 에디터를 재시작한다.
2. Project Settings > Raw Input에 Vendor ID `0x054C`, Product ID `0x0CE6` 장치를 추가한다.
3. ProjectBA 기준 버튼 번호는 Square=1, Cross=2, Circle=3, Triangle=4, L1=5, R1=6, R2=8, R3=12다.
4. IMC의 각 표준 Gamepad 키 옆에 필요한 `Generic USB Controller Button` 행을 추가한다.
5. XInput 변환 도구를 함께 사용할 때는 Generic USB 행을 비활성화하거나 제거해 액션이 두 번 발생하지 않게 한다.

RawInput 축은 장치별 보정과 반전 확인이 필요하므로 실제 DualSense 직접 연결 요구가 확정된 뒤 ProjectBA의 축 설정을 이식한다.
