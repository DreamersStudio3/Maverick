---
제목: "레벨업 UI 참고 위젯"
부제목: "외부 프레임워크 의존성을 분리한 배치·스타일 자료"
최근수정일: 2026-09-18
관련문서:
  - "[[Architecture/document|Maverick Architecture]]"
---

# 레벨업 UI 참고 위젯

## 위치와 범위

- 대상: `/Game/UI/LevelUp/RestMenu/LevelUpMenu/W_LevelUp.W_LevelUp`
- 원본 프로젝트: `I:/Workspace/Unreal/Framework-testbed/FrameworkTest`, SoulslikeFramework 위젯
- 용도: 새 레벨업 시스템 제작을 위한 디자이너 배치·스타일 참고
- 구성: `UserWidget` 직접 상속 위젯 10개, 글꼴·재질·텍스처 18개
- 보존: 위젯 계층·슬롯 배치·표시 스타일, 제목과 통화 영역의 정적 문구
- 제외: 원본 게임 로직·입력 연결·능력치 목록 생성·전환 애니메이션
- 정적 표시: 원본 예시 숫자, 빈 능력치 패널, 숨긴 전체 흐림막과 미연결 입력 아이콘

## 열기 실패 원인과 복구

- 파일 이동 후에도 원본 `/Game/SoulslikeFramework/` 참조 유지
- 부모 `W_Navigable_InputReader`·`W_Navigable` 및 종속 에셋 누락으로 부모 클래스 해석 실패
- 에디터에서 원본 디자인 트리를 새 위젯으로 재구성하고 필요한 표시 에셋만 이관
- 엔진의 에셋 이름 변경·저장 기능을 통한 `/Game/UI/LevelUp/` 참조 정리
- 파일 탐색기의 `.uasset` 이동과 내부 참조 경로 갱신은 별개 작업
- 기존 `RestMenu`·`_Generic` 에셋 8개는 복구 범위 밖, 원본 바이트 보존
- 로컬 백업: `Saved/LevelUpRecovery/OriginalBackup/LevelUp`, Git 제외 임시 자료

## 검증 범위

- Windows 11·UE 5.8.1에서 실제 `W_LevelUp` 디자이너 열기·화면 확인·저장 완료
- 최종 저장본만 포함한 독립 프로젝트에서 28개 로드·10개 컴파일 실행, 오류·경고 0건
- 위젯 10개 모두 `/Script/UMG.UserWidget` 직접 상속, 다른 프로젝트 경로 의존성 검사 통과
- 독립 검증은 `-nullrhi` 실행으로 로드·컴파일 확인에 한정, 화면은 실제 에디터에서 별도 확인
- PIE·실제 레벨업 동작 검증 제외: 게임 로직을 포함하지 않는 참고 위젯 범위
