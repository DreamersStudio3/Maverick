# 멀티플레이 액션 프로필 설계 메모

## 배경

정식 출시 시점에는 제한적인 리슨 서버 멀티플레이를 지원할 가능성이 있다. 호스트 플레이어와 초대된 협력자는 런타임 소유권과 네트워크 식별자는 다르지만, 같은 플레이어 액션셋을 공유할 수 있다.

## 설계 원칙

- CharacterIndexId는 네트워크 플레이어 식별자가 아니라 캐릭터 데이터 정의 ID로 사용한다.
- ActionProfileId는 사용할 액션 목록, 스탯, 애니메이션 Chooser 묶음을 선택하는 ID로 사용한다.
- 호스트 플레이어와 협력자가 같은 액션셋을 사용하면 같은 ActionProfileId를 공유한다.
- PlayerState, NetId, Controller, Actor는 멀티플레이 런타임 식별자로 별도 관리한다.
- 적대/우호/소환자 여부는 CharacterIndexId가 아니라 Faction, Team, SummonType 같은 게임 규칙 데이터로 분리한다.

## 추후 작업

- [ ] ActionComponent의 CharacterIndexId 또는 ActionProfileId 복제 정책 정의
- [ ] 서버 권한 액션 시작 요청 흐름 설계
- [ ] 클라이언트 예측이 필요한 액션과 서버 판정 전용 액션 분류
- [ ] 협력자용 CharacterIndex row가 플레이어 ActionProfileId를 공유할지, 별도 보정 프로필을 가질지 결정
- [ ] PvE 소환/협력 상태를 나타내는 팀/진영 데이터 설계

