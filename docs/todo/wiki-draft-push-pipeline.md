# 위키 초안·push 파이프라인

- 목표: 위키 승인 단계 없이 에이전트 초안, 사람 편집, 위키 커밋, 원격 push 흐름 정립
- 상태: `document.md` 파일명 통일, Templater 공유 설정, Architecture 루트 문서와 Features 직속 런타임 흐름 분리 반영 완료; 장치별 자동 실행 스위치 활성화 필요
- 결정: 사람의 편집 완료 전달을 최종화 신호로 사용, 초안 비교·승인 표식·내용 해시 제외, 승인받은 영문 경로의 `document.md`와 `attachments/` 생성 후 Obsidian 열기, frontmatter는 제목·부제목·최근수정일·최근수정자·관련문서로 제한, 사람의 수동 Markdown 생성에는 Templater 폴더 양식 적용, Architecture 진입점은 `Architecture/document.md`, 세부 실행 흐름은 `Features/` 직속 문서 폴더, Architecture 본문의 관련 문서 목록 대신 대상 문서의 직접 백링크 사용
- 위키 검토: `갱신: docs/wiki/README.md, Architecture/document.md, Features/{AI-StateTree,Death-and-Field-Transition,Hit-Stat-HitReaction,Input-to-Action,LockOnTarget-Boundary,Table-Data,UI-and-CommonUI}/document.md`
- 검증: 필수 frontmatter, 개별 문서 attachments 구조, Wiki·Markdown 링크, Templater 2.25.0 공유 설정, Skill 공식 검사, Architecture 하위 문서 7개·Entry 링크 7개, 관련 문서 직접 백링크, Hive 지식 lint 통과
