---
schema_version: 1
id: wiki-developer-interview-wiki-draft-skill
kind: open-question
summary: 개발자 문답으로 압축 초안을 만들고 승인된 영문 경로의 `document.md`에 frontmatter와 attachments 구조를 생성한 뒤 Obsidian에서 여는 위키 작성 Skill
tags:
- documentation
- skill
- wiki
aliases:
- draft-wiki-with-developer
sources:
- raw:.hive/knowledge/Raw/skill/10f73ba2420bacb6bb538b12890a365854b109d2a467b067d2008f60d5f135fe.md#sha256:10f73ba2420bacb6bb538b12890a365854b109d2a467b067d2008f60d5f135fe
- raw:.hive/knowledge/Raw/skill/27d2fce986a3dcc0cc24b9bb1a67c44dd867ff3f3b3466719fad581c6287d64f.md#sha256:27d2fce986a3dcc0cc24b9bb1a67c44dd867ff3f3b3466719fad581c6287d64f
- raw:.hive/knowledge/Raw/skill/5933276d8b1cc3e5db9d0c76f412e8be1c3f2ada54e4d0e3b02b42415700ed24.md#sha256:5933276d8b1cc3e5db9d0c76f412e8be1c3f2ada54e4d0e3b02b42415700ed24
- raw:.hive/knowledge/Raw/skill/7490883eb8226e256e0faf21a47fff63dbf1a4afbd9fee3cc2f39ebe8cb53a3f.md#sha256:7490883eb8226e256e0faf21a47fff63dbf1a4afbd9fee3cc2f39ebe8cb53a3f
- raw:.hive/knowledge/Raw/skill/8fa13f64e2832923b9dc3e10aa92360290bc070d0f6a159a5266a4670567b3c6.md#sha256:8fa13f64e2832923b9dc3e10aa92360290bc070d0f6a159a5266a4670567b3c6
- raw:.hive/knowledge/Raw/skill/ad53daba2c3a2d592ee57105d2a24f11988c9490ecfd6761900e69a0021de056.md#sha256:ad53daba2c3a2d592ee57105d2a24f11988c9490ecfd6761900e69a0021de056
- raw:.hive/knowledge/Raw/skill/e137462c18a697a054d0be588dd28750130af1658b0cad1796d84e176fe8fb3a.md#sha256:e137462c18a697a054d0be588dd28750130af1658b0cad1796d84e176fe8fb3a
links:
- wiki-human-edited-wiki-push-pipeline
contradictions: []
status: open-question
created_at: 2026-08-11T05:11:46Z
updated_at: 2026-08-11T06:51:25Z
---

# 개발자 문답형 위키 초안 Skill

## 현재 동작

- 개발자 문답과 최대 8개 항목의 압축 초안 작성
- `docs/wiki/README.md` 기준 대분류와 같은 주제 문서 확인
- `docs/wiki/<분류>/<문서명>/document.md` 경로 제안 후 개발자 승인 대기
- 제목·부제목·최근수정일·최근수정자·관련문서 frontmatter 생성
- `attachments/.gitkeep`과 문서별 자료 링크 규칙 적용
- 기존 문서 덮어쓰기 금지와 충돌 시 갱신·대체 경로 선택 요청
- 문서 생성 후 절대 경로 기반 Obsidian URI로 문서 열기 요청
- 사람이 직접 생성한 빈 Markdown: `template/document.md` Templater 양식 사용

## 이번 결과

- 프로젝트: Maverick
- 도구: `skill-creator`, `draft-wiki-with-developer`
- 결과: Skill 생성 파일명을 `docs.md`에서 `document.md`로 통일
- 수용 기준: 승인 경로 사용, Git 작업자 식별, 최근 수정 정보 갱신, 첨부 자료의 문서별 소유

## 검증

- `skill-creator` 공식 빠른 검증 통과
- 8개 실제 위키 문서의 frontmatter·첨부 구조·내부 링크 검사 통과

## 남은 확인

- 새 Skill 흐름으로 다음 `document.md`와 attachments 자동 생성 실사용 확인
- Obsidian 생성 문서 탭 활성화 확인

## 요청 요약

- 기존 위키와 Skill의 표준 문서 파일명을 `document.md`로 변경 요청
