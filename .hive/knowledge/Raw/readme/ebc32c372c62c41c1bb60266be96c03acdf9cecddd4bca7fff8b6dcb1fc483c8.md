---
제목: "프로젝트 매버릭 위키"
부제목: "기능 지식의 팀 공유와 인수인계를 위한 문서 시작점"
최근수정일: 2026-08-11
최근수정자: "곽민규"
관련문서:
  - "[[Architecture/docs|Maverick Architecture]]"
  - "[[Convention/Header-Documentation/docs|C++ 헤더 책임 문서화]]"
---

# 프로젝트 매버릭 위키

## 위키란 무엇인가

- 프로젝트 Maverick의 기능과 설계 지식을 모은 팀 백과사전

## 위키를 써야 하는 이유

- 기능 지식의 작업자 귀속 방지
- 새 작업자의 인수인계 단절과 재조사 비용 축소

## 분류

- `Architecture/`: 전체 구조와 책임 경계
- `Convention/`: 코드·문서 작성 규칙
- `Features/`: 기능별 설계와 구현 계약
- `Research/`: 시장·설계·도구 조사

각 문서의 기본 구조:

```text
<Category>/<Document-Name>/
├─ docs.md
└─ attachments/
   └─ <descriptive-name>.<ext>
```

## 문서 작성 요령

- `docs.md` 첫머리에 제목·부제목·최근 수정·관련 문서 frontmatter 유지
- 사람이 직접 생성한 Markdown에는 frontmatter 자동 추가 없음; 새 문서는 위키 초안 Skill 사용
- 사진·GIF 등은 같은 문서 폴더의 `attachments/`에 저장
- 첨부 파일명은 영문 소문자와 하이픈을 사용한 설명형 이름 권장
- 첨부 링크는 `![설명](attachments/combat-flow.gif)` 형식 사용
- 새 문서는 기존 대분류와 관련 문서를 확인한 뒤 추가

## 작업 흐름

- 에이전트: 구현·검증 후 개발자 문답을 거쳐 압축 초안과 문서 생성
- 개발자: `docs/wiki/`를 Obsidian 보관함으로 열어 수정·첨삭
- 편집 완료 전달 이후: 별도 위키 커밋과 관련 코드 커밋을 함께 push
- PR 작업 세부사항: 위키 커밋과 변경 문서 링크 포함
- `graphify-out/`: 에이전트용 로컬 캐시, Git 공유 대상 제외

## Obsidian

- 보관함 경로: `docs/wiki/`
- 첫 연결: Obsidian의 보관함 관리 화면에서 `docs/wiki/` 폴더 선택
- `.obsidian/`: 개인 화면·플러그인 설정, Git 공유 대상 제외
- `graphify-out/obsidian/`: 사람용 위키로 사용 금지
