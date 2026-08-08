# 문서와 Graphify 운영 워크플로

## 목적

Maverick의 문서는 사람이 빠르게 의도를 이해하고, 에이전트가 전체 코드를 다시 읽지 않고도 안전하게 작업을 시작할 수 있게 한다. 자동 생성 그래프와 사람이 관리하는 문서는 서로 대체하지 않고 각자 잘하는 정보를 맡는다.

## 정보 계층과 책임

| 위치 | 역할 | 편집 방식 |
| --- | --- | --- |
| `TODO/<task>.md` | 진행 중인 작업의 목표, 상태, 결정, 검증 | 작업자가 직접 갱신하는 단기 기록 |
| C++ 헤더의 `/** ... */` | 타입과 가까운 책임, 라이프사이클, 불변조건 | 해당 코드를 바꾼 작업자가 함께 갱신 |
| `docs/wiki/Architecture.md` | 현재 구조, 책임 경계, 주요 흐름과 바이너리 에셋 공백 | 구조 변경 시 사람이 검토해 갱신 |
| `docs/wiki/`의 나머지 문서 | 설계 의도, 운영 규칙, 장기 결정의 정본 | 사람이 검토해 직접 갱신 |
| `graphify-out/graph.json` | 코드와 문서 관계를 질의하는 지식 그래프 | Graphify로만 생성 |
| `graphify-out/wiki/` | 커뮤니티 단위의 에이전트 탐색용 위키 | Graphify로만 생성 |
| `graphify-out/obsidian/` | 노드 단위의 Obsidian 탐색 vault | Graphify로만 생성 |
| `graphify-out/GRAPH_REPORT.md` | 그래프 상태, 중심 노드, 연결, 질문의 감사 보고서 | Graphify로만 생성 |

`docs/wiki/`는 "왜 이 경계가 존재하는가"를 설명하고, Graphify는 "현재 어떤 선언과 파일이 실제로 연결되는가"를 계산한다. 생성 산출물을 손으로 고치면 다음 갱신 때 사라지므로 정정은 원본 코드나 `docs/wiki/`에 반영한다.

## 갱신 타이밍

### 1. 프롬프트를 받을 때

이 시점에는 Graphify를 재생성하지 않는다.

1. `POLICY.md`와 관련 지침을 확인한다.
2. `TODO/<task>.md`에 목표와 초기 체크리스트를 만든다.
3. 기존 `graphify-out/graph.json`이 있으면 `graphify query`로 관련 구조를 먼저 좁힌다.
4. 그래프 근거가 부족하거나 stale 가능성이 있을 때만 원본 코드를 추가로 읽는다.

프롬프트마다 그래프를 만들면 아직 채택되지 않은 실험과 중간 상태가 반복 반영되고 생성 파일 churn이 커진다. 프롬프트 시점의 문맥은 TODO가 담당한다.

### 2. 커밋을 만들기 전

커밋은 하나의 일관된 코드 스냅샷이므로 일상적인 Graphify 갱신 시점으로 사용한다.

1. 변경이 코드뿐이면 `graphify update .`를 실행한다. PATH에 명령이 없으면 `python -m graphify update .`를 사용한다.
2. `docs/wiki/`나 설계 문서가 바뀌었으면 Codex에서 `$graphify . --update`를 사용해 의미 추출까지 수행한다.
3. C++ `/** ... */`만 바뀐 경우에도 manifest 최신화를 위해 일반 증분 갱신을 실행한다. Graphify 0.9.36은 C++ 주석 내용을 의미 노드로 추출하지 않으므로, 여러 타입에 걸친 중요한 의도는 `docs/wiki/`에도 반영한다.
4. 기존 `graphify-out/wiki/`와 `graphify-out/obsidian/`이 있으면 둘 다 다시 내보낸다.
5. 변경한 정본과 생성 산출물을 같은 작업 단위의 커밋에 포함한다.

Graphify의 공식 Git 훅은 커밋 뒤 코드 AST를 저비용으로 갱신하는 안전망이다. 문서의 의미 추출은 처리하지 않으므로 그것만으로 문서 최신성을 보장했다고 판단하지 않는다.

### 3. 원격 push 전

pre-push는 누락을 막는 최종 게이트다. push할 커밋에 지식 원본 변경이 있으면 다음 wrap-up을 수행한다.

1. TODO의 완료 상태와 미해결 항목을 확인한다.
2. `$graphify . --update`로 코드와 문서 변경을 모두 반영한다.
3. 그래프 상태 검사를 확인하고 `wiki` 및 `obsidian` export를 재생성한다.
4. 구조적 책임이나 경계가 달라졌으면 `docs/wiki/Architecture.md`를 갱신한다.
5. 생성 산출물과 정본 문서를 검토하고 wrap-up 최신성 표식을 만든다.
6. 변경을 커밋한 뒤 pre-push 훅을 다시 통과시킨다.

훅은 의미 문서의 품질을 대신 판단하지 않는다. 대신 마지막 wrap-up 이후 코드나 정본 문서가 달라졌는지를 결정론적으로 검사하고, stale 상태라면 push를 중단한다.

## 초기화와 조회 명령

Codex에서 프로젝트 전체를 처음 구축할 때 사용한다.

```text
$graphify . --wiki --obsidian
```

PowerShell에서 CLI를 직접 호출할 때는 선행 `/`를 쓰지 않는다.

```powershell
graphify query "액션 입력에서 전투 실행까지의 흐름"
graphify path "MVInputManagerComponent" "MVCombatComponent"
graphify explain "MVActionComponent"
```

현재 셸처럼 실행 파일이 PATH에 없으면 `graphify`를 `python -m graphify`로 바꾼다.

생성 wiki와 Obsidian vault는 `update`나 공식 Git 훅이 자동으로 다시 만들지 않으므로 wrap-up에서 명시적으로 실행한다.

```powershell
python -m graphify export wiki
python -m graphify export obsidian --dir graphify-out/obsidian
python -m graphify export html
```

## Obsidian 사용

Obsidian에서 저장소의 `graphify-out/obsidian/` 디렉터리를 vault로 연다. 사람이 작성하는 장기 문서는 `docs/wiki/`에 유지하고, Obsidian 생성 vault는 그래프 탐색용 읽기 모델로 취급한다. 둘을 한 디렉터리에 섞지 않아야 재생성 때 수동 문서가 덮어쓰이지 않는다.

## 디렉터리별 문서 원칙

`Source/**/README.md` 같은 디렉터리별 목차를 일괄 생성하지 않는다. 파일 목록과 심볼 관계는 Graphify가 더 정확하게 계산하며, 수동 목차는 코드 이동 때 쉽게 낡는다.

다음 조건을 모두 만족할 때만 `docs/wiki/<Domain>.md`를 추가한다.

- 세 개 이상의 타입이나 code/asset 경계를 가로지른다.
- 책임과 호출 순서만으로는 이유나 운영 계약을 설명하기 어렵다.
- `Architecture.md` 한 문단으로는 변경 시 검토 기준을 보존할 수 없다.

AI StateTree, 사망·부활, 테이블 파이프라인처럼 이미 독립 설계 문서가 있는 도메인은 새 문서를 중복 생성하지 않고 기존 정본을 갱신한다.
