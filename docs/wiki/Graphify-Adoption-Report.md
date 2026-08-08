# Graphify 내부 위키 도입 보고서

## 요약

프로젝트 매버릭의 내부 위키는 사람이 관리하는 위키 문서와 Graphify가 만드는 읽기 모델로 분리한다. 작업 시작에는 TODO와 기존 그래프 쿼리를 사용하고, 개별 커밋 직후에는 저비용 AST 증분 갱신을 수행한다. 지원되는 브랜치·커밋 전환에서는 새 snapshot의 전체 코드 graph를 다시 만들고, 원격 push 직전에는 문서 의미 추출, graph 진단, wiki·Obsidian·HTML export, 최신성 stamp를 한 번에 커밋한 뒤 pre-push Git hook 게이트를 거친다.

핵심 결론은 다음과 같다.

- Graphify는 코드와 문서의 현재 관계를 좁히는 검색 지도다. 책임, lifecycle, 바이너리 에셋 경계를 설명하는 `Architecture.md`는 계속 필요하다.
- C++의 `/** ... */`는 JSDoc이 아니라 Doxygen 형식 문서 주석으로 보고 유지한다. 모든 헤더의 형식적 보유율보다 비자명한 주요 타입의 로컬 계약을 우선한다.
- `docs/todo/`는 작업 중 실행 맥락, `docs/wiki/`는 장기 위키, `MaverickDesign/`은 테이블 원본·도구·schema 공간, `graphify-out/`은 재생성 가능한 읽기 모델이다.
- 각 소스 디렉터리에 `README.md`를 자동 생성하지 않는다. 파일 목록과 심볼 관계는 Graphify에 맡기고, 여러 타입·에셋 경계를 가로지르는 설계 근거만 위키 문서로 만든다.

도입과 검증은 로컬 `graphifyy 0.9.36`, [Graphify 공식 저장소](https://github.com/Graphify-Labs/graphify), [공식 PyPI 패키지](https://pypi.org/project/graphifyy/)를 기준으로 수행했다.

## 갱신 시점 결정

| 시점 | 필수 행동 | 이 시점에 하지 않는 일 |
| --- | --- | --- |
| 프롬프트 수신 | `POLICY.md` 확인, `docs/todo/<task>.md` 생성·갱신, 기존 graph를 `query`/`path`/`explain`으로 조회 | Graphify 재생성 |
| 일반 커밋 | 코드와 위키 문서를 함께 커밋하고 공식 `post-commit` 훅의 비동기 AST 증분 갱신을 로컬 질의 안전망으로 사용 | 중간 `graphify-out/`만 별도 커밋, 문서 의미 추출 완료로 간주 |
| switch/checkout으로 브랜치·커밋 전환 | 조건을 만족하면 공식 `post-checkout` 훅으로 새 snapshot의 전체 코드 graph를 비동기 재구축하고, 즉시 질의할 때 현재 전환의 완료 로그 확인 | reset·restore·pull까지 포괄한다고 간주, 문서 의미·wiki·Obsidian 최신성 보장으로 간주 |
| 원격 push 전 | `$graphify . --update` 전체 의미 갱신, 진단·라벨 검토, wiki·Obsidian·HTML export, stage, stamp, wrap-up 커밋, guard 검증 | 훅 안에서 LLM 추출이나 대규모 export 실행 |

프롬프트마다 그래프를 갱신하면 아직 확정되지 않은 실험 내용이 반영되어 그래프가 금방 더러워질 수 있다. 반대로 push 직전에만 갱신하면 작업이 길어졌을 때 개발 의도 같은 것들이 빠질 수 있다. 따라서 **post-commit + post-checkout + pre-push** 조합을 채택했다.

현재 0.9.36 공식 `post-checkout`은 Git의 세 번째 hook 인자가 `1`인 switch/checkout 계열 전환에서만 실행한다. 변경 파일 목록이 없으므로 새 snapshot의 전체 코드 corpus를 재추출하지만, 백그라운드 작업이며 기존 semantic 문서 노드는 보존될 수 있고 wiki·Obsidian export까지 보장하지 않는다. 경로 단위 checkout은 인자가 `0`이라 건너뛰며, merge·rebase·cherry-pick 중, `graphify-out/`이 없는 브랜치와 linked worktree에서도 실행하지 않는다. 로그에는 대상 ref 표식이 없으므로 현재 전환 뒤 추가된 결과를 확인해야 한다. Maverick의 `pre-push`는 생성 작업을 하지 않고, outgoing commit의 코드·위키·manifest·semantic coverage·생성물·stamp가 같은 Git 스냅샷인지 빠르게 검증한다.

## Obsidian에서 Graphify 문서 보기

1. **Obsidian을 설치한다.** [Obsidian 공식 다운로드 페이지](https://obsidian.md/download)에서 운영체제에 맞는 설치 파일을 내려받아 설치한다. Windows에서는 `Download for Windows`를 선택하고 설치 파일을 실행하면 된다.
2. **Graphify 문서 폴더를 확인한다.** Maverick 저장소를 최신 상태로 받은 뒤 `graphify-out/obsidian/` 폴더와 그 안의 `graph.canvas`가 있는지 확인한다. 이 폴더 전체가 Obsidian에서 열 vault다.
3. **폴더를 vault로 연결한다.** Obsidian을 처음 실행했다면 `Open folder as vault` 옆의 `Open`을 선택하고 `Maverick/graphify-out/obsidian/` 폴더를 지정한다. 다른 vault가 이미 열려 있다면 왼쪽 아래 `Vault profile` → `Manage Vaults...` → `Open folder as vault` 순서로 같은 폴더를 연다. 화면 위치가 달라졌다면 [Obsidian의 vault 관리 안내](https://obsidian.md/help/manage-vaults)를 참고한다.
4. **전체 구조를 먼저 살펴본다.** 왼쪽 파일 탐색기에서 `graph.canvas`를 열면 코드와 문서의 연결을 한 화면에서 볼 수 있다. 빈 공간을 드래그해 이동하고 마우스 휠로 확대·축소한다. Canvas 조작법은 [Obsidian Canvas 안내](https://obsidian.md/help/plugins/canvas)에서 확인할 수 있다.
5. **필요한 문서를 검색한다.** `Ctrl+O`를 눌러 파일명으로 노트를 찾거나 `Ctrl+Shift+F`를 눌러 전체 본문을 검색한다. `_COMMUNITY_`로 시작하는 노트는 관련 코드와 개념을 묶은 요약 문서이며, 일반 노트의 링크를 따라가면 연결된 타입·함수·원본 파일을 확인할 수 있다. 자세한 단축키는 [Quick switcher](https://obsidian.md/help/plugins/quick-switcher)와 [Search](https://obsidian.md/help/Plugins/Search) 안내를 따른다.
6. **생성 문서는 조회용으로만 사용한다.** `graphify-out/obsidian/` 안의 Markdown과 Canvas를 직접 수정해도 다음 Graphify 갱신에서 덮어써진다. 장기 보존할 설명은 `docs/wiki/`에 작성하고, 생성 vault는 최신 Graphify 결과를 탐색하는 용도로 사용한다.

## `Architecture.md`를 함께 관리하는 이유

Graphify는 관련된 정보와 파일을 빠르게 찾고 연결 경로를 좁히는 데 유용하다. 다만 Maverick의 Graphify 입력에는 Unreal Editor의 `Content/`와 생성 디렉터리가 포함되지 않는다. 그래서 Blueprint, WBP, StateTree, Chooser, Montage, DataTable 같은 에셋의 연결은 그래프에 나타나지 않을 수 있고, 코드만으로는 컴포넌트의 소유권이나 실제 실행 순서를 알기 어려운 경우도 있다.

이 빈 부분은 `docs/wiki/Architecture.md`에 기록한다. 이 문서는 파일 목록을 다시 적는 곳이 아니라, 프로젝트의 큰 책임 경계와 Graphify가 읽지 못한 에셋 연결을 모아 두는 첫 진입점이다. 에이전트와 개발자는 다음 순서로 프로젝트를 살펴본다.

1. `Architecture.md`에서 전체 구조, 주요 책임, 에셋 확인이 필요한 부분을 파악한다.
2. `graphify query`, `path`, `explain`으로 작업과 관련된 심볼과 파일을 좁힌다.
3. 좁혀진 원본 코드와 Unreal Editor의 관련 에셋을 직접 확인한다.

### 무엇을 기록하는가

- 모듈과 디렉터리의 역할, 주요 컴포넌트와 subsystem의 소유권 및 수명
- 입력, Action, hit, stat, hit reaction, death, field transition으로 이어지는 주요 실행 흐름과 판단 주체
- Blueprint, WBP, StateTree, Chooser, Montage, DataTable 등 코드 밖 에셋의 연결
- 현재 코드에서 확인한 구현과 `docs/wiki/`에 적힌 설계 목표의 차이
- 세부 내용을 확인할 수 있는 대표 코드, 설정, 설계 문서와 에셋의 진입점

개별 함수의 구현이나 모든 파일 목록까지 옮겨 적지는 않는다. 그런 정보는 Graphify와 원본 코드에서 확인하고, `Architecture.md`에는 여러 파일과 에셋을 함께 봐야 이해되는 내용만 남긴다.

### 언제 갱신하는가

다음과 같은 변경을 한 작업자는 코드나 에셋 변경과 같은 작업 단위에서 `Architecture.md`도 함께 갱신한다.

- 모듈, 플러그인 의존성 또는 디렉터리의 역할이 달라진 경우
- 컴포넌트나 subsystem의 소유권, 수명, 판단 권한이 달라진 경우
- 입력, 전투, 피격, 사망, 필드 전환, AI, UI, 테이블의 주요 흐름이나 순서가 달라진 경우
- C++와 Blueprint 사이의 책임을 옮기거나 `.uasset` 연결을 추가·변경한 경우
- 주요 타입이나 파일을 이동·이름 변경·삭제해 기존 진입점이 맞지 않게 된 경우
- Graphify 조회 결과와 문서 내용이 달라 원본을 확인한 결과, 문서가 낡았다고 확인된 경우

함수 내부 구현만 바뀌고 위 책임이나 흐름에 영향이 없다면 `Architecture.md`를 고치지 않아도 된다.

### 어떻게 갱신하는가

1. 관련 코드, 설정, 설계 문서와 Unreal Editor 에셋을 먼저 확인한다.
2. 변경 이력을 문장 끝에 계속 덧붙이지 않고, 기존 문단·표·다이어그램을 현재 상태에 맞게 다시 정리한다.
3. 코드에서 확인한 사실, 아직 구현되지 않은 설계 목표, 에디터에서 추가 확인할 내용을 구분해 적는다. 확인하지 못한 에셋 연결은 추측하지 않고 `에셋 확인 필요`로 남긴다.
4. 구조를 바꾼 작업자가 코드와 `Architecture.md`를 같은 커밋에 포함한다.
5. push 전 wrap-up에서 문서와 코드가 맞는지 마지막으로 확인한 뒤 Graphify를 갱신한다. `graphify-out/`의 생성 문서는 직접 수정하지 않는다.

## C++ 헤더 문서화 결론

2026-08-08 `Source/Maverick/**/*.h` 단순 감사 결과는 다음과 같다.

| 항목 | 수량 |
| --- | ---: |
| 프로젝트 자체 헤더 | 138 |
| `/**` 포함 파일 | 71 |
| `/**` 미포함 파일 | 67 |
| 내용 없는 블록 포함 파일 | 5 |

이 수치는 보유율이지 품질 지표가 아니다. 상태를 소유하는 Component·Subsystem·Manager·Controller·Character, paired lifecycle, 이벤트 구독 해제, 비자명한 public contract를 우선 문서화한다. enum, 자명한 POD, 얇은 wrapper와 boilerplate는 생략할 수 있다.

우선 보완 가치가 큰 주 선언은 다음 여섯 개다.

- `UMVUISubsystem`
- `UMVTableManager`
- `UMVTableAssetGenerator`
- `UMVFinisherComponent`
- `FMVUIFadeController`
- `AMVAIController`

로컬 `graphifyy 0.9.36`의 C++ extractor는 class, function, include, call 구조는 추출하지만 Doxygen 주석 본문을 의미 노드로 만들지 않는다. 따라서 헤더 블록은 인간과 에이전트가 타입 가까이에서 읽는 로컬 계약으로 유지하고, 여러 타입에 걸친 구조와 설계 근거는 의미 추출 대상인 `docs/wiki/`에 둔다.

## TODO와 Graphify의 역할

두 체계는 병합 가능한 lifecycle을 가지지만 같은 저장소 역할을 맡지 않는다.

| 단계 | TODO | 위키 문서 | Graphify |
| --- | --- | --- | --- |
| 시작·진행 | 목표, 체크리스트, 임시 결정, 검증 상태 | 필요할 때만 갱신 | 기존 graph 조회 |
| 구조 결정 확정 | 완료 상태와 근거 링크 기록 | 장기 가치가 있는 책임·설계·운영 지식 반영 | 아직 재생성하지 않아도 됨 |
| push 전 | 미해결 항목 확인·종료 정리 | 코드와 일치하는지 최종 검토 | 위키에서 의미 추출하고 읽기 모델 재생성 |

`docs/todo/`는 `.graphifyignore`에서 제외했다. 미완성 가설과 작업 로그가 지식 그래프를 오염시키지 않게 하기 위해서다. 작업 종료 때 장기 가치가 있는 결정만 `docs/wiki/`로 옮기고 완료 파일은 삭제한다. 과거 완료 기록 중 다시 찾을 가치가 있는 결과만 `docs/todo/Completed-Work.md`에 짧게 합친다.

## 추가·변경된 파일

| 범주 | 파일 또는 경로 | 역할 |
| --- | --- | --- |
| 상위 행동강령 | `AGENTS.md`, `POLICY.md` | query-first, 문서 책임, 커밋·switch/checkout·pre-push 규칙의 진입점 |
| 입력·Git 규칙 | `.graphifyignore`, `.gitignore`, `.gitattributes` | corpus 제외, 로컬 cache 제외, LF와 graph merge driver |
| 단기 작업 기록 | `docs/todo/README.md`, `docs/todo/Completed-Work.md` | TODO 생성·갱신·종료 규칙과 압축된 완료 결과 |
| 사람이 관리하는 위키 | `docs/wiki/Documentation-Workflow.md` | 단일 운영 절차 |
| 사람이 관리하는 위키 | `docs/wiki/Architecture.md` | 현재 구조, 책임, 주요 흐름, 에셋 검증 공백 |
| 사람이 관리하는 위키 | `docs/wiki/Header-Documentation.md` | Doxygen 대상·품질·생략 기준 |
| 사람이 관리하는 위키 | `docs/wiki/Graphify-Adoption-Report.md` | 도입 결정, 지표, 근거, 한계 |
| 공유 도구 | `Scripts/Graphify/Install-Hooks.ps1` | 공식 훅·merge driver와 Maverick pre-push 설치·상태·제거 |
| 공유 도구 | `Scripts/Graphify/hooks/pre-push` | outgoing ref를 guard로 전달 |
| 공유 도구 | `Scripts/Graphify/knowledge_guard.py` | export provenance, stage stamp, commit 최신성 검증 |
| 생성 읽기 모델 | `graphify-out/graph.json`, `GRAPH_REPORT.md`, `graph.html`, `manifest.json` | 관계 graph, 감사 보고서, 시각화, 증분 기준 |
| 생성 읽기 모델 | `graphify-out/wiki/`, `graphify-out/obsidian/` | 에이전트용 커뮤니티 wiki와 Obsidian vault |
| 최신성 증거 | `graphify-out/export-provenance.json`, `wrap-up.json` | export exact hash와 Git snapshot attestation |

Codex에서 강제 장치로 오인될 수 있던 `.codex/hooks.json`의 `graphify hook-check`는 제거했다. Graphify 0.9.36의 Codex hook-check는 호환을 위한 의도적 no-op이며, 실제 query-first 규칙은 `AGENTS.md`, push 최신성은 저장소 pre-push guard가 담당한다.

## 단계별 커밋과 근거

| 커밋 | 작업 | 주요 근거 |
| --- | --- | --- |
| `bc27495` | Graphify 위키 운영 기준 | 기존 `AGENTS.md`, `POLICY.md`, TODO 관행, Graphify skill의 update·hook·export 지침 |
| `1ad5bb6` | 초기 graph·wiki·Obsidian 생성 | `.graphifyignore`로 감지한 303파일 corpus, Graphify AST·문서 의미 추출, 진단·라벨·export 결과 |
| `9f57fd7` | Architecture와 헤더 기준 | Graphify query 결과, `Source/Maverick/`, `Plugins/LockOnTarget/`, `.Build.cs`·`.uproject`·`.ini`, 당시 설계 자료, 138개 헤더 감사 |
| `3aa7907` | pre-push 최신성 검사 | 공식 `graphify hook install/status`, 설치된 0.9.36 구현 확인, manifest·semantic hash·export 구조·Git blob 검증 테스트 |
| 최종 wrap-up | 본 보고서 의미 추출, 생성 뷰·provenance·stamp 갱신 | 위 위키 문서 전체와 최종 staged snapshot; 정확한 hash는 이 보고서를 포함한 커밋 후 Git 이력에서 확인 |
| 후속 브랜치 전환 보완 | `post-checkout` 갱신 시점·조건·예외 문서화 | 설치된 0.9.36 훅과 `graphify/hooks.py`, 전역 skill의 `references/hooks.md` 대조 |

외부 기준은 [Graphify 공식 저장소](https://github.com/Graphify-Labs/graphify)와 [graphifyy PyPI](https://pypi.org/project/graphifyy/)다. 실행 절차는 전역 Graphify skill의 `SKILL.md`, `references/update.md`, `references/hooks.md`, `references/exports.md`, `references/extraction-spec.md`를 사용했다. `references/hooks.md`는 주로 post-commit을 설명하므로 post-checkout의 세부 조건은 설치된 훅, `graphify/hooks.py`, `graphify/watch.py`를 직접 대조했다. 프로젝트 사실은 Graphify 결과만 믿지 않고 관련 C++·설정·설계 문서와 대조했다.

## 지표

아래 값은 초기 도입 작업의 진행과 `7f7945e` 도입 완료 snapshot을 보존한 역사적 기준선이다. 이번 후속 보완을 포함한 실시간 수치는 이 문서에 반복 기록하지 않고 생성물인 `graphify-out/GRAPH_REPORT.md`와 export provenance를 기준으로 확인한다.

| 지표 | 초기 `1ad5bb6` | 도입 중간 snapshot | 도입 완료 `7f7945e` |
| --- | ---: | ---: | --- |
| corpus | 303파일 | 305파일 | 309파일 |
| 추정 단어 | 114,275 | 116,535 | 121,631 |
| graph node | 3,814 | 3,843 | 3,905 |
| graph relation | 5,558 | 5,585 | 5,738 |
| community | 268 | 277 | 281 |
| hyperedge | 9 | 9 | 11 |
| wiki Markdown, index 포함 | 279 | 288 | 292 |
| Obsidian Markdown | 4,082 | 4,120 | 4,186 |

도입 완료 당시 canvas는 3,905개 file node와 281개 community group, 합계 4,186개 node를 가졌다. 당시 export provenance는 wiki 292개와 Graphify 소유 Obsidian 파일 4,189개의 exact tree hash를 기록했다. 초기 token-reduction benchmark는 전체를 순진하게 읽는 약 254,266 tokens 대비 평균 질의 문맥 약 5,335 tokens, 약 47.7배 감소를 측정했다. 이는 탐색 문맥량 비교이며 답변 정확도나 실행 시간의 보장은 아니다.

## 검증 결과

- 설치 환경은 Python 3.14의 `graphifyy 0.9.36`이며 PATH에 `graphify`가 없을 때 `python -m graphify`로 실행됨을 확인했다.
- 공식 `post-commit`, `post-checkout` 훅과 `merge.graphify` driver, Maverick `pre-push`가 설치됨을 `Install-Hooks.ps1 -Action Status`로 확인했다.
- 설치된 `post-checkout`이 checkout 구분 인자 `1`만 허용하고, 전환 뒤 전체 코드 corpus를 비동기 재구축하며 경로 단위 checkout, merge·rebase·cherry-pick 중, 결과 디렉터리가 없는 브랜치와 linked worktree에서는 건너뛰는 것을 훅 본문과 설치 패키지에서 확인했다.
- installer 재실행, 상태 확인, pinned Python, LF·실행 모드, 기존 foreign hook 거부와 저장소 밖 shared hook 경로 거부를 확인했다.
- guard가 stage되지 않은 위키·코드·산출물, stale semantic hash, 불일치 export provenance, stamp 없는 commit을 거부함을 확인했다.
- 삭제 ref만 있는 push는 검사할 commit이 없으므로 no-op으로 통과함을 확인했다.
- Obsidian 개인 UI 상태는 생성물 fingerprint에 포함하지 않고 Graphify 소유 파일만 검증한다.
- 최종 증분 병합의 post-build extraction 진단에서는 dangling·missing endpoint, self-loop, endpoint collapse가 모두 0이었다. 초기 raw extraction 경고는 별도 한계로 계속 보존한다.

최종 인수 조건은 wrap-up commit 후 `python Scripts/Graphify/knowledge_guard.py check --commit HEAD` 통과다.

## 알려진 한계와 후속 조치

- 초기 진단은 1,362개 dangling-endpoint relation과 동일 endpoint collapse 145개(directed 해석), 147개(undirected 해석)를 보고했다. graph는 탐색에 사용할 수 있지만 완전한 호출 graph로 간주하지 않고, 매 wrap-up에서 진단을 공개·비교해야 한다.
- `GRAPH_REPORT.md`와 `cost.json`의 0 input/0 output은 협업 에이전트 결과 API가 실제 usage를 제공하지 않아 기록된 값이다. 의미 추출 비용이 실제로 0이라는 뜻이 아니다.
- 로컬 Git hook은 clone마다 설치해야 하며 `git push --no-verify`로 우회할 수 있다. 팀 강제가 필요하면 CI에서 `knowledge_guard.py check --commit <SHA>`를 필수 check로 실행하고 branch protection을 건다.
- Windows의 Graphify rebuild 잠금은 0.9.36에서 no-op fallback이므로 빠른 연속 switch/checkout은 백그라운드 갱신끼리 경쟁할 수 있다. 현재 전환의 재구축 완료를 확인한 뒤 다음 전환이나 graph 질의를 수행한다.
- 커뮤니티가 재배치되면 wiki와 Obsidian 파일명이 대량 변경될 수 있다. 일반 커밋마다 생성물을 올리지 않고 pre-push wrap-up 한 커밋으로 churn을 모은다.
- C++ Doxygen 본문은 0.9.36 의미 graph에 들어가지 않는다. 교차 타입 의도는 위키 Markdown에도 반영해야 한다.
- Graphify는 텍스트 corpus의 지도이며 Blueprint·StateTree·Chooser·Montage·DataTable 내부와 런타임 순서의 최종 증거가 아니다. 해당 판단은 `Architecture.md`의 `에셋 확인 필요` 표시와 Unreal Editor 검증을 따른다.
- 전역 설치 버전이 바뀌면 output schema와 hook 동작이 달라질 수 있다. 업그레이드 시 공식 release와 guard 호환성을 먼저 확인한다.

## 운영 quickstart

새 clone에서 한 번 실행한다.

```powershell
./Scripts/Graphify/Install-Hooks.ps1
./Scripts/Graphify/Install-Hooks.ps1 -Action Status
```

작업을 시작할 때는 `docs/todo/`에 짧은 작업 파일을 만들고 기존 graph부터 조회한다.

```powershell
python -m graphify query "변경할 기능의 책임과 호출 경로는?"
python -m graphify path "시작 심볼" "도착 심볼"
python -m graphify explain "핵심 심볼"
```

일반 구현 커밋 뒤에는 공식 hook의 AST 증분 갱신을 기다리되, 그 중간 `graphify-out/`은 stage하지 않는다. switch/checkout으로 브랜치나 커밋을 바꾼 뒤에는 훅 실행 조건과 현재 전환의 전체 코드 재구축 결과를 확인한 다음 graph를 질의한다. 훅이 건너뛴 상태에서 즉시 코드 graph가 필요하면 `python -m graphify update .`를 동기 실행한다. push 전에는 Codex의 Graphify skill로 전체 의미 갱신을 수행한 뒤 다음 순서를 따른다.

```powershell
# Codex에서: $graphify . --update
python Scripts/Graphify/knowledge_guard.py export

# 변경한 위키·코드와 graphify-out 생성물을 명시적으로 stage
python Scripts/Graphify/knowledge_guard.py stamp
git add graphify-out/wrap-up.json

$env:GRAPHIFY_SKIP_HOOK = "1"
git commit -m "문서 수정: Graphify wrap-up 갱신"
Remove-Item Env:GRAPHIFY_SKIP_HOOK

python Scripts/Graphify/knowledge_guard.py check --commit HEAD
git push
```

stamp는 stage된 snapshot을 대상으로 하므로 위키·코드와 생성물을 먼저 stage해야 한다. 검사가 실패하면 우회하지 말고 오류가 지목한 semantic extraction, export, stage 또는 stamp 단계를 다시 수행한다.
