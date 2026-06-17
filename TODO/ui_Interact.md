# UI Interaction TODO

상호작용 UI 작업 중 추후 정리해야 할 항목을 모은다.

## InteractionPromptPopup 후보 전환

- [x] Detector 범위 안에 여러 상호작용 대상이 동시에 들어온 경우, 화면에는 `InteractionPromptPopup`을 하나만 표시한다.
- [x] 후보 대상은 우선순위, 플레이어와의 거리, 카메라/시선 방향 등을 기준으로 정렬한다.
- [x] 키보드 화살표 좌우 입력과 게임패드 D-Pad/스틱 좌우 입력으로 현재 선택된 상호작용 후보를 이전/다음 대상으로 전환할 수 있게 C++ API를 추가한다.
- [ ] BP 입력 액션에서 좌우 입력을 `SelectPreviousInteractable` / `SelectNextInteractable`에 연결한다.
- [x] 선택된 후보가 바뀌면 프롬프트 텍스트와 대상 정보도 즉시 갱신한다.
- [ ] 여러 후보가 있을 때는 현재 선택 후보 아래에 나머지 후보가 쌓여 있는 상태처럼 보이게 WBP를 구성한다.
- [x] 상호작용이 완료된 후보는 후보 목록에서 제거하거나 상태를 갱신하고, 다음 후보가 자연스럽게 선택되도록 한다.

## Popup FadeInOut

- [x] `Popup` 계열 UI에 공통 FadeIn/FadeOut 효과를 추가한다.
- [x] 팝업이 화면에 표시될 때는 FadeIn으로 등장한다.
- [x] 팝업이 닫힐 때는 바로 제거하지 않고 FadeOut이 끝난 뒤 제거한다.
- [x] 자동 종료 타이머가 있는 팝업도 FadeOut 흐름을 거쳐 닫히도록 정리한다.
- [x] Fade 효과는 게임플레이 입력을 막지 않는 방식으로 동작해야 한다.

## DialogueWindow 정리

- [x] `DialogueWindow`가 중복 생성되는 버그를 수정한다.
- [x] 한 시점에 활성 Dialogue는 하나만 존재하도록 보장한다.
- [x] Dialogue 전용 DataTable row 타입을 구현한다.
- [ ] 실제 Dialogue DataTable 에셋과 manifest row를 생성한다.
- [x] Dialogue DataTable에는 화자, 대사 텍스트, 표시 시간, 다음 대사 ID, 음성 리소스 연결 지점 등을 포함할 수 있게 설계한다.
- [ ] Dialogue 음성 재생을 실제로 연결한다.
- [x] Dialogue를 입력으로 스킵할 수 있는 `SkipDialogueWindow` API를 추가한다.
- [ ] 여러 줄 Dialogue의 다음 대사 넘기기/시퀀스 진행 로직을 구현한다.

## Popup 표시 정책

- [x] `DialogueWindow`가 종료되면 `InteractionPromptPopup`이 FadeIn으로 다시 나타나야 한다.
- [x] `InteractionPromptPopup`이 `DialogueWindow` 아래에 계속 떠 있다가 Dialogue가 사라진 뒤 곧바로 보이는 구조가 아니어야 한다.
- [x] Popup은 한 화면에 한 개만 표시되도록 정책을 정리한다.
- [x] `UMVUISubsystem`에서 현재 활성 Popup을 추적하고, 새 Popup을 띄울 때 기존 Popup을 닫는 구조를 구현한다.
