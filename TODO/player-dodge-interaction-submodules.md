# Player dodge/interaction submodules

- [x] `POLICY.md` 확인
- [x] 기존 Dodge 컴포넌트 구현을 `PlayerCharacter.Dodge` 서브모듈로 이동
- [x] 기존 InteractionDetector 컴포넌트 구현을 `PlayerCharacter.InteractionDetector` 서브모듈로 이동
- [x] `CharacterBase`에서 플레이어 전용 컴포넌트 책임 제거
- [x] 기존 참조, 설정 리다이렉트, 문서 정리
- [x] 빌드 또는 정적 검증
- [x] 미사용 DodgeLaunch AnimNotifyState와 no-op 호환 API 제거
- [x] 전력질주 스태미나 소모 정책을 PlayerCharacter로 이동
- [x] 옛 컴포넌트 자산 정리 후 호환 shim 제거
- [x] BP 입력 그래프용 PlayerCharacter 상호작용 wrapper 추가
