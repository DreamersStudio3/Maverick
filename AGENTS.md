# AGENTS.md

## 프로젝트 개요

Maverick은 Unreal Engine 5.6 기반의 3인칭 소울라이크 프로젝트다.

프로젝트 구조와 세부 구현 규칙은 기능이 개발되는 대로 갱신한다. 현재 문서에 없는 구조나 규칙은 코드베이스의 실제 상태를 먼저 확인한 뒤 작업한다.

## 작업 전 확인

작업을 시작하기 전에 루트의 `POLICY.md`를 확인한다.

세부 정책은 `AGENTS.md`가 아니라 `POLICY.md`에 추가하거나 갱신한다. `AGENTS.md`는 프로젝트의 상위 맥락과 정책 문서로 연결하는 진입점 역할만 한다.

C++ 구현 규칙과 Unreal Engine unity build 호환성 관련 세부 정책도 `POLICY.md`를 따른다.

## 작업 진행 관리

작업을 진행할 때는 루트의 `TODO` 디렉토리에 현재 작업 목록을 정리하고, 그 내용을 기준으로 진행 상태를 관리한다. `TODO/README.md`의 생성·갱신·종료 규칙을 따르며, TODO를 장기 설계 문서나 위키 대신 사용하지 않는다.

## 디버깅과 버그 수정

유저가 디버깅이나 버그 수정을 요청하면 의심되는 런타임 경로에 추적 로그를 먼저 붙이고, 가능하면 재현 로그를 수집해 분석한 뒤 수정 방향을 정한다. 세부 로그 작성과 정리 원칙은 `POLICY.md`를 따른다.

## 코드 문서화

ProjectBA에서 사용하던 방식처럼 주요 C++ 타입의 선언 바로 앞에 책임과 라이프사이클을 요약하는 문서 블록을 둔다.

문서화 이후 변경사항이 생기면 단순히 문장을 누적하지 않는다. 기존 문서와 실제 책임을 함께 검토해 전체 내용을 다시 압축하고, 문서 길이가 무작정 늘어나지 않게 관리한다.

문서 블록의 필수 대상과 작성 기준은 `docs/wiki/Header-Documentation.md`를 따른다. 모든 헤더의 형식적 보유율을 목표로 삼지 않으며, 빈 블록과 Unreal 템플릿 주석은 문서화로 인정하지 않는다.

## 코드 책임 분리

`CharacterBase` 계열은 공통 캐릭터 상태, 컴포넌트 연결, 엔진 라이프사이클 브리지에 집중한다.

회피, 액션 버퍼, 전투, UI처럼 특정 도메인에 종속적인 정책은 해당 컴포넌트나 도메인 파일로 분리한다. 새 코드를 작성할 때는 확장성을 먼저 검토하고, 파일의 최소 책임을 넘어서는 편의성 코드를 방만하게 추가하지 않는다.

## Graphify와 내부 위키

이 프로젝트의 지식 그래프와 생성 위키는 `graphify-out/`에 둔다. 세부 행동강령과 명령은 `docs/wiki/Documentation-Workflow.md`를 따른다.

- 프로젝트 전체 구조나 책임 경계를 파악할 때는 `docs/wiki/Architecture.md`를 먼저 읽는다.
- 코드베이스 질문을 받으면 `graphify-out/graph.json`이 있는지 확인하고, 원본 파일을 넓게 읽기 전에 `graphify query "<질문>"`으로 세부 관계를 좁힌다. 관계 추적은 `graphify path`, 단일 개념 조사는 `graphify explain`을 사용한다.
- 질의 결과가 부족할 때 `graphify-out/wiki/index.md`, `graphify-out/GRAPH_REPORT.md`, 관련 원본 코드 순으로 확인한다.
- `graphify-out/`은 생성 산출물이므로 직접 편집하지 않는다. 코드와 위키 문서를 수정한 뒤 Graphify로 다시 생성한다.
- 각 커밋 뒤 공식 Graphify `post-commit` 훅의 저비용 AST 갱신을 안전망으로 사용하되, 이 중간 생성물만 따로 커밋하지 않는다.
- `git switch`나 `git checkout <branch|commit>`으로 브랜치 또는 커밋을 전환하면 공식 `post-checkout` 훅의 백그라운드 코드 그래프 재구축을 확인한 뒤 질의한다. 훅이 실행되지 않는 조건과 수동 갱신법은 `docs/wiki/Documentation-Workflow.md`를 따른다. 전환 뒤 생긴 중간 `graphify-out/` 변경도 따로 커밋하지 않는다.
- 원격 push 전에는 문서 의미 추출과 생성 뷰를 포함한 Graphify wrap-up을 커밋하고 pre-push 최신성 검증을 반드시 통과한다.
- `graphify` 실행 파일이 PATH에 없고 Python 패키지만 설치된 환경에서는 같은 명령을 `python -m graphify`로 실행한다.
