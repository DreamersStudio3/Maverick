# 사람용 위키를 Obsidian에서 사용하는 방법

Maverick의 내부 위키: `docs/wiki/`의 Markdown 문서

Obsidian의 역할: 위키 열람·검색·링크 편집을 위한 편집기

Graphify의 역할: `graphify-out/`에 에이전트용 코드 관계 자료 생성

## 처음 한 번 연결하기

1. [Obsidian 공식 설치 안내](https://obsidian.md/help/getting-started/download-and-install-obsidian)에서 운영체제용 설치 파일 다운로드 및 설치
2. Maverick 저장소를 최신 상태로 받은 뒤 `docs/wiki/Architecture.md` 존재 여부 확인
3. Obsidian 실행 후 `Open folder as vault` 선택
4. Maverick 저장소의 **`docs/wiki/` 폴더 선택**
5. 왼쪽 파일 목록에서 `Architecture.md`를 열어 vault 연결 확인

이미 다른 vault가 열린 상태라면 왼쪽 아래 vault 메뉴에서 `Manage vaults`를 선택한 뒤 `Open folder as vault` 사용. 화면 문구가 달라진 경우 [공식 vault 관리 안내](https://obsidian.md/help/manage-vaults) 참고

## 권장 읽기 순서

1. `Architecture.md`: 전체 구조, 책임 경계, 주요 실행 흐름
2. 작업 도메인 문서: 전투 설계, 문서 운영, 헤더 문서화 등
3. 문서의 링크가 가리키는 원본 코드와 Unreal Editor 에셋

코드 심볼의 호출 관계나 관련 파일 탐색은 Obsidian 대신 에이전트의 `graphify query`, `path`, `explain` 사용

## 평소 작업 순서

1. 작업 시작 시 관련 위키 문서 확인
2. `docs/todo/<task>.md`의 `위키 검토` 항목에 예상 영향 기록
3. 구현 완료 후 기존 위키와 실제 코드·에셋 대조
4. 오래 남을 책임, 흐름, 설계 이유, 에셋 연결에 변화가 있으면 기존 문서 갱신
5. 기존 문서로 다루기 어려운 독립 주제만 새 문서로 추가
6. push 전 TODO에 `갱신: docs/wiki/<문서>.md` 또는 `변경 불필요: <이유>` 기록
7. PR 생성 시 같은 결과를 PR 설명의 `위키 검토` 항목으로 이동

모든 작업에서 위키 **검토**는 필수. 문서 의미가 달라지지 않은 작업까지 억지로 문장을 바꾸는 규칙은 없음

## 자주 쓰는 기능

- 빠른 문서 열기: Windows/Linux `Ctrl+O`, macOS `Cmd+O`
- 전체 내용 검색: Windows/Linux `Ctrl+Shift+F`, macOS `Cmd+Shift+F`
- 내부 링크: `[[Architecture]]`, `[[Documentation-Workflow#원격 push 전]]`
- 현재 문서를 가리키는 문서 확인: `Backlinks`
- 위키 문서 관계 확인: `Local graph`

검색과 링크 세부 사용법: [공식 Search 안내](https://obsidian.md/help/plugins/search), [공식 내부 링크 안내](https://obsidian.md/help/links)

## 편집과 공유 원칙

- 사람이 직접 편집하는 위치: `docs/wiki/`
- Graphify가 자동 생성하는 위치: `graphify-out/`
- 위키 문장 갱신 방식: 변경 이력 누적보다 현재 상태에 맞춘 기존 문단 재작성
- `.obsidian/` 설정: 개인 화면 상태이므로 Git 제외
- 새 문서 기준: 여러 타입·모듈·에셋을 가로지르는 장기 지식
- 타입 가까이 둘 내용: C++ 헤더의 Doxygen 문서 블록
- 작업 중 임시 판단: `docs/todo/`

## `graphify-out/obsidian/`을 열지 않는 이유

Graphify의 Obsidian export는 코드 노드마다 Markdown 파일을 만드는 기술 탐색 화면. 사람을 위한 설명문이 아니라 에이전트가 읽는 그래프를 노트 형태로 펼친 결과

Maverick의 기본 파이프라인에서는 해당 export를 생성하거나 Git으로 공유하지 않음. 필요할 때 개인 로컬 탐색용으로 직접 생성 가능하지만 팀 내부 위키로 편집 금지

```powershell
python -m graphify export obsidian --dir graphify-out/obsidian
```

위 명령의 결과는 `.gitignore` 대상. 공식 작업 지식은 계속 `docs/wiki/`에 기록

## 문제 해결

| 증상 | 확인 항목 |
| --- | --- |
| 코드 심볼별 기계적인 문서 수천 개 노출 | 잘못된 vault 선택. `graphify-out/obsidian/`이 아닌 `docs/wiki/`를 새 vault로 열기 |
| 위키 문서 미노출 | 선택한 폴더가 Maverick 저장소의 `docs/wiki/`인지 확인 |
| 다른 PC에서 화면 배치나 플러그인 설정 차이 | `.obsidian/`은 개인 설정이므로 정상. Markdown 내용만 팀 공유 |
| 코드와 문서 내용 불일치 | 원본 코드·에셋 확인 후 `docs/wiki/` 갱신, 이어서 push 전 Graphify wrap-up 수행 |
| 새 문서 필요 여부 불명확 | [Documentation-Workflow.md](Documentation-Workflow.md)의 디렉터리별 문서 원칙 확인 |

## 관련 문서

- [Architecture.md](Architecture.md): 프로젝트 구조와 주요 흐름
- [Documentation-Workflow.md](Documentation-Workflow.md): 작업별 위키 검토와 Graphify 갱신 절차
- [Header-Documentation.md](Header-Documentation.md): C++ 타입 가까이 둘 문서 기준
