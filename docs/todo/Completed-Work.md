# 완료 작업 압축 기록

2026-08-08에 루트 `TODO/`의 완료 기록 77개를 이 문서로 통합했다. 이 문서는 과거 작업의 탐색용 요약이며 현재 동작의 기준은 코드와 `docs/wiki/`다. 세부 변경은 Git 이력에서 확인한다.

## 남겨 둔 결과

- **입력과 액션:** GameplayTag 입력을 `UMVInputManagerComponent`에서 버퍼링하고 Dodge·Combat 같은 도메인 처리기로 전달하는 구조, recovery window 전환, hold/instant 입력, 오래된 montage 종료 이벤트 방지가 정리됐다. 현재 흐름은 [Maverick Architecture](../wiki/Architecture/document.md)의 입력·Action 절을 따른다.
- **전투와 피격:** 공격 Ability, 무기 배율 기반 hit resolve, hit reaction·groggy·airborne·recovery 정책과 관련 회귀 수정이 반영됐다. 현재 구현 경계는 [Maverick Architecture](../wiki/Architecture/document.md), 목표 전투 계약은 [Maverick 전투 MDA와 목표 전투 계약](../wiki/Research/Combat-Design-MDA/document.md)에 모았다.
- **전투 v0.3와 시장 조사:** 약/차지 Tap·Hold, S1 Tempo·S2 Area·S3 Response·S4 Signature, Perfect Dodge 수직 검증안의 전투 위키 통합. S1 상세 구현과 2026 소울라이크·보스러시 조사의 `docs/wiki/` 별도 문서 이관.
- **AI:** StateTree의 sensing·focus·공격 기회·cooldown·dodge token·피격 회복·사망 상태와 필드 전환 reset 흐름을 구축했다. StateTree와 Blueprint 연결은 바이너리 에셋이므로 코드만 보고 확정하지 않는다.
- **사망과 부활:** `UMVDeathComponent`가 사망 표현을, UI overlay와 loading gate가 화면 전환을, `UMVFieldTransitionSubsystem`이 reset을 맡도록 책임을 나눴다. dissolve, 입력 차단, HUD 정리, 적 reset 결과도 [Maverick Architecture](../wiki/Architecture/document.md)에 흡수했다.
- **플레이어·무기·소모품:** Dodge·상호작용·lock-on·소모품을 플레이어 소유 하위 모듈로 정리하고, `UMVWeaponComponent`의 장착·소켓 검증·양손 mesh 흐름과 DataTable/manifest 연결을 보완했다.
- **UI와 조작:** CommonUI 창·popup 소유권, 상호작용 후보, stamina/boss damage/skill cooldown HUD, 게임패드와 키보드 입력 구성을 정리했다. 실제 IMC·WBP 연결은 Unreal Editor 에셋에서 확인한다.
- **문서 운영:** 사람용 내부 위키는 `docs/wiki/`, 에이전트용 Graphify 출력은 로컬 `graphify-out/` 캐시로 분리했다. 에이전트의 근거 기반 초안, 개발자의 Obsidian 편집 완료 전달, 별도 위키 커밋, PR의 커밋 고정 문서 링크 흐름을 사용하며 별도 승인·초안 비교 절차는 두지 않는다. 현재 절차는 [프로젝트 매버릭 위키 README](../wiki/README.md) 기준이다.

## 별도 조사 결론

- 기후위기 세계관은 WMO·UNEP·IPCC·한국 공식 전망을 기준으로 관측 사실, 모델 전망, 창작적 추론을 구분한다. 단일 시점의 전면 붕괴보다 지역·계층별 충격과 복합 시스템 압력을 사용한다.
- PvPvE는 독립된 공신력 시장 규모가 없어 인접 장르와 작품별 성과를 함께 봐야 한다. 8인 전면 전환보다 같은 전투 코어의 PvE 협동과 PvPvE 프로토타입을 먼저 비교하고, 매치 인구·서버·운영비를 통과할 때만 확장한다.

옛 문서의 미완료 체크박스, 임시 로그 계획, 브랜치·merge 절차, 에셋 잠금 메모는 완료된 작업의 현재 지식이 아니므로 승계하지 않았다.
