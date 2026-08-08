---
source_file: "Source/Maverick/Character/PC/Dodge/MVPlayerDodge.cpp"
type: "code"
community: "플레이어 회피 방향 해석"
location: "L368"
tags:
  - graphify/code
  - graphify/EXTRACTED
  - community/___
---

# UMVPlayerDodge::MakeDodgeInputContext()

## Connections
- [[DodgeClampControllerSpaceInput()]] - `calls` [EXTRACTED]
- [[DodgeResolveDirectionFromControllerSpaceInput()]] - `calls` [EXTRACTED]
- [[DodgeResolveWorldDirectionFromControllerSpaceInput()]] - `calls` [EXTRACTED]
- [[FMVDodgeInputContext]] - `references` [EXTRACTED]
- [[FVector2D_4]] - `references` [EXTRACTED]
- [[GetReferenceForwardVector()]] - `calls` [EXTRACTED]
- [[MVPlayerDodge.cpp]] - `contains` [EXTRACTED]
- [[ResolveDirectionVectorFromReferenceRotation()]] - `calls` [EXTRACTED]
- [[ResolveDodgeChooserDirection()]] - `calls` [EXTRACTED]
- [[ResolveDodgeFacingDirection()]] - `calls` [EXTRACTED]
- [[ResolveStrafeReferenceRotation()]] - `calls` [EXTRACTED]

#graphify/code #graphify/EXTRACTED #community/___