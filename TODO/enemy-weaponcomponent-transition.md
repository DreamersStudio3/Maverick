# Enemy WeaponComponent Transition

## Status

- [x] Removed `AMVEnemy` runtime BP weapon spawn/attach path.
- [x] Stopped `AMVEnemy::EndPlay` from destroying the legacy BP weapon actor.
- [x] Updated `AMVEnemy` class responsibility comment.
- [x] C++ build verification.
- [x] Added secondary weapon mesh support to `UMVWeaponComponent`.
- [x] Kept trace socket lookup in Blueprint by exposing managed mesh components instead of wrapper functions.
- [ ] Verify enemy weapon visual/equip data is configured through `UMVWeaponComponent`.
- [ ] Rewire melee trace ability away from `MyMVEnemyWeapon` and toward `UMVWeaponComponent`/equipped weapon mesh data.

## Notes

`AMVEnemy::BeginPlay` no longer creates `AMVEnemyWeapon` or calls:

```text
AttachDualToHands
AttachCombinedToHand
```

The legacy `WeaponActor` members are still present for asset compatibility, but the enemy class no longer equips that BP weapon actor by itself. The next step is to make enemy weapon visuals and trace source data come from the shared weapon component path.

`UMVWeaponComponent` now keeps `WeaponMeshComponent` private and manages a second private `SecondaryWeaponMeshComponent` internally. Both are `BlueprintReadOnly` with `AllowPrivateAccess`, so Blueprint trace code can read sockets from the component references without adding dedicated C++ socket query functions:

```text
WeaponMeshComponent -> Get Socket Location Trace_Start / Trace_End
SecondaryWeaponMeshComponent -> Get Socket Location Trace_Start / Trace_End
```
