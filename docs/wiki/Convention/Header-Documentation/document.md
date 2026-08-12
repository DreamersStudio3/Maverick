---
제목: C++ 헤더 주석
부제목:
최근수정일: 2026-08-11
최근수정자: 곽민규
관련문서:
  - "[[Architecture/document|Maverick Architecture]]"
  - "[[README|프로젝트 매버릭 위키]]"
---

# C++ 헤더 주석

우리 프로젝트 C++ 헤더에 달린 주석에 대한 설명문입니다.

## 의의

헤더에 잘 작성돼있는 블록은 해당 모듈의 소유권, 위임 경계, lifecycle을 선언 가까이에서 설명해주어 개발자와 에이전트 모두의 탐색 범위를 줄이므로 유익한 존재입니다. 다만 모든 헤더에 강제로 주석 블록을 달 필요는 없습니다.

## 현황

2026-08-08 기준 헤더 138개 중 71개에 있고 67개에는 없음(빈 블록도 포함돼있음)

## 주의

개발자분들은 자체적으로 판단하셔서 적당히 비슷한 문체로 추가하시면 됩니다.
**아래 내용은 Agent에 의해 생성되었으며 Agent가 참고할 내용입니다.**

---
## 필수 대상(For Agent)

- 상태를 소유하는 Component, Subsystem, Manager, Controller, Character, Actor
- 입력·포커스·타이머·delegate·이벤트 구독과 해제를 관리하는 기반 타입과 strategy
- 도메인 간 계약을 표현하는 interface와 editor/runtime 경계 도구
- Enter/Tick/Exit, Begin/End처럼 짝이 있는 생명주기와 정리 불변조건을 가진 StateTree task, NotifyState, runtime UObject
- key mapping·단위·유효성·fallback이 비자명한 public struct, table row, config

## 생략 가능한 대상(For Agent)

- 순수 enum·tag 선언
- 이름과 필드만으로 계약이 자명한 POD
- 상태 없는 얇은 wrapper와 one-shot notify
- 단일 표시 책임만 있고 생명주기·외부 계약이 없는 leaf widget
- module boilerplate와 marker type
- 파일 종류만으로 일괄 생략 금지
- 이벤트 구독 해제·입력 소비·실패 fallback 등 비자명한 계약이 있는 작은 타입도 짧은 블록 대상

## 블록 내용(For Agent)

현재 구현에 필요한 항목만 압축

1. 한 문장의 핵심 책임
2. 소유 책임과 의도적으로 소유하지 않는 경계
3. 초기화·구독·실행·정리 생명주기
4. 호출자 불변조건과 실패·fallback 의미
5. C++와 Blueprint의 책임 경계

- 공개 함수 목록과 구현 절차 반복 제외
- 미래 계획과 TODO 제외
- 동작 변경 시 기존 블록 전체 재압축
- 한국어 설명과 실제 타입·함수·이벤트 코드 심볼 유지

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
