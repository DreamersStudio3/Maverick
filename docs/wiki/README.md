---
제목: "프로젝트 매버릭 위키"
부제목: "기능 지식의 팀 공유와 인수인계를 위한 문서 시작점"
최근수정일: 2026-08-12
최근수정자: "곽민규"
관련문서:
  - "[[Architecture/document|Maverick Architecture]]"
  - "[[Convention/Header-Documentation/document|C++ 헤더 책임 문서화]]"
---

# 프로젝트 매버릭 위키

## 목적

- 프로젝트 Maverick의 기능·설계 지식을 모은 팀 백과사전
- 기능 지식의 작업자 귀속 방지
- 신규 작업자의 인수인계 단절과 재조사 비용 축소

## 지식 체계

| 구분 | 상태 | 핵심 역할 | 갱신 기준 |
|---|---|---|---|
| Graphify | 사용 | 현재 프로젝트의 구조·관계 지도 | 관련 질문 직전 초기화·증분 갱신 |
| 사내위키 | 사용·정본 | 팀원이 읽고 인수인계에 사용하는 설계·운영 지식 | 구현·검증 완료 후 원격 반영 전 |
| Hive Knowledge | 비활성·기록 삭제 | 비사용 | 신규 자동 포착·질의 금지 |

- 구조·호출·관계 질문: Graphify 우선
- 설계 의도·운영법·인수인계: 사내위키 기록
- `.hive/knowledge/Raw`, `.hive/knowledge/Wiki`: 기존 기록 삭제와 신규 축적 금지

## 분류

- `Architecture/`: 전체 구조와 책임 경계
- `Convention/`: 코드·문서 작성 규칙
- `Features/`: 기능별 설계와 구현 계약
- `Research/`: 시장·설계·도구 조사

## 문서 구조

```text
<Category>/<Document-Name>/
├─ document.md
└─ attachments/
   └─ <descriptive-name>.<ext>
```

## 작성 기준

- 문체와 압축 기준: 루트 `POLICY.md`의 `프로젝트 문체` 준수
- 기능 흐름과 상태 전이: Mermaid 우선
- Mermaid와 동일한 산문 반복 금지
- 조사 대기 메모와 미확정 연결 제외
- 확정된 책임·흐름·운영 계약만 기록
- `document.md` 첫머리: 제목·부제목·최근 수정·관련 문서 Frontmatter
- 새 문서 파일명: `document.md`
- 사진·GIF: 같은 문서 폴더의 `attachments/`에 저장
- 첨부 파일명: 영문 소문자와 하이픈을 사용한 설명형 이름
- 첨부 링크: `![설명](attachments/combat-flow.gif)` 형식
- 새 문서 추가 전 기존 대분류와 관련 문서 확인

## Templater 자동 양식

- 템플릿: `template/document.md`
- 적용 범위: 보관함 루트 `/`와 모든 하위 폴더
- 제외 범위: `template/`
- 새 빈 Markdown 생성: 제목·부제목·최근 수정자 문답 후 Frontmatter와 H1 생성
- `Trigger Templater on new file creation`: 장치별 활성화 필요
- 공유 설정: `.obsidian/plugins/templater-obsidian/manifest.json`, `.obsidian/plugins/templater-obsidian/data.json`

## 작업 흐름

- 에이전트: 구현·검증 후 개발자 문답과 압축 초안 생성
- 개발자: `docs/wiki/`를 Obsidian 보관함으로 열어 수정·첨삭
- 편집 완료 전달 이후: 위키 커밋과 관련 코드 커밋을 함께 push
- PR 작업 세부사항: 위키 커밋과 변경 문서 링크 포함
- `graphify-out/`: 에이전트용 로컬 캐시, Git 공유 제외

## Obsidian

- 보관함 경로: `docs/wiki/`
- 첫 연결: Obsidian 보관함 관리 화면에서 `docs/wiki/` 선택
- `.obsidian/app.json`: 새 첨부 파일을 현재 문서 폴더의 `attachments/`에 저장
- `.obsidian/community-plugins.json`, `.obsidian/templates.json`: 활성 플러그인 목록과 기본 템플릿 폴더 공유
- 커뮤니티 플러그인: `manifest.json`·`main.js`·`styles.css`와 검토를 마친 `data.json` 공유
- 플러그인 자격 증명: 공유 `data.json`의 API Key·Token 빈 값 유지
- 다른 장치: 저장소 동기화 후 보관함의 커뮤니티 플러그인 신뢰·활성화
- Git 제외: `workspace*.json`, 외형·개인 상태, 검토되지 않은 플러그인 `data.json`
- `graphify-out/obsidian/`: 사람용 위키 사용 금지
