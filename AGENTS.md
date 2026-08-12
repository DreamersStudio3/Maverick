# AGENTS.md

## 프로젝트 개요

Maverick: Unreal Engine 5.8 기반 3인칭 소울라이크 프로젝트

프로젝트 구조와 세부 구현 규칙은 기능이 개발되는 대로 갱신한다. 현재 문서에 없는 구조나 규칙은 코드베이스의 실제 상태를 먼저 확인한 뒤 작업한다.

## 작업 전 확인

작업을 시작하기 전에 루트의 `POLICY.md`를 확인한다.

세부 정책은 `AGENTS.md`가 아니라 `POLICY.md`에 추가하거나 갱신한다. `AGENTS.md`는 프로젝트의 상위 맥락과 정책 문서로 연결하는 진입점 역할만 한다.

C++ 구현 규칙과 Unreal Engine unity build 호환성 관련 세부 정책도 `POLICY.md`를 따른다.

## 작업 진행 관리

작업을 진행할 때는 `docs/todo/`에 현재 작업 목록을 짧게 정리하고, 그 내용을 기준으로 진행 상태를 관리한다. `docs/todo/README.md`의 생성·갱신·종료 규칙을 따르며, 완료한 작업 파일을 장기 설계 문서나 이력 저장소처럼 쌓지 않는다.

## 디버깅과 버그 수정

유저가 디버깅이나 버그 수정을 요청하면 의심되는 런타임 경로에 추적 로그를 먼저 붙이고, 가능하면 재현 로그를 수집해 분석한 뒤 수정 방향을 정한다. 세부 로그 작성과 정리 원칙은 `POLICY.md`를 따른다.

## 코드 문서화

ProjectBA에서 사용하던 방식처럼 주요 C++ 타입의 선언 바로 앞에 책임과 라이프사이클을 요약하는 문서 블록을 둔다.

문서화 이후 변경사항이 생기면 단순히 문장을 누적하지 않는다. 기존 문서와 실제 책임을 함께 검토해 전체 내용을 다시 압축하고, 문서 길이가 무작정 늘어나지 않게 관리한다.

문서 블록의 필수 대상과 작성 기준은 `docs/wiki/Convention/Header-Documentation/document.md`를 따른다. 모든 헤더의 형식적 보유율을 목표로 삼지 않으며, 빈 블록과 Unreal 템플릿 주석은 문서화로 인정하지 않는다.

## 코드 책임 분리

`CharacterBase` 계열은 공통 캐릭터 상태, 컴포넌트 연결, 엔진 라이프사이클 브리지에 집중한다.

회피, 액션 버퍼, 전투, UI처럼 특정 도메인에 종속적인 정책은 해당 컴포넌트나 도메인 파일로 분리한다. 새 코드를 작성할 때는 확장성을 먼저 검토하고, 파일의 최소 책임을 넘어서는 편의성 코드를 방만하게 추가하지 않는다.

## Agent 의존성

프로젝트 고정 의존성: Aigent Hive `0.8.0`, Graphify `0.9.38`. 버전 기준: `Config/AgentDependencies.json`.

- 단순 질문을 제외한 첫 작업 전에 `./Scripts/Agent/Ensure-AgentDependencies.ps1 -HostName <codex|claude|antigravity>` 실행
- 원격 push 직전 동일 명령 재실행
- 누락 의존성 자동 설치 허용 범위: Hive 공식 npm 패키지, 프로젝트 로컬 Graphify 가상환경, 사용자 범위 Graphify agent skill
- `.hive/config/harness.toml` 누락 또는 설치·검증 실패 시 작업·push 중단과 실패 원인 보고
- `.agent-tools/`, `graphify-out/`: 로컬 생성 상태, Git 추가 금지

## 내부 위키와 Graphify

사람용 내부 위키: `docs/wiki/`. 에이전트용 Graphify 코드 그래프: 로컬 `graphify-out/`. 문서 분류·frontmatter·첨부 구조 기준: `docs/wiki/README.md`.

- 활성 지식 체계: 구조·관계 탐색용 Graphify와 설계 의도·운영 지식 정본인 `docs/wiki/`
- Hive Knowledge: 자동 포착·질의 비활성, 기존 Raw·Wiki 기록 삭제, 신규 생성 금지

- 전체 구조와 책임 경계 조사 시작점: `docs/wiki/Architecture/document.md`
- 구조·호출·관계 질문: 원본 파일의 광범위 탐색 전에 `./Scripts/Graphify/Invoke-Graphify.ps1 query "<질문>"` 실행
- 관계 경로: `./Scripts/Graphify/Invoke-Graphify.ps1 path "<시작 심볼>" "<도착 심볼>"`
- 단일 개념: `./Scripts/Graphify/Invoke-Graphify.ps1 explain "<심볼>"`
- 질의 직전 스크립트가 로컬 그래프 초기화 또는 코드 AST 증분 갱신 수행
- 일반 커밋·브랜치 전환·push에 따른 Graphify 자동 갱신 없음
- Graphify 결과 부족 또는 stale 의심 시 관련 원본 코드 확인, Unreal 에셋 연결은 Editor 검증
- 구현과 검증 완료 후 원격 반영 전에 에이전트가 실제 변경 근거를 바탕으로 `docs/wiki/` 문서 초안 생성 또는 갱신
- 새 문서: `<분류>/<문서명>/document.md`, 문서별 자료: 같은 폴더의 `attachments/`
- 문서 frontmatter: 제목·부제목·최근 수정·관련 문서 유지
- 사람의 수동 문서 생성: `template/document.md`와 Templater 폴더 템플릿 사용, 장치별 자동 실행 활성화 필요
- 개발자가 Obsidian 편집 완료를 알리면 승인 표식·초안 비교·내용 해시 확인 없이 현재 위키를 최종본으로 취급
- 사용자의 push 요청 또는 위키 편집 완료 전달 후 위키 변경을 별도 `문서 수정` 커밋으로 남기고 관련 코드 커밋과 함께 push
- PR 설명의 작업 세부사항에 위키 커밋과 해당 커밋에서 변경한 `docs/wiki/` 문서 링크 추가
- 위키 변경 불필요 작업은 TODO와 PR에 근거 기록, 위키 커밋 생략

<!-- AIGENT-HIVE:START -->
# Aigent Hive

Project: `Maverick`
Profile: `general`
Setup mode: `custom`
Preference provenance: `project-custom`
Interface language: `ko`
Wiki: `disabled` (`ko`)
Persona: `strict`
Primary host: `codex`
Resolved orchestration owner: `host-native`
Resolution evidence: `sha256:70b8da8772791c175efaaa867a88143f840cb5135521bd6ee59eb57befe1c4fb`

- Read canonical Hive configuration from `.hive/config/harness.toml`.
- Before editing anything, read `.hive/directives/00-editing-discipline.md` in full. Apply all four sections as the highest-priority editing discipline within the Hive contract; never compact, summarize, omit, or substitute any part. Its literal `# CLAUDE.md` heading is original text, not Claude-only scope: the directive applies identically on Codex, Claude, and Gemini Antigravity. Higher-priority instructions and Hive security, ownership, credential, and production boundaries still control.
- Usage guard: disabled by installed preference. Do not run `hive usage enforce` or call a native/CodexBar sensor automatically. Automatic resume must report `data.usage_guard.enforced=false`, `outcome=disabled`, one authorization ID, and exactly one dispatch brief.
- Load only the directives and knowledge required by the current request.
- For a simple question, do not load project memory, spawn agents, or edit files.
- Route explicit prompt authoring or improvement intent to `hive-prompt-refine` in `refine-only` mode unless the same request explicitly asks to execute the result.
- If an ordinary work prompt materially lacks a goal, scope, constraints, acceptance criteria, or output contract, offer one concise optional refinement suggestion without rewriting the prompt, loading the Skill, or executing the suggestion. Do not interrupt sufficiently clear ordinary work or a simple question.
- Before presenting pending actions or a user handoff, complete every safe, in-scope, automatable action that does not require new user authority, credentials, a protected external mutation, or a materially different product decision. Then give only the genuinely user-owned actions as a concise ordered guide with each exact location, command or operation, expected result or return evidence, and reason user authority is required. List failed or impossible work separately with its cause and recovery path.
- Keep durable role identity in `.hive/team/roles/`; the active host owns sessions and subagents.
- Keep durable knowledge in Markdown. Treat `.hive/index/*.sqlite*` as disposable.
- When this marker reports Wiki enabled, run agent-reviewed task-fact autocapture before the final response for material work. Record the bounded outcome, tool or project, criteria, and originating request summary from current authorized artifacts; never ingest a raw transcript, hook payload, tool output, hidden prompt, or runtime state.
- Keep the selected interface language consistent throughout every question and response. In Korean, keep English only for proper nouns, product or package names, commands, code identifiers, paths, schema keys, exact UI labels, and terms without a clear Korean equivalent; replace ordinary English nouns with Korean. In English, write the full passage in English except for exact Korean names, literals, quotations, or text the user explicitly asks to preserve.
- Write human-readable project documents in concise Korean unless the user explicitly requests another language. Prefer short headings, bullets, tables, checklists, and semantic noun phrases.
- Do not end authored explanatory Korean prose with declarative or conversational forms. `~다`, `~한다`, `~된다`, `~이다`, `~있다`, `~없다`, `~않는다`, `~했다`, `~됐다`, `~합니다`, `~됩니다`, and `~해요` are non-exhaustive prohibited examples.
- Do not mechanically change those endings to `~음` or the attached `~ㅁ` form. This includes Korean stems, mixed English-Korean forms, state labels followed by a copula, and possibility clauses. Rewrite the full clause: avoid `Release 계약이 구현됐다.` and `Release 계약이 구현됐음.`; use `Release 계약 구현 완료`. Avoid `API key를 요청하거나 저장하지 않는다.` and `API key를 요청하거나 저장하지 않음.`; use `API key 요청·저장 없음`.
- Exact bad → good examples (not exhaustive): `Aigent Hive는 provider-neutral 로컬 agent harness다.` → `Aigent Hive: provider-neutral 로컬 agent harness`; `Product version은 0.7.0이다.` → `Product version: 0.7.0`; `Release 계약이 구현됐다.` → `Release 계약 구현 완료`; `API key를 요청하거나 저장하지 않는다.` → `API key 요청·저장 없음`; `이 기능을 사용합니다.` → `기능 사용`; `다음 단계에서 검증해요.` → `다음 단계: 검증`; `검증이 필요합니다.` → `검증 필요`; `업데이트가 완료되었습니다.` → `업데이트 완료`; `Release 계약이 구현됐음.` → `Release 계약 구현 완료`; `API key를 요청하거나 저장하지 않음.` → `API key 요청·저장 없음`.
- Mechanical nounization examples (not exhaustive): `Status는 INDETERMINATE다.` → `Status: INDETERMINATE`; `문서를 읽음.` → `문서 확인`; `작업이 끝남.` → `작업 완료`; `연결이 닫힘.` → `연결 종료`; `설정 값을 가짐.` → `설정 값 보유`; `정책을 따름.` → `정책 준수`; `compile됨.` → `compile 완료`; `검증할 수 있음.` → `검증 가능`; `검증할 수 없음.` → `검증 불가`.
- Do not use conversational imperative endings such as standalone `~줘` or attached `~해` in authored explanation. Exact user-prompt or UI-prompt samples require the path, line, reason, and exact line digest exception. Examples: `문서를 보여 줘.` → `문서 확인 요청`; `기능을 사용해.` → `기능 사용 요청`.
- Never gain brevity by removing a qualifier needed to interpret a result. For every passed, failed, skipped, deferred, unverified, or unsupported item, name the affected scope, exact reason, relationship to the current host or platform, whether it actually ran, and what the result does and does not prove. Do not use a platform adjective such as "Windows-only" or "Unix-only" without stating whether the current platform ran or skipped that item and why.
- Apply the same rule to authored callouts and blockquotes. Blockquote syntax is not proof of an exact quotation. Preserve narrative-form text only for an exact external quote, UI prompt, protocol sample, fixture payload, or another byte-sensitive literal with an explicit path, line, reason, and exact line digest.
- Do not call model-provider APIs or request provider API credentials.
- Require explicit approval before activating optional Skills.
- Resolve compatible OMX on Codex and compatible OMC on Claude before host-native capability; never ask the user to select an owner or switch owners mid-run.
- Treat OMX/OMC cancellation output as auxiliary evidence only; it never substitutes for the bound usage halt marker or durable goal/task state.
- Treat fallback hooks as optional data-integrity guards only. They require conclusive external capability absence plus exact capability, event, path, command, and digest consent.
- Never use a fallback hook for prompt classification or rewriting, Skill activation, memory ingestion, subagent orchestration, or continuation. A `Stop` hook always returns a neutral allow result.
- Preserve user text and third-party marker blocks outside this Hive block.
<!-- AIGENT-HIVE:END -->
