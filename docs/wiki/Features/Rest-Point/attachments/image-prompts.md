# UI 이미지 생성 요청문

- 생성일: 2026-11-09
- 도구: 내장 image_gen
- 성격: 구현 전 시안, 실제 게임 화면·에셋·밸런스의 검증 결과 아님
- 입력 이미지: 없음, 검색한 게임 UI의 구조를 문장으로 정리해 새 이미지 생성
- 출력: 같은 폴더의 PNG 3장

## 거점 메뉴

```text
Use case: ui-mockup
Asset type: Maverick third-person soulslike game, original rest-point menu prototype, single full-screen 16:9 1920x1080 landscape frame, Korean UI.
Primary request: A polished production-oriented concept screenshot inspired by the understated vertical Site of Grace menu in Elden Ring, the economical bonfire list in Dark Souls 1, and Lies of P's atmospheric Stargazer equipment menu. Create an original design, not a copy of any game's assets.
Scene/backdrop: A ruined stone sanctuary at dusk, an armored traveler seated beside a low weathered stone beacon with a small pale amber light, misty muted slate architecture. Keep the environment visible on the right two thirds, cinematic but secondary to exceptionally legible UI. No sword-in-bonfire icon, no copyrighted game logo.
Composition: 5% safe margins, left 32% charcoal translucent gradient panel, refined ivory Korean type, thin muted brass horizontal rules, restrained warm gold selection with a small pointer. Large title at upper left, location below. Only four vertically spaced menu options; selected option "휴식하기". Brief selected-action explanation beneath list, generous breathing room. Small currency at upper right. Controller-neutral keyboard hints at lower left. No decorative dashboard cards, no clutter.
Text verbatim:
"휴식 거점"
"잿빛 성소"
"휴식하기"
"레벨업"
"무기 스킬"
"떠나기"
"생명력과 소모품을 회복합니다"
"일반 적이 다시 배치됩니다"
"보유 재화  12,840"
"Enter  선택"
"Esc  돌아가기"
Typography: crisp readable Korean, faithful exact spelling, large enough for a living-room game UI; warm ivory body with bright gold selected text. No additional prose or English heading.
Constraints: Show an actual game UI prototype, not a website, not a presentation, one coherent 16:9 frame. Subtle vignette and atmospheric backdrop, dark readable menu. No unnecessary inventory panels, no health HUD.
```

## 레벨업

```text
Use case: ui-mockup
Asset type: Maverick original soulslike rest-point LEVEL-UP screen, Korean UI, one full-screen 16:9 landscape image.
Primary request: Create a highly readable, elegant dark fantasy game level-up menu in the same family as a restrained charcoal, ivory and muted gold bonfire interface. Inspired by Elden Ring's clear current-versus-preview comparison, Dark Souls' terse readable lists and Lies of P's tidy information hierarchy. Original art.
Scene: same misty ruined stone sanctuary at dusk with armored traveler and small amber stone beacon, heavily dimmed under large translucent menu panels. Interface dominates.
Composition: Title top left "레벨업", small breadcrumb "휴식 거점 / 잿빛 성소". Upper full-width slim summary strip with four evenly spaced fields. Below: left 40% investment list and right 48% preview comparison table, separated by one thin gold vertical line. A selected HP row has a faint warm highlight and pointer. Large clean readable Korean. Bottom right bordered confirm button. Footer control hints. Straight-on game screenshot, no device frame.
Exact text and numbers:
Top summary: "레벨" "24 → 25" ; "보유 재화" "12,840" ; "필요 재화" "1,250" ; "남은 재화" "11,590"
Left heading "능력치 투자"
Three rows only: "생명력" "−   6 → 7   +" ; "지구력" "−   5   +" ; "정신력" "−   12   +"
Right heading "변경 미리보기"
Column headers "현재" "변경 후"
Rows "최대 생명력" "220" "240"; "최대 스태미너" "150" "150"; "최대 마나" "220" "220"
Only changed number 240 is pale teal with an up arrow. No other attributes.
Small helper "확정 전에는 재화가 사용되지 않습니다"
Button "레벨업 확정"
Footer "↑ ↓  항목 선택" "← →  투자 조절" "Enter  확정" "Esc  돌아가기"
Constraints: no website cards, no unrelated equipment, no ornate excessive borders, no extra text, generous 5% safe margins, visually disciplined game-ready mockup. Numbers must remain exactly correct. Original design.
```

## 무기 스킬 선택

```text
Use case: ui-mockup
Asset type: Maverick original soulslike weapon skill selection prototype, one full-screen 16:9 Korean game UI.
Primary request: A polished dark charcoal, ivory text, muted brass-lined interface from the same rest-point menu family, informed by Elden Ring and Dark Souls' restrained list typography and Lies of P's readable equipment management layout. Original assets and iconography.
Scene: heavily dimmed blurred stone sanctuary background with subtle warm amber beacon glow. Three strong columns, generous margins; no website styling.
Header "무기 스킬" with breadcrumb "휴식 거점 / 잿빛 성소"; upper right "스킬 포인트  3".
Left column 22%: heading "보유 무기", three vertically stacked slim rows with original item drawings and labels "카타나", "대검", "활". Katana is selected with soft gold fill. Footer note "같은 무기 종류끼리 선택을 공유합니다".
Middle column 32%: tabs "사용 스킬" and "패시브", "패시브" is active with gold underline. Header "패시브 선택". Three candidate rows:
"폭풍 칼날" with outlined gold focus and a small arrow, sublabel "미리보기"
"약점 포착" with a pale teal checkmark and label "장착 중"
"잔상 이동" subdued with a clear padlock and label "잠김"
Focus and equipped state MUST visibly belong to different rows. Include a fourth neutral text action "선택 해제".
Right column 38%: title "폭풍 칼날"; a large monochrome etched illustration of flowing wind around an original curved sword (no logo), elegant contained art panel. Below:
"연속 공격으로 공격속도가 증가합니다"
"중첩당 +2% · 최대 +20%"
"피격 시 중첩 초기화"
A prominent bordered button "적용", small helper "적용 전에는 현재 패시브가 유지됩니다".
Footer exactly "← →  영역 이동" "↑ ↓  항목 선택" "Enter  적용" "Esc  돌아가기".
Style: refined readable Korean, ivory body, warm gold for focus only, desaturated teal for equipped badge, thin lines and high contrast. Make distinction between preview versus currently equipped unmistakable. Broad negative space and clear hierarchy. No extra stats or extra text. The layout should look ready to implement in Unreal CommonUI, not an explanatory design board.
```
