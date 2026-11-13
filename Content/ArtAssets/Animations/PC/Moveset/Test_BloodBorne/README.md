# BloodBorne 테스트 Moveset

- 위치: `/Game/ArtAssets/Animations/PC/Moveset/Test_BloodBorne`
- 구성: Manny 루트 모션 시퀀스 126개, 착지 가산 시퀀스 1개, 몽타주 132개, 이동 BlendSpace 2개
- 명명: 시퀀스 `AS_BB_<종류>_<원본 ID>_Root`, 몽타주 `AM_BB_*`
- 분류: Detail / Dodge / Idle / Walk / Run / Sprint / Fall / Knockdown / Getup
- 기존 BareHand·OneHand·DualWield 에셋 유지, 동일 회피 시퀀스의 테스트 사본 6개 통합

## 게임플레이 연결

- develop 병합 시 플레이어 캐릭터 충돌은 develop 버전 채택, 아래 테스트 ABP 연결 기록은 병합 이전 기준
- 테스트 ABP와 에셋은 보관하며 현재 캐릭터에 자동 재연결하지 않음, 회피 테이블의 BloodBorne 참조는 유지

- 플레이어 일반 회피 18행은 새 Dodge 폴더의 기존 몽타주 12개 참조
- 기존 회피 몽타주의 길이·알림 보존, 시퀀스는 무기 스타일 간 공유
- BH·1H 이름의 몽타주는 기존 블렌드·타이밍 구분용, 별도 시퀀스 중복 없음
- 회피 외 몽타주는 개별 모션 확인용, 로코모션은 상태 머신의 시퀀스 재생 구조 사용
- 기본 대기·8방향 걷기/달리기·질주·정지·회전·낙하·착지를 ABP_Base_Test_BloodBorne와 Test_BloodBorne 플레이어 Anim Layer 3종에 연결
- 넘어짐 탈출·기상 연결은 이전 상태 유지
- 대각선 8개는 인접 원본 두 동작을 동일 정규화 시간으로 50:50 혼합한 `_Blend_Root` 파생본, 원본에 없는 대각선의 테스트용 보완
- 착지는 004200을 Idle 000000의 첫 프레임 기준 Local Space 가산 시퀀스로 변환
- 시퀀스 Force Root Lock 적용, 몽타주의 루트 모션과 기존 가산 기울기·AimOffset 유지
- 제자리 회전 보간 시간은 90도 20/30초, 180도 22/30초를 사용

## 지원 자료

- 소스 메시·스켈레톤: `/Game/ArtAssets/Mannequins/Meshes/Test_BloodBorne`
- IK Rig·리타게터: `/Game/ArtAssets/Mannequins/Rigs/Test_BloodBorne`
- 원본 스켈레톤 시퀀스 131개: 위 Rigs 폴더의 `SourceAnimations`
- 원본: GRACEBORNE a0.2.5 `c0000`, 출처 슬롯명은 원본 게임의 분류이며 모드 입력 선택 규칙과 별개
- ID·구간·에셋 경로: `asset-index.json`

## 검증 범위

- 전체 이동 에셋 로드, 시퀀스 길이·스켈레톤·루트 모션 설정 및 몽타주 참조 검증
- 기존 회피 몽타주 12개의 알림 개수·길이 및 DataTable 비참조 필드 보존
- 기상 FaceUp: `a000_004260` 30–137프레임, FaceDown: `a000_004270` 40–137프레임 분리본
- 원본 모션 길이와 궤적 보존, 발 접촉·무기 그립·전환 체감 보정 미포함

## 로코모션 연결 검증

- Windows UE 5.8.4에서 4개 ABP 재로드·컴파일 및 시퀀스/BlendSpace 참조 확인
- PIE 한손 플레이어의 8방향 걷기/달리기·전방 질주·정지·낙하/착지 실행, 111개 표본에서 루트와 캐릭터의 수평 거리 0 cm
- 맨손·쌍수는 클래스 기본값 검증, 실제 무기 전환과 대각선 발 접촉·회피 후 블렌딩 체감은 별도 확인 필요

## 전환·접지 보정

- NormalStop의 일반 정지 시퀀스를 가산 입력에서 Base 포즈 입력으로 이동, `ABP_Base_Test_BloodBorne`와 `ABP_PlayerAnimLayerBase_Test_BloodBorne` 모두 적용
- 정지 및 회피 몽타주 블렌딩 재현에서 뼈 스케일 최대 2048 → 약 1, 수정 후 1441표본 확인
- Idle 000000/000001의 발목·무릎·골반 키를 Manny 접지 자세에 맞춰 보정, 발끝 XY와 뼈 길이·스케일 유지
- Idle 000000은 골반 약 2.75–3.23 cm 하강, 원본 리타게팅 결과와 구별되는 테스트 보정본
- 모션 편집 위치와 가산 입력 주의사항: `docs/wiki/Features/Player-Animation/document.md`

## 테스트 ABP 분리

- 기존 ABP 4개는 Git 원본 상태로 복원
- `Characters/AnimBP/ABP_Base_Test_BloodBorne` 및 `AnimLayer/*Test_BloodBorne` 3개로 동일 구조 분리
- `BP_ThirdPersonCharacter`의 Mesh Anim Class와 `ApplyWeaponAnimationLayer` 분기 4개에 테스트 클래스 지정
- 애니메이션 에셋은 공유하며 ABP·레이어만 분리, 회피 데이터테이블 연결은 유지
