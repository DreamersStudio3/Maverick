# 사람용 위키와 Graphify 역할 분리

- 목표: `docs/wiki/`를 사람이 Obsidian으로 읽고 갱신하는 내부 위키로 확정하고, `graphify-out/`은 에이전트용 생성 지식창고로 정리
- 상태: 진행 중
- 결정:
  - 작업마다 push 전 위키 검토 필수
  - 위키 갱신 또는 갱신 불필요 사유를 TODO와 PR 설명에 기록
  - 사람용 Obsidian vault는 `docs/wiki/`
  - 생성 Obsidian vault는 기본 export와 Git 추적 대상에서 제외
- 검증: 문서·guard·ignore 수정, 생성물 재구축, pre-push 검사, 원격 push 예정
