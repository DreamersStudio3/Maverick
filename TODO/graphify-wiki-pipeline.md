# Graphify 내부 위키 파이프라인 도입

## 목표

- Maverick의 코드와 정본 문서를 Graphify 지식 그래프, 생성 위키, Obsidian vault로 초기화한다.
- 작업 시작, 커밋, pre-push의 역할을 분리하고 원격에 stale 문서가 올라가지 않게 한다.
- 에이전트가 전체 코드를 읽기 전에 사용할 Architecture 문서와 query-first 행동강령을 만든다.
- 기존 TODO와 C++ 헤더 문서화의 역할을 Graphify와 충돌하지 않게 정리한다.

## 시작 상태

- [x] 루트 `POLICY.md` 확인
- [x] 기존 워킹트리의 사용자 소유 미추적 파일 확인 및 제외
- [x] 전역 Graphify 스킬 지침과 hook/update/export 참고 문서 확인
- [x] 설치된 Graphify CLI 버전과 공식 저장소 문서 확인
- [x] 기존 TODO 형식과 헤더 문서화 현황 조사

## 작업 단위

- [x] Graphify 운영 정책과 TODO 역할 정리
- [ ] 전체 코퍼스 초기 그래프, wiki, Obsidian vault 생성
- [ ] `Architecture.md`와 헤더 문서화 기준 작성
- [ ] 커밋 갱신 및 pre-push 최신성 검증 구현·설치
- [ ] 최종 Graphify wrap-up, 검증, 작업 보고서 작성

## 결정

- 프롬프트 수신 시점은 TODO 작성과 기존 그래프 조회에 사용하고 Graphify를 재생성하지 않는다.
- 일상 갱신은 커밋 전 수행한다. 코드만 바뀌면 AST 증분 갱신을 사용하고, 문서가 바뀌면 Graphify 스킬의 의미 추출을 포함한다.
- pre-push는 전체 wrap-up의 필수 게이트로 사용한다.
- `TODO/`는 단기 실행 메모이고 Graphify 입력에서 제외한다.
- `docs/wiki/`는 사람이 관리하는 정본, `graphify-out/`은 재생성 가능한 읽기 모델로 분리한다.
- 현재 Graphify 0.9.36과 공식 문서는 프로젝트 산출물에 `.graphify/`가 아니라 `graphify-out/`을 사용한다.

## 검증 기록

- 프로젝트 자체 헤더 138개 중 현재 `/** ... */` 블록이 있는 파일은 71개, 없는 파일은 67개다. 존재 여부만 센 값이며 품질 평가는 Architecture 단계에서 수행한다.
- `graphify` 실행 파일은 현재 PATH에 없지만 Python 3.14의 `graphifyy 0.9.36` 패키지는 설치되어 있어 `python -m graphify`로 실행할 수 있다.
