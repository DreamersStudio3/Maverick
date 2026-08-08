# Graphify 위키를 Obsidian에서 보는 방법

Graphify가 만든 코드·문서 연결 지도를 Obsidian에서 찾아보는 방법을 설명한다. Obsidian에서 여는 폴더는 `graphify-out/obsidian/`이며, Obsidian에서 하나의 작업 공간으로 여는 폴더를 **vault**라고 부른다.

`graphify-out/obsidian/`은 Graphify가 다시 만드는 조회용 문서다. 직접 고쳐야 할 설명은 원본 코드나 `docs/wiki/`에 작성한다.

## 처음 한 번 연결하기

1. **Obsidian을 설치한다.** [공식 설치 안내](https://obsidian.md/help/Getting%2Bstarted/Download%2Band%2Binstall%2BObsidian)에서 운영체제에 맞는 설치 파일을 받는다. Windows에서는 `Universal` 설치 파일을 선택하면 된다.
2. **Graphify 결과가 있는지 확인한다.** Maverick 저장소의 `graphify-out/obsidian/` 아래에 `graph.canvas`, `_COMMUNITY_`로 시작하는 Markdown 문서, 그 밖의 노트가 있어야 한다. 폴더가 없다면 먼저 최신 브랜치를 받고, 그래도 없다면 아래의 [문제 해결](#문제-해결)을 확인한다.
3. **생성 폴더를 vault로 연다.** Obsidian을 실행하고 `Open folder as vault`의 `Open`을 누른 다음 `Maverick/graphify-out/obsidian/` 폴더를 선택한다. 이미 다른 vault가 열려 있다면 왼쪽 아래의 `Vault profile` → `Manage Vaults...` → `Open folder as vault` 순서로 연다. 저장소 루트나 `docs/wiki/`가 아니라 **`graphify-out/obsidian/` 자체**를 선택해야 한다. 화면 구성이 달라졌다면 [공식 vault 관리 안내](https://obsidian.md/help/manage-vaults)를 참고한다.
4. **`graph.canvas`를 연다.** 왼쪽 파일 목록에서 `graph.canvas`를 선택하면 전체 노트와 연결을 한 화면에서 볼 수 있다. `Space`를 누른 채 빈 공간을 드래그해 이동하고, `Ctrl`을 누른 채 마우스 휠을 움직여 확대·축소한다. `Shift+1`을 누르면 전체가 화면에 맞춰진다. 자세한 조작법은 [공식 Canvas 안내](https://obsidian.md/help/plugins/canvas)에 있다.
5. **필요한 항목을 검색한다.** Windows와 Linux에서는 `Ctrl+O`로 파일명을 빠르게 찾고 `Ctrl+Shift+F`로 모든 노트의 본문을 검색한다. macOS에서는 각각 `Cmd+O`, `Cmd+Shift+F`를 사용한다. 예를 들어 `MVActionComponent`, `DeathRespawnFlow`, `AI StateTree`처럼 타입이나 기능 이름을 입력한다. 검색 범위를 좁힐 때는 `file:MVActionComponent`, `content:"Death and Field Transition"` 같은 [검색 연산자](https://obsidian.md/help/plugins/search)를 사용할 수 있다.

## 평소에는 이렇게 찾아본다

1. 전체 영역을 훑을 때는 `graph.canvas`를 연다.
2. 관심 있는 묶음을 찾았으면 `_COMMUNITY_`로 시작하는 요약 노트를 연다. 이 노트는 서로 관련된 타입, 함수, 문서를 한곳에 모아 보여준다.
3. 요약 노트의 링크를 따라 개별 노트로 들어간다. 개별 노트에는 코드 심볼이나 문서 개념, 연결된 항목, 원본 파일 위치가 담겨 있다.
4. 실행 순서나 소유권처럼 Graphify만으로 판단하기 어려운 내용은 `docs/wiki/Architecture.md`를 함께 읽는다. 이 파일은 생성 vault 밖에 있으므로 저장소의 코드 편집기나 Git 뷰어에서 별도로 연다.
5. 실제 변경 전에는 노트가 가리키는 원본 코드와 Unreal Editor 에셋을 확인한다. Graphify 노트는 탐색을 돕는 지도이지 원본을 대신하지 않는다.

`Ctrl+O` 사용법은 [Quick switcher 안내](https://obsidian.md/help/plugins/quick-switcher), 전체 검색 방법은 [Search 안내](https://obsidian.md/help/plugins/search)에서 더 자세히 볼 수 있다.

## 생성 폴더 안의 파일

| 파일 | 용도 |
| --- | --- |
| `graph.canvas` | 전체 노트와 연결을 배치한 Canvas |
| `_COMMUNITY_*.md` | 관련 항목을 묶은 영역별 요약 |
| 그 밖의 `*.md` | 코드 심볼, 파일, 문서 개념을 나타내는 개별 노트 |
| `.obsidian/graph.json` | 팀에서 함께 쓰는 그래프 보기 설정 |

## Graphify가 갱신된 뒤

1. 최신 커밋을 받은 뒤 기존 vault를 그대로 연다. Obsidian은 폴더 밖에서 바뀐 파일도 자동으로 다시 읽는다.
2. Graphify가 노트를 합치거나 이름을 바꾸면 열려 있던 탭의 파일이 사라질 수 있다. 이때 `Ctrl+O` 또는 `Cmd+O`로 새 이름을 다시 찾는다.
3. 파일 목록이 계속 예전 상태라면 vault를 닫았다가 다시 열어 본다. 그래도 해결되지 않으면 Obsidian의 `Settings` → `Files and links`에서 `Rebuild vault cache`를 실행한다.
4. 갱신된 구조가 이상하다면 생성 노트를 직접 고치지 말고 원본 코드·`docs/wiki/`와 Graphify wrap-up 결과를 확인한다.

## 수정과 공유 원칙

- `graphify-out/obsidian/`의 Markdown과 `graph.canvas`는 직접 수정하지 않는다. 다음 Graphify 갱신 때 덮어써진다.
- 오래 남겨야 할 구조 설명과 결정은 `docs/wiki/`에, 타입 가까이에서 볼 책임과 계약은 C++ 헤더 문서 블록에 적는다.
- 개인 메모가 필요하면 생성 vault 밖의 별도 vault를 사용한다. 생성 폴더 안에 개인 문서를 두면 다음 갱신 때 없어질 수 있다.
- Graphify가 관리하는 노트, Canvas, `.obsidian/graph.json`은 공유한다. `workspace*.json`, `app.json` 같은 개인 화면 상태는 Git과 wrap-up 검사 대상에서 제외한다.

## 문제 해결

| 증상 | 확인할 내용 |
| --- | --- |
| `graphify-out/obsidian/`이 없다 | 먼저 Graphify 결과가 포함된 최신 커밋을 받았는지 확인한다. 프로젝트에 결과를 처음 만들 때는 `Documentation-Workflow.md`의 **초기화와 조회 명령**을, 기존 결과를 갱신할 때는 **원격 push 전** 절차를 따른다. |
| `graph.canvas`가 보이지 않는다 | vault로 연 폴더가 저장소 루트가 아니라 `graphify-out/obsidian/`인지 확인한다. |
| 검색 결과가 코드와 맞지 않는다 | 최신 커밋을 받은 뒤 vault를 다시 열어 본다. 그래도 낡았다면 최신 push에 Graphify wrap-up이 포함됐는지 확인한다. |
| 직접 고친 문장이 사라졌다 | 생성 문서는 재생성되므로 정상이다. 보존할 내용은 원본 코드나 `docs/wiki/`로 옮긴다. |
| Canvas가 너무 커서 보기 어렵다 | `Shift+1`로 전체를 맞춘 뒤 관심 영역을 확대하거나, `Ctrl+O`/`Cmd+O`로 `_COMMUNITY_` 요약부터 연다. |

## 관련 문서

- [Documentation-Workflow.md](Documentation-Workflow.md): 작업 중 문서 관리와 push 전 Graphify 갱신 절차
- [Architecture.md](Architecture.md): 프로젝트 책임 경계와 주요 실행 흐름
- [Graphify-Adoption-Report.md](Graphify-Adoption-Report.md): Graphify 도입 결정과 검증 결과
- [Obsidian 데이터 저장 방식](https://obsidian.md/help/data-storage): 외부 파일 변경 감지와 `.obsidian/` 설정 파일 설명
