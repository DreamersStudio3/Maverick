# BackOff launch montage task

## Goal

- 임시 BackOff 애니메이션을 ActionComponent 없이 직접 재생하고, 타겟 반대 방향으로 Launch하는 StateTree 태스크를 추가한다.

## Checklist

- [x] 기존 StateTree 태스크 패턴 확인
- [x] 직접 몽타주 재생 + Launch 태스크 추가
- [x] 빌드 검증
- [x] 사용 방법 정리

## Result

- `BackOff Launch Montage Task` 추가.
- `ActionComponent`를 거치지 않고 `AnimInstance->Montage_Play()`로 몽타주를 직접 재생한다.
- Owner가 Target을 바라보도록 회전한 뒤, Target 반대 방향으로 `LaunchCharacter()`를 호출한다.
- `MaverickEditor Win64 Development` 빌드 성공.
