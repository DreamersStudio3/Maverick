# Damage Applied HUD Event

## Goal

- [x] Add a generic stat-level event that reports actual HP damage after clamping.
- [x] Forward boss damage events from the boss HP widget to Widget Blueprint.
- [x] Reject the owning player's StatComponent when binding boss status.
- [x] Ignore raw `UpdateBossStatus` calls while the boss widget is bound to a StatComponent.
- [x] Rename groggy-only recovery delay to recent-damage reset delay.
- [x] Accumulate recent applied damage while attacks keep landing.
- [x] Reset accumulated HUD damage after the recent-damage reset delay expires.
- [ ] Configure the boss Widget BP to display the damage text or floating number.
- [x] Remove temporary HUD binding trace logs after verifying Main HUD boss widget binding.

## Notes

- `UMVStatComponent::OnDamageApplied` is the reusable source event for any actor that owns a stat component.
- `UMVBossHPBarWidget::BP_OnBossDamageApplied` is the boss HUD hook for Blueprint presentation.
- `AppliedDamage` is computed from `PreviousHP - CurrentHP`, so overkill damage displays only the HP that was actually removed.
- Temporary HUD binding trace logs were removed after the boss widget binding issue was fixed.
- `UMVMainHUDWidget::BindBossStatus` now rejects the owning player's stat component, and `UpdateBossStatus` is treated as a legacy/manual path while no StatComponent binding exists.
- `RecentDamageResetDelay` replaces the groggy-only delay as the shared recent-hit window for groggy recovery delay and HUD damage accumulation reset.
- `GroggyRecoveryDelay` is kept only as a deprecated data-table fallback so existing rows keep their old values until the table column is migrated.
- Boss HUD now listens to `OnDamageAccumulated`; the existing `BP_OnBossDamageApplied` first argument is fed with accumulated damage for compatibility, and `BP_OnBossDamageAccumulationReset` is called when the window expires.
