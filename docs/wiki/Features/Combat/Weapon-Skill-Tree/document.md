---
제목: "무기 스타일별 스킬 트리"
부제목: "스타일별 해금·장착 상태와 공유 스킬 포인트 관리 계약"
최근수정일: 2026-10-20
최근수정자: "Kwon-Nahoo-Man"
관련문서:
  - "[[Features/Combat/Skill/document|플레이어 스킬 구조]]"
  - "[[Features/Combat/Weapon-Swap/document|플레이어 무기 교체]]"
  - "[[Features/Combat/Combat-System/document|Maverick 전투 시스템]]"
---

# 무기 스타일별 스킬 트리

## 핵심

- 해금·장착 상태는 개별 무기가 아닌 `EMVEquippedStyle`별 공유
- 스킬 포인트는 모든 스타일이 함께 사용하는 `UMVWeaponComponent` 소유 재화

## 초기화와 변경

```mermaid
flowchart TD
    Begin["BeginPlay"] --> Build["스타일별 SkillBlueprint·SkillTreeStates 구성"]
    Build --> Save{"유효한 SaveGame 로드 성공?"}
    Save -->|예| Restore["무기·퀵슬롯·스킬 상태·공유 포인트 복원"]
    Restore --> Loaded["복원 완료 후 저장된 무기 상태 적용"]
    Save -->|아니오| Unlock["RequiredPoints <= 0 스킬 자동 해금"]
    Unlock --> Empty{"슬롯이 비어 있는가"}
    Empty -->|예| Equip["순회상 최초 유효 스킬 장착"]
    Empty -->|아니오| Keep["기존 장착 유지"]
    Equip --> Default["기본 무기 또는 맨손 장착"]
    Keep --> Default
    Request["UnlockSkill"] --> Check{"Row·상태·포인트 검증"}
    Check -->|성공| Spend["해금 추가와 공유 포인트 차감"]
    Check -->|실패| Unchanged["포인트·해금 상태 유지"]
```

- 잘못된 스타일·Row, 중복 해금, 포인트 부족 요청은 상태 불변
- 동일 스킬 재장착은 전투 갱신 없이 성공하는 멱등 처리
- 비현재 스타일 장착 변경은 상태만 저장하고 실제 스타일 변경에만 전체 전투 맵 갱신
- 현재 SaveGame 경로는 `if (0)`으로 비활성화하고 복원 범위를 `Todo:`로 유지

## 공개 API

| API | 계약 |
|---|---|
| `GetWeaponSkillState` | 스타일별 `FMVSkillTreeState` 사본 조회, 부재 시 출력 초기화와 `false` 반환 |
| `GetSkillTree` | 스타일별 `UMVWeaponSkillMap` 조회, 부재 시 `nullptr`와 `false` 반환 |
| `GetEquippedSkill` | 슬롯별 장착 맵 사본 조회, `None`은 현재 무기 스타일 사용 |
| `UnlockSkill` | 검증 성공 시 `max(0, RequiredPoints)` 차감과 해금 상태 추가 |
| `EquipSkill` | 해금 스킬의 슬롯 상태 변경, 현재 스타일 변경만 임시 전체 전투 갱신 |
| `GetSkillPoints` | 모든 스타일이 공유하는 현재 스킬 포인트 조회 |
| `AddSkillPoints` | 양수 포인트만 추가, `int32` 초과 요청 거부 |

## 남은 일

- 선택 Row를 직접 반영하는 슬롯 단위 `CombatComponent` 교체 API
- 스타일별 스킬 상태·공유 포인트·무기 진행 상태 Save/Load 및 변경 이벤트
