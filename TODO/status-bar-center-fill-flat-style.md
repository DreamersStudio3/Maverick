# Status Bar Center Fill And Flat Style

- [x] 공용 StatusBar를 사용하는 인스턴스별로 채우기 방향을 지정할 수 있는 API 추가
- [x] BossHPBar의 GroggyBar 인스턴스만 중앙 기준 가로 채우기로 설정
- [x] 기본 ProgressBar 그라데이션 대신 단색 Fill 브러시를 사용해 평평한 외형 적용
- [x] ProgressBar 기본 Background 외곽선을 제거하고 최상단 Outline만 보이도록 배경 레이어 정리
- [x] WBP_StatusBar 브러시 구성을 C++ 런타임 스타일 갱신이 덮어쓰지 않도록 소유권 분리
- [x] Maverick 게임 타깃에서 변경된 두 C++ 파일의 UHT 및 개별 컴파일 성공 확인
- [ ] 열린 Unreal Editor의 Live Coding 종료 후 MaverickEditor 전체 빌드 확인

## 검증 메모

- `MVStatusBarWidget.cpp`, `MVBossHPBarWidget.cpp` 컴파일 성공
- 게임 타깃 전체 빌드는 기존 `MVSheetSpecs.cpp`의 `UScriptStruct::HasMetaData/GetMetaData` 사용 오류에서 중단
- 에디터 타깃 빌드는 실행 중인 Live Coding 세션 때문에 중단
