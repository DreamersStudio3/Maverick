# 문서와 Graphify 운영 워크플로

## 목적

Maverick의 문서는 사람이 빠르게 의도를 이해하고, 에이전트가 전체 코드를 다시 읽지 않고도 안전하게 작업을 시작할 수 있게 한다. 자동 생성 그래프와 사람이 관리하는 문서는 서로 대체하지 않고 각자 잘하는 정보를 맡는다.

## 정보 계층과 책임

| 위치 | 역할 | 편집 방식 |
| --- | --- | --- |
| `TODO/<task>.md` | 진행 중인 작업의 목표, 상태, 결정, 검증 | 작업자가 직접 갱신하는 단기 기록 |
| C++ 헤더의 `/** ... */` | 타입과 가까운 책임, 라이프사이클, 불변조건 | 해당 코드를 바꾼 작업자가 함께 갱신 |
| `docs/wiki/Architecture.md` | 현재 구조, 책임 경계, 주요 흐름과 바이너리 에셋 공백 | 구조 변경 시 사람이 검토해 갱신 |
| `docs/wiki/`의 나머지 문서 | 설계 의도, 운영 규칙, 장기 결정을 담는 위키 | 사람이 검토해 직접 갱신 |
| `MaverickDesign/` | 도메인 상세 설계, 테이블 데이터와 schema의 기존 위키 | 해당 기능과 데이터 변경 시 기존 문서를 갱신 |
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

### 2. 커밋할 때

커밋 시점을 일상적인 저비용 갱신 시점으로 사용한다. 저장소에 설치한 Graphify 공식 `post-commit` 훅이 코드 AST를 비동기로 증분 갱신한다.

1. 먼저 코드와 사람이 관리하는 위키 문서를 하나의 일관된 작업 단위로 커밋한다.
2. 공식 훅이 `graphify update`를 실행해 로컬 질의용 graph와 manifest를 따라오게 한다.
3. 훅이 바꾼 graph만으로 wiki·Obsidian을 갱신하거나 커밋하지 않는다. 문서 의미 노드와 생성 뷰가 아직 이전 상태일 수 있기 때문이다.
4. 다음 일반 커밋에서는 `graphify-out/` 중간 변경을 stage하지 않고, push 전 wrap-up에서 한 번에 정리한다.

이 선택은 프롬프트마다 전체 그래프를 만드는 비용을 피하면서 긴 작업 중 로컬 코드 관계가 지나치게 낡는 것도 막는다. `docs/wiki/`나 설계 문서 의미 추출, 커뮤니티 이름 갱신, wiki·Obsidian export는 최종 pre-push 단계로 모은다.

C++ `/** ... */`만 바뀐 경우 공식 훅이 파일 hash와 AST를 갱신하지만 Graphify 0.9.36은 주석 내용을 의미 노드로 추출하지 않는다. 여러 타입에 걸친 중요한 의도는 `docs/wiki/`에도 반영한다.

### 3. switch/checkout으로 브랜치나 커밋을 전환할 때

`git switch <branch>`, `git switch --detach <commit>`, `git checkout <branch|commit>`으로 현재 작업 트리가 다른 Git snapshot을 체크아웃하는 시점에는 설치된 공식 `post-checkout` 훅을 갱신 안전망으로 사용한다. 이전 브랜치의 코드 관계가 새 snapshot 질의에 섞이지 않게 하는 시점이다. 여기서 말하는 전환은 이 checkout 계열 명령이며, 작업 트리를 크게 바꿀 수 있는 모든 Git 명령을 뜻하지 않는다.

1. 진행 중인 변경을 먼저 커밋하거나 안전하게 보관한 뒤 브랜치를 전환한다.
2. 훅은 Git의 checkout 구분 인자가 `1`일 때만 실행하며, 전환된 snapshot의 전체 코드 corpus를 백그라운드에서 다시 추출한다. 일반 커밋의 변경 파일 증분 추출보다 범위가 넓다.
3. 전환 직후 graph를 질의해야 하면 로그에서 **현재 전환 뒤 새로 추가된** 결과를 확인한다. 성공 시 `[graphify watch] Rebuilt:` 또는 변경 없음 메시지가 기록되고, 실패 시 `Rebuild failed`가 기록된다. 로그에는 branch나 commit SHA가 없으므로 과거 성공 줄을 현재 결과로 오인하지 않는다. 특히 Windows에서는 rebuild 잠금이 no-op fallback이므로 현재 재구축이 끝나기 전에 다시 전환하거나 질의하지 않는다.
4. 대상 브랜치에 `graphify-out/`이 없거나 현재 디렉터리가 Git linked worktree이면 0.9.36 훅은 실행하지 않는다. 로그에서 현재 전환 결과를 구분할 수 없거나 훅이 건너뛴 일반 코드 graph를 즉시 써야 하면 `graphify-out/`이 있는 상태에서 동기식 `python -m graphify update .`를 실행한다. 결과 디렉터리 자체가 없으면 초기 Graphify 구축 절차를 먼저 수행한다.
5. 이 단계도 문서·미디어 의미 재추출이나 wiki·Obsidian export를 완료하지 않는다. 기존 semantic 문서 노드는 보존될 수 있으므로 문서에 관한 판단은 원본과 대조한다. 훅이 만든 중간 `graphify-out/`만 stage하지 않고, 최종 생성 뷰는 push 전 wrap-up에서 동기화한다.

```powershell
$graphifyLog = Join-Path $HOME ".cache/graphify-rebuild.log"
if (Test-Path $graphifyLog) {
    Get-Item $graphifyLog | Select-Object LastWriteTimeUtc, Length
    Get-Content $graphifyLog -Tail 50
} else {
    Write-Warning "Graphify rebuild 로그가 아직 없습니다."
}
```

`git checkout -- <path>` 같은 경로 단위 복원은 checkout 구분 인자가 `0`이므로 현재 훅이 즉시 종료한다. `git restore`, `git reset`, 같은 브랜치의 pull·merge도 `post-checkout` 보장 범위가 아니며, rebase·merge·cherry-pick 진행 중에는 훅이 호출되더라도 충돌을 피하려고 건너뛴다. 이런 작업으로 코드가 달라졌다면 다음 일반 커밋의 `post-commit` 또는 최종 pre-push wrap-up을 기준으로 삼고, 그 전에 graph를 사용해야 하면 동기식 코드 갱신을 실행하거나 원본 코드와 대조한다.

### 4. 원격 push 전

pre-push는 누락을 막는 최종 게이트다. push할 커밋에 지식 원본 변경이 있으면 다음 wrap-up을 수행한다.

1. TODO의 완료 상태와 미해결 항목을 확인한다.
2. 구조적 책임이나 경계가 달라졌으면 `docs/wiki/Architecture.md`를 갱신한다.
3. Codex에서 `$graphify . --update`를 실행해 코드 구조 추출과 문서·미디어 의미 추출을 모두 반영한다.
4. 그래프 진단과 커뮤니티 이름을 확인하고 wiki, Obsidian, HTML을 다시 내보낸다.
5. 위키·코드와 생성 산출물을 검토한 뒤 함께 stage한다.
6. `python Scripts/Graphify/knowledge_guard.py stamp`로 결정론적 최신성 표식을 만든다.
7. `graphify-out/wrap-up.json`까지 stage하고, 공식 비동기 post-commit 재실행을 막기 위해 해당 wrap-up 커밋에만 `GRAPHIFY_SKIP_HOOK=1`을 설정한다.
8. 커밋 직후 수동 guard 검증을 통과시킨 뒤 push를 다시 실행한다.

훅은 위키 문서의 품질을 대신 판단하지 않는다. 대신 마지막 wrap-up 이후 코드나 위키 문서가 달라졌는지를 결정론적으로 검사하고, stale 상태라면 push를 중단한다.

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

생성 wiki와 Obsidian vault는 `update`나 공식 Git 훅이 자동으로 다시 만들지 않으므로 wrap-up에서 전용 wrapper를 명시적으로 실행한다. wrapper는 세 export를 모두 실행하고 현재 `graph.json` 및 생성 뷰의 exact hash를 `graphify-out/export-provenance.json`에 기록한다.

```powershell
python Scripts/Graphify/knowledge_guard.py export
```

## Git 훅 설치와 검증

clone마다 한 번 다음 installer를 실행한다.

```powershell
./Scripts/Graphify/Install-Hooks.ps1
./Scripts/Graphify/Install-Hooks.ps1 -Action Status
```

installer는 Graphify 공식 `post-commit`, `post-checkout`, graph merge driver를 설치한 뒤 Maverick의 동기식 `pre-push` gate를 추가한다. `post-checkout`은 지원되는 브랜치·커밋 전환 뒤 코드 graph를 비동기로 따라오게 하는 훅이지, 문서 의미와 생성 뷰의 최신성을 보증하는 gate가 아니다. 저장소 밖 shared hook 경로와 자동 통합할 수 없는 기존 `pre-push`는 덮어쓰지 않고 설치를 거부한다. 제거할 때는 `./Scripts/Graphify/Install-Hooks.ps1 -Action Uninstall`을 사용한다.

gate는 LLM이나 Graphify 생성 작업을 훅 안에서 실행하지 않는다. push할 각 ref tip의 코드·위키 Git blob, `graphify-out/` 산출물, `wrap-up.json` fingerprint가 같은 스냅샷인지 빠르게 검사한다. 누락되거나 stale이면 push를 중단하고 위 wrap-up 절차를 요구한다.

수동으로 현재 커밋을 검사할 수 있다.

```powershell
python Scripts/Graphify/knowledge_guard.py check --commit HEAD
```

stamp는 stage된 스냅샷을 기준으로 생성한다. 따라서 위키·코드와 생성 산출물에 stage되지 않은 변경이 있거나, Graphify corpus와 manifest hash가 다르거나, 문서·미디어 `semantic_hash`가 최신이 아니거나, graph와 wiki·Obsidian 구조가 맞지 않으면 생성을 거부한다. 이 성질 때문에 단순 `graphify update`만 실행하고 의미 추출·export를 생략한 상태는 정상 push 경로를 통과할 수 없다.

이 gate는 installer를 실행한 clone에서의 로컬 필수 장치이며 `git push --no-verify`로 우회할 수 있다. 팀 정책을 기술적으로 강제해야 하면 CI에서 `python Scripts/Graphify/knowledge_guard.py check --commit <검사 SHA>`를 실행하고 branch protection의 필수 check로 등록한다.

PowerShell에서 wrap-up 커밋 예시는 다음과 같다.

```powershell
$env:GRAPHIFY_SKIP_HOOK = "1"
git commit -m "문서 수정: Graphify wrap-up 갱신"
Remove-Item Env:GRAPHIFY_SKIP_HOOK
python Scripts/Graphify/knowledge_guard.py check --commit HEAD
```

## Obsidian 사용

Obsidian에서 저장소의 `graphify-out/obsidian/` 디렉터리를 vault로 연다. 사람이 작성하는 장기 문서는 `docs/wiki/`에 유지하고, Obsidian 생성 vault는 그래프 탐색용 읽기 모델로 취급한다. 둘을 한 디렉터리에 섞지 않아야 재생성 때 수동 문서가 덮어쓰이지 않는다. Graphify가 생성하는 `.obsidian/graph.json`은 공유하지만, Obsidian이 개인별로 만드는 `workspace*.json`, `app.json` 등의 UI 상태는 wrap-up 검증과 Git 추적에서 제외한다.

## 디렉터리별 문서 원칙

`Source/**/README.md` 같은 디렉터리별 목차를 일괄 생성하지 않는다. 파일 목록과 심볼 관계는 Graphify가 더 정확하게 계산하며, 수동 목차는 코드 이동 때 쉽게 낡는다.

다음 조건을 모두 만족할 때만 `docs/wiki/<Domain>.md`를 추가한다.

- 세 개 이상의 타입이나 code/asset 경계를 가로지른다.
- 책임과 호출 순서만으로는 이유나 운영 계약을 설명하기 어렵다.
- `Architecture.md` 한 문단으로는 변경 시 검토 기준을 보존할 수 없다.

AI StateTree, 사망·부활, 테이블 파이프라인처럼 이미 독립 설계 문서가 있는 도메인은 새 문서를 중복 생성하지 않고 기존 위키 문서를 갱신한다.
