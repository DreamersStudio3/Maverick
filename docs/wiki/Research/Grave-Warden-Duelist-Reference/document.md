---
제목: "묘지기 투사 참고 에셋"
부제목: "코트 없는 쌍망치 변형의 메시와 원본 ID별 동작 자료"
최근수정일: 2026-10-03
최근수정자: "Codex"
관련문서:
  - "[[Architecture/document|Maverick Architecture]]"
---

# 묘지기 투사 참고 에셋

## 범위와 위치

- 출처: 로컬 ELDEN RING 설치본에서 기존 추출·변환한 `c3400` 참고 자료
- 대상: 코트 없는 쌍망치 변형, 망치와 연결 사슬 포함
- 프로젝트 위치: `Content/DuelistComplete`, Unreal 경로 `/Game/DuelistComplete`
- 원본 보관: `C:/Users/mindo/Workspace/AssetReviews/EldenRingDuelist`
- 편집용 FBX·Blender·원본 게임 묶음은 위 보관 위치에 유지, 저장소에는 Unreal 에셋만 포함

| 위치 | 구성 | 개수 |
|---|---|---:|
| `Mesh` | `SK_Duelist_TwinHammers_NoCoat`와 전용 Skeleton | 2 |
| `Materials` | 피부·장비·천·망치 재질 | 9 |
| `Textures` | 색상·노멀·거칠기·금속성 텍스처 | 19 |
| `Animations` | 원본 ID를 유지한 AnimSequence | 325 |
| `Review` | 외형 확인용 `L_DuelistReview` 레벨 | 1 |

## 동작 탐색

- 메시 시작점: `/Game/DuelistComplete/Mesh/SK_Duelist_TwinHammers_NoCoat`
- 쌍망치 기본 자세: `/Game/DuelistComplete/Animations/A_Duelist_a001_000000`
- 쌍망치 동작 탐색: `Animations` 폴더에서 `a001` 검색

| 원본 접두사 | 개수 | 적용 범위 |
|---|---:|---|
| `a000` | 181 | 다른 무기 형태와 공용·보조 동작 포함, 일부 동작의 쌍망치 위치 불일치 |
| `a001` | 142 | 쌍망치 기본 자세 포함, 개별 공격 의미의 전체 분류 범위 밖 |
| `a002` | 1 | 용도 미분류 원본 추가 동작 |
| `a004` | 1 | 용도 미분류 원본 추가 동작 |

325개는 `c3400` 관련 원본 동작 묶음의 개수로, 독립 공격 325종과 구분

## 반영 검증

- 환경·실행: 2026-10-03 Windows·Codex, Unreal Engine `5.8.2-56702186+++UE5+Release-5.8` 명령줄 검증 실제 실행
- 방법: 저장소의 `Content/DuelistComplete` 복사본을 별도 검증 프로젝트의 동일 `/Game/DuelistComplete` 경로에 연결, `NullRHI` 사용
- 파일 무결성: 356개 전체의 SHA-256 원본 일치, 합계 509,476,318 bytes, 최대 파일 7,721,875 bytes
- 엔진 로딩: 위 표의 356개 에셋과 메시 재질 슬롯 9개 연결 확인
- 동작 유효성: 325개 전체의 Skeleton·길이와 시작·중간·끝 압축 자세 검사 통과, 각 표본의 786개 본 변환값 유한성 확인
- 원본 자세 비교: 대표 동작 2개의 3시점 비교 통과, 축·단위 변환과 Master 기준 정렬 후 최대 본 위치 오차 0.7631cm 미만
- 패키지 참조: Asset Registry 기준 외부 `/Game/` 의존성과 누락 `/Game/` 패키지 각 0개
- 증명 범위: 파일 보존·엔진 로딩·참조 연결·표본 자세 유효성; 실제 화면 렌더링과 Maverick 게임 내 동작 보증 범위 밖
- 검증 요약: [반영 검사 결과](attachments/transfer-validation.json)

## 적용 한계

- 메시·동작: 원본 본 계층과 루트 이동을 포함한 참고용 변환 자료
- 재질: 원작의 공용 미세 질감·다층 혼합·전용 효과 완전 재현 범위 밖
- 사슬·천: 저장된 본 애니메이션 포함, 원작 실시간 물리와 제어의 Chaos 재구성 미실행
- 이번 Windows·Codex 작업: 기존 변환본의 파일 이동과 유효성 확인 범위; AI·공격 판정·몽타주·효과·소리 연결 및 Maverick 게임 내 재생 미실행
- C++ 변경 부재로 이번 Windows 작업에서 프로젝트 빌드·패키징 미실행; 실행 코드·배포본 검증 근거와 구분
- 원작 대비 외형 승인과 전체 프레임 육안 재생 검사는 이번 작업 범위 밖
