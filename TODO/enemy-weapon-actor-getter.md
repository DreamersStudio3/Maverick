# Enemy Weapon Actor Getter

## Status

- [x] `AMVEnemy` weapon ownership flow checked.
- [x] Blueprint-readable `GetWeaponActor` API added.
- [x] C++ build verification.
- [ ] Blueprint graph target rewired to `GetWeaponActor`.

## Blueprint Usage

Use this flow in the dual weapon melee ability:

```text
GetOwnerCharacter
  -> Cast to MVEnemy
  -> GetWeaponActor
  -> Is Valid
  -> GetMeleeDualWeaponData
```

`GetMeleeDualWeaponData` target should be the returned weapon actor, not the enemy character, if the interface is implemented on `MyMVEnemyWeapon`.
