# C++ 헤더 책임 문서화

Maverick의 `/** ... */` 블록은 JavaScript JSDoc과 모양이 같지만 C++에서는 Doxygen 형식 문서 주석으로 부른다. 이 문서는 보유율이 아니라 실제 이해 비용을 줄이는 기준을 정의한다.

## 결론

현재 방식은 유지할 가치가 크다. 좋은 블록은 타입의 소유권, 위임 경계, lifecycle을 선언 가까이에서 설명해 사람과 에이전트 모두의 탐색 범위를 줄인다. 다만 모든 헤더에 형식적으로 넣는 방식은 빈 블록, 중복, stale 문장을 늘리므로 사용하지 않는다.

2026-08-08 단순 문자열 감사 기준으로 프로젝트 자체 헤더 138개 중 71개에 `/**`가 있고 67개에는 없다. 이 수치는 빈 블록 5개와 보조 타입에만 붙은 블록도 포함하므로 문서 품질이나 주요 타입 coverage를 의미하지 않는다.

## 필수 대상

- 상태를 소유하는 Component, Subsystem, Manager, Controller, Character/Actor.
- 입력, 포커스, 타이머, delegate, 이벤트 구독과 해제를 관리하는 기반 타입과 strategy.
- 도메인 간 계약을 노출하는 interface와 editor/runtime 경계 도구.
- Enter/Tick/Exit, Begin/End처럼 짝이 있는 lifecycle과 정리 불변조건을 가진 StateTree task, NotifyState, runtime UObject.
- key mapping, 단위, 유효성, fallback이 비자명한 public struct, table row, config.

## 생략 가능한 대상

- 순수 enum/tag 선언.
- 이름과 필드만으로 계약이 자명한 POD.
- 상태가 없는 얇은 wrapper나 one-shot notify.
- 단일 표시 책임만 있고 lifecycle이나 외부 계약이 없는 leaf widget.
- module boilerplate와 marker type.

파일 종류만으로 일괄 결정하지 않는다. 작은 타입이라도 이벤트 구독 해제, 입력 소비, 실패 fallback 같은 비자명한 계약이 있으면 짧은 블록이 필요하다.

## 블록 내용

필요한 항목만 사용해 현재 구현을 압축한다.

1. 한 문장의 핵심 책임.
2. 소유하는 책임과 의도적으로 소유하지 않는 경계.
3. 초기화, 구독, 실행, 정리의 lifecycle.
4. 호출자가 지켜야 할 불변조건과 실패/fallback 의미.
5. Blueprint나 binary asset 검증이 필요한 경계.

공개 함수 목록이나 구현 절차를 그대로 반복하지 않는다. 미래 계획과 TODO는 헤더 계약에 넣지 않는다. 동작이 바뀌면 문장을 덧붙이기보다 기존 블록 전체를 다시 압축한다. 설명은 한국어를 기본으로 하고 실제 타입·함수·이벤트 이름은 코드 심볼을 유지한다.

## 좋은 기준점과 우선 보완 대상

현재 좋은 기준점은 다음과 같다.

- `Source/Maverick/Character/MVCharacterBase.h`: 공통 책임, 위임 경계, BeginPlay/Input/Tick 흐름.
- `Source/Maverick/Combat/MVHitResolverSubsystem.h`: 입력 경계, 맨손 fallback, 결과 전달 순서.
- `Source/Maverick/Public/Interface/MVActionInputHandlerInterface.h`: 중앙 라우터와 구현자의 소비 계약.
- `Source/Maverick/UI/HUD/MVStatusBarWidget.h`: Blueprint 소유 영역과 C++ fallback.

우선 보완 가치가 큰 타입은 복잡도에 비해 주 선언 문서가 부족한 `UMVUISubsystem`, `UMVTableManager`, `UMVTableAssetGenerator`, `UMVFinisherComponent`, `FMVUIFadeController`, `AMVAIController`다. 이번 파이프라인 도입은 기준 확정까지이며, 해당 헤더 전체를 일괄 수정하는 작업은 별도 범위로 둔다.

## 현재 발견된 안티패턴

- 내용 없는 `/** */` placeholder와 Unreal 템플릿 주석.
- 보조 struct에만 문서가 있고 파일의 주 클래스에는 책임 문서가 없는 경우.
- 구현되지 않은 미래 priority나 TODO를 현재 계약과 섞는 경우.
- 구현이 완료된 뒤에도 임시 cue, 임시 bridge라고 남아 있는 stale 설명.
- 같은 책임을 여러 bullet에서 반복해 블록이 계속 길어지는 경우.

## Graphify와의 관계

로컬 `graphifyy 0.9.36`의 C++ extractor는 class/function/include/call 구조를 추출하지만 C++ Doxygen 내용을 의미 노드로 만들지 않는다. 따라서 헤더 블록을 삭제해도 Graphify 구조 그래프는 거의 같을 수 있지만, 사람의 코드 이해 품질은 낮아진다.

여러 타입에 걸친 설계 근거와 흐름은 Graphify가 의미 추출할 수 있는 `docs/wiki/`에 기록한다. 헤더에는 해당 타입의 로컬 계약만 둔다. 이 구분은 같은 내용을 디렉터리별 Markdown과 헤더 양쪽에 복제하지 않게 한다.

## 디렉터리별 Markdown 판단

각 소스 디렉터리에 `README.md`를 자동으로 만들지 않는다. 도메인 문서를 새로 만드는 상세 조건과 기존 설계 문서의 재사용 기준은 단일 운영 기준인 `docs/wiki/Documentation-Workflow.md`의 "디렉터리별 문서 원칙"을 따른다.
