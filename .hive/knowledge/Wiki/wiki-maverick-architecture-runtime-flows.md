---
schema_version: 1
id: wiki-maverick-architecture-runtime-flows
kind: synthesis
summary: Maverick Architecture 진입 문서와 첨부 자료는 Architecture 루트에, 핵심 런타임 흐름 문서 7개는 Features 직속 폴더에 배치하고 관련 문서 관계를 직접 백링크로 관리하는 위키 구조
tags:
- architecture
- documentation
- runtime
- wiki
aliases:
- architecture-runtime-flows
sources:
- raw:.hive/knowledge/Raw/document/9a0735a8993ac8ff70b826c9002f80f0e6bb8b4ad96e5ad5781c04d7b392699e.md#sha256:9a0735a8993ac8ff70b826c9002f80f0e6bb8b4ad96e5ad5781c04d7b392699e
- raw:.hive/knowledge/Raw/document/cce2dc8e121db34266b8d9492034cb3eb9be6e39e6ecb061d250ce4bb1c3634c.md#sha256:cce2dc8e121db34266b8d9492034cb3eb9be6e39e6ecb061d250ce4bb1c3634c
- raw:.hive/knowledge/Raw/document/d885c570340bcbb125a653307d87c18dbe2712f6f6e8dc1e17be0346c5aa9a29.md#sha256:d885c570340bcbb125a653307d87c18dbe2712f6f6e8dc1e17be0346c5aa9a29
- raw:.hive/knowledge/Raw/document/dadd81ce0f8d7863d41003ebf008f2be3970921f6a822c9b0c086ee461a67159.md#sha256:dadd81ce0f8d7863d41003ebf008f2be3970921f6a822c9b0c086ee461a67159
links:
- wiki-human-edited-wiki-push-pipeline
contradictions: []
status: active
created_at: 2026-08-11T07:13:48Z
updated_at: 2026-08-11T07:33:07Z
---

# Maverick Architecture 문서 구조

## 현재 구조

- 진입 문서: `docs/wiki/Architecture/document.md`
- 진입 문서 자료: `docs/wiki/Architecture/attachments/`
- 전투 기능 묶음: `docs/wiki/Features/Combat/`
- 입력·Action: `docs/wiki/Features/Input-to-Action/`
- 적중·수치·피격: `docs/wiki/Features/Hit-Stat-HitReaction/`
- 사망·필드 전환: `docs/wiki/Features/Death-and-Field-Transition/`
- AI: `docs/wiki/Features/AI-StateTree/`
- UI: `docs/wiki/Features/UI-and-CommonUI/`
- 테이블: `docs/wiki/Features/Table-Data/`
- LockOnTarget: `docs/wiki/Features/LockOnTarget-Boundary/`
- 각 세부 문서: 해당 폴더의 `document.md`
- 각 세부 문서 자료: 같은 문서 폴더의 `attachments/`

## 문서 관계

- Architecture 본문의 `## 관련 문서` 목록 없음
- Architecture의 핵심 런타임 흐름: `Features/<Flow>/document.md` 7개 링크
- 위키 내부 관련 문서: 각 문서 frontmatter에서 `[[Architecture/document|Maverick Architecture]]` 직접 참조
- 위키 보관함 밖 관련 README: Architecture 문서를 향하는 상대 Markdown 링크 사용
- Obsidian 백링크 패널: `docs/wiki/` 보관함 내부 문서만 자동 색인

## 이번 결과

- 프로젝트: Maverick
- 도구: Graphify, 프로젝트 Markdown 위키, Obsidian
- 결과: Runtime-Flows 아래의 핵심 런타임 흐름 7개 문서 폴더를 Features 루트로 이동하고 빈 중간 폴더 제거
- 수용 기준: 이동 전후 문서 해시 동일, Architecture 새 링크 7개, 이전 실사용 경로 제거
- 검증 결과: 구조·해시·링크·Git 공백 검사 통과

## 요청 요약

- Features의 Runtime-Flows 중간 폴더를 없애고 하위 문서 폴더를 한 단계 밖으로 이동하는 요청
