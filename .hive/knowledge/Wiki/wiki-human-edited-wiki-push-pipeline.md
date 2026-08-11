---
schema_version: 1
id: wiki-human-edited-wiki-push-pipeline
kind: synthesis
summary: document.md와 문서별 attachments 구조를 사용하고 Templater 자동 frontmatter와 개발자 Obsidian 편집 후 별도 위키 커밋으로 반영하는 Maverick 문서 파이프라인
tags:
- documentation
- obsidian
- pull-request
- wiki
aliases:
- wiki-draft-push
sources:
- raw:.hive/knowledge/Raw/document/cce2dc8e121db34266b8d9492034cb3eb9be6e39e6ecb061d250ce4bb1c3634c.md#sha256:cce2dc8e121db34266b8d9492034cb3eb9be6e39e6ecb061d250ce4bb1c3634c
- raw:.hive/knowledge/Raw/document/dadd81ce0f8d7863d41003ebf008f2be3970921f6a822c9b0c086ee461a67159.md#sha256:dadd81ce0f8d7863d41003ebf008f2be3970921f6a822c9b0c086ee461a67159
- raw:.hive/knowledge/Raw/documentation-workflow/0dbb2cd448eaad52c8f844c39e06626112f73c48aa684148970d57d9940f349f.md#sha256:0dbb2cd448eaad52c8f844c39e06626112f73c48aa684148970d57d9940f349f
- raw:.hive/knowledge/Raw/readme/01927c821658ab4ce96f0779be7dc7c72c60b62a642c47a7fa11cca2d1d1f348.md#sha256:01927c821658ab4ce96f0779be7dc7c72c60b62a642c47a7fa11cca2d1d1f348
- raw:.hive/knowledge/Raw/readme/5d802e0670a4171b2227cb4b8a433b308b35e84fef5e39ba040462c122694667.md#sha256:5d802e0670a4171b2227cb4b8a433b308b35e84fef5e39ba040462c122694667
- raw:.hive/knowledge/Raw/readme/9169d5c4c0b9426451f025b924ccea2184f3a8d98307b902b460472d8b8cf29a.md#sha256:9169d5c4c0b9426451f025b924ccea2184f3a8d98307b902b460472d8b8cf29a
- raw:.hive/knowledge/Raw/readme/ebc32c372c62c41c1bb60266be96c03acdf9cecddd4bca7fff8b6dcb1fc483c8.md#sha256:ebc32c372c62c41c1bb60266be96c03acdf9cecddd4bca7fff8b6dcb1fc483c8
- raw:.hive/knowledge/Raw/wiki-principles/8a38af883ac3f39377d0f6dda0521d0ff0dde549d6189ae8bdae28aca7367eb0.md#sha256:8a38af883ac3f39377d0f6dda0521d0ff0dde549d6189ae8bdae28aca7367eb0
links:
- wiki-developer-interview-wiki-draft-skill
- wiki-graphify-query-driven-pipeline
- wiki-maverick-architecture-runtime-flows
contradictions: []
status: active
created_at: 2026-08-10T01:41:43Z
updated_at: 2026-08-11T07:33:07Z
---

# 사람 편집 기반 위키 push 파이프라인

## 현재 구조

- 루트 `docs/wiki/README.md`: 분류와 작성 기준
- `Architecture/document.md`: 전체 구조와 책임 경계의 진입 문서
- `Architecture/attachments/`: Architecture 진입 문서 자료
- `Convention/`: 코드·문서 작성 규칙
- `Features/Combat/`: 전투 기능 문서
- `Features/<Flow>/`: 핵심 런타임 흐름별 직속 문서 폴더
- `Research/`: 시장·설계·도구 조사
- 개별 문서: `<분류>/<문서명>/document.md`
- 개별 자료: 같은 문서 폴더의 `attachments/`
- 수동 생성 양식: `template/document.md`

## 문서 계약

- frontmatter 필수 항목: 제목, 부제목, 최근수정일, 최근수정자, 관련문서
- 사람이 직접 생성한 빈 Markdown: Templater 폴더 양식 적용
- 생성 문답: 제목, 부제목, 최근 수정자
- 공유 설정: `.obsidian/plugins/templater-obsidian/data.json`
- Templater 자동 실행: 보안상 장치별 활성화 대상
- 새 위키 문서: 개발자 문답과 경로 승인을 거친 위키 초안 Skill 또는 수동 Templater 생성
- 빈 첨부 폴더의 Git 공유를 위한 `attachments/.gitkeep`
- 첨부 파일의 영문 설명형 이름과 상대 Markdown 링크
- 개발자의 Obsidian 수정·첨삭 완료 전달을 문서 최종화 신호로 사용
- 현재 위키 변경을 별도 `문서 수정` 커밋으로 기록

## 이번 결과

- 프로젝트: Maverick
- 도구: Templater `2.25.0`, `draft-wiki-with-developer`, Obsidian
- 결과: 핵심 런타임 흐름 7개 폴더를 Features 루트로 이동하고 Runtime-Flows 중간 폴더 제거
- 수용 기준: 문서·attachments 보존, Architecture 링크 동기화, 이전 Runtime-Flows 경로 제거
- 미검증: 현재 장치의 `Trigger Templater on new file creation` 보안 스위치 활성화와 실제 생성 문답

## 요청 요약

- Features의 Runtime-Flows 중간 폴더 제거 요청
