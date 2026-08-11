---
schema_version: 1
id: wiki-graphify-query-driven-pipeline
kind: synthesis
summary: Maverick의 Graphify를 로컬 query-driven 코드 그래프로 운영하고 조사 문서를 Research 분류의 `document.md`에 보관하는 결과
tags:
- agent-dependencies
- graphify
- hive
- query-driven
aliases:
- graphify-pipeline
sources:
- raw:.hive/knowledge/Raw/docs/085d1e445b4b5413c9bd699740d511a070196f06aaf6726278e4be79bb42e810.md#sha256:085d1e445b4b5413c9bd699740d511a070196f06aaf6726278e4be79bb42e810
- raw:.hive/knowledge/Raw/document/13586a4cd90a2eb19b496e3d2e208d466ec45c0589b67ccf3287a0647d811ecb.md#sha256:13586a4cd90a2eb19b496e3d2e208d466ec45c0589b67ccf3287a0647d811ecb
- raw:.hive/knowledge/Raw/graphify-adoption-report/085d1e445b4b5413c9bd699740d511a070196f06aaf6726278e4be79bb42e810.md#sha256:085d1e445b4b5413c9bd699740d511a070196f06aaf6726278e4be79bb42e810
- raw:.hive/knowledge/Raw/graphify-adoption-report/13586a4cd90a2eb19b496e3d2e208d466ec45c0589b67ccf3287a0647d811ecb.md#sha256:13586a4cd90a2eb19b496e3d2e208d466ec45c0589b67ccf3287a0647d811ecb
- raw:.hive/knowledge/Raw/graphify-adoption-report/b70f86463d6cc12a18b2a52aab3716fe7a19b15c98b61a02b3c17993aaa1f4c0.md#sha256:b70f86463d6cc12a18b2a52aab3716fe7a19b15c98b61a02b3c17993aaa1f4c0
links:
- wiki-human-edited-wiki-push-pipeline
contradictions: []
status: active
created_at: 2026-08-10T01:31:07Z
updated_at: 2026-08-11T06:51:25Z
---

# Graphify query-driven 파이프라인

## 결과

- Aigent Hive `0.8.0`, Graphify `0.9.38` 프로젝트 버전 고정
- 첫 material 작업과 원격 push 직전 자동 의존성 준비
- 구조 질문 직전 로컬 code-only Graphify 초기화 또는 AST 증분 갱신
- post-commit·post-checkout Graphify 훅과 생성물 pre-push guard 제거
- `graphify-out/`과 `.agent-tools/` Git 제외
- 사람용 조사 문서: `docs/wiki/Research/Graphify-Adoption-Report/document.md`

## 인수 기준

- Hive 사용자 설치와 프로젝트 harness 서명 CLI 검증 통과
- 의존성 준비 스크립트 최초 실행과 무변경 재실행 통과
- Graphify wrapper를 통한 scoped graph 조회 성공
- Git index의 `graphify-out/` 추적 파일 0개

## 요청 요약

- 위키 표준 문서 파일명을 `document.md`로 변경하면서 Graphify 조사 문서 경로 동기화 요청
