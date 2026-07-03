#include "Tags/MVGameplayTags.h"

namespace MVGameplayTags
{
	UE_DEFINE_GAMEPLAY_TAG(Character_Player_P1, "Character.Player.P1");
	UE_DEFINE_GAMEPLAY_TAG(Character_NPC_Enemy_E1, "Character.NPC.Enemy.E1");
	UE_DEFINE_GAMEPLAY_TAG(Character_NPC_Enemy_NamelessPuppet, "Character.NPC.Enemy.NamelessPuppet");
	UE_DEFINE_GAMEPLAY_TAG(Character_NPC_Neutral_N1, "Character.NPC.Neutral.N1");
	UE_DEFINE_GAMEPLAY_TAG(Event_HitReaction_ApplyDamage, "Event.HitReaction.ApplyDamage");
	UE_DEFINE_GAMEPLAY_TAG(Interaction_Command_Event_HitReaction_ApplyDamage, "Interaction.Command.Event.HitReaction.ApplyDamage");
	UE_DEFINE_GAMEPLAY_TAG(Interaction_Choice_Entry, "Interaction.Choice.Entry");
	UE_DEFINE_GAMEPLAY_TAG(Interaction_Dialogue, "Interaction.Dialogue");
	UE_DEFINE_GAMEPLAY_TAG(Interaction_Flow_Step, "Interaction.Flow.Step");
	UE_DEFINE_GAMEPLAY_TAG(Interaction_Menu, "Interaction.Menu");
	UE_DEFINE_GAMEPLAY_TAG(Interaction_Menu_Entry, "Interaction.Menu.Entry");
	UE_DEFINE_GAMEPLAY_TAG(Interaction_Menu_Entry_Checkpoint_LevelUp, "Interaction.Menu.Entry.Checkpoint.LevelUp");
	UE_DEFINE_GAMEPLAY_TAG(Interaction_Menu_Entry_Checkpoint_NoTravelTargets, "Interaction.Menu.Entry.Checkpoint.NoTravelTargets");
	UE_DEFINE_GAMEPLAY_TAG(Interaction_Menu_Entry_Checkpoint_Rest, "Interaction.Menu.Entry.Checkpoint.Rest");
	UE_DEFINE_GAMEPLAY_TAG(Interaction_Menu_Entry_Checkpoint_Storage, "Interaction.Menu.Entry.Checkpoint.Storage");
	UE_DEFINE_GAMEPLAY_TAG(Interaction_Menu_Entry_Checkpoint_Travel, "Interaction.Menu.Entry.Checkpoint.Travel");
	UE_DEFINE_GAMEPLAY_TAG(Interaction_Menu_Page, "Interaction.Menu.Page");
	UE_DEFINE_GAMEPLAY_TAG(Interaction_Menu_Page_Checkpoint, "Interaction.Menu.Page.Checkpoint");
	UE_DEFINE_GAMEPLAY_TAG(Interaction_Menu_Page_Checkpoint_Travel, "Interaction.Menu.Page.Checkpoint.Travel");
	UE_DEFINE_GAMEPLAY_TAG(StateTreeEvent_Enemy_Hit, "StateTreeEvent.Enemy.Hit");
}
