# PIE Action Test Airborne Groggy

## 작업 내용

- [x] PIE 액션 테스트 위젯의 Airborne 테스트에서 groggy damage를 제거했다.
- [x] 버튼 표시 문구도 `Groggy +0`으로 맞춰 테스트 의도가 드러나게 했다.

## 이유

Airborne 리액션 자체를 PIE에서 확인할 때 groggy 누적이 함께 들어가면, Airborne 흐름과 Groggy 진입 흐름이 섞여 테스트 결과를 해석하기 어렵다.

이번 변경은 런타임 전투 공식이나 일반 공격 데이터가 아니라 `MVPIEActionTestWidget`의 디버그 테스트 입력값만 조정한다.
