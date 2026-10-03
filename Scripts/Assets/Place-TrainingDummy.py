"""BaseBossTestLevel 바닥 중앙의 훈련용 허수아비 1개 배치"""
import json
import traceback
from pathlib import Path
import unreal

report = {"errors": []}
project = Path(unreal.Paths.project_dir()).resolve()
level = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
try:
    assert level.load_level("/Game/Characters/NPC/BaseBoss/Level/BaseBossTestLevel")
    all_actors = actors.get_all_level_actors()
    floor = next(a for a in all_actors if a.get_actor_label() == "Floor")
    center, extent = floor.get_actor_bounds(False)
    ground_z = center.z + extent.z
    dummy_class = unreal.EditorAssetLibrary.load_blueprint_class("/Game/Characters/NPC/TrainingDummy/BP_TrainingDummy")
    existing = [a for a in all_actors if isinstance(a, unreal.MVTrainingDummy)]
    assert len(existing) <= 1, "Multiple training dummies: manual review required"
    position = unreal.Vector(center.x, center.y, ground_z + 100)
    dummy = existing[0] if existing else actors.spawn_actor_from_class(dummy_class, position)
    dummy.set_actor_location(position, False, True)
    dummy.set_actor_rotation(unreal.Rotator(0, 0, 180), True)
    dummy.set_actor_label("TrainingDummy_LiesOfP")
    # 원점의 보스·시작 위치와 새 훈련 대상의 겹침 해소
    for actor in all_actors:
        if actor.get_actor_label() == "TutorialBoss_Test":
            old = actor.get_actor_location()
            actor.set_actor_location(unreal.Vector(center.x + 2500, center.y + 2500, old.z), False, True)
        elif isinstance(actor, unreal.PlayerStart):
            actor.set_actor_location(unreal.Vector(center.x - 600, center.y, ground_z + 92.5), False, True)
            actor.set_actor_rotation(unreal.Rotator(0, 0, 0), True)
    assert level.save_current_level()
    origin, bounds = dummy.get_actor_bounds(False)
    report.update(complete=True, location=str(position), bounds=str((origin, bounds)),
        mesh=str(dummy.get_editor_property("mesh").get_editor_property("skeletal_mesh_asset")),
        idle=str(dummy.get_editor_property("idle_animation")), hit=str(dummy.get_editor_property("hit_animation")),
        sound_count=len(dummy.get_editor_property("hit_sounds")))
except Exception:
    report["errors"].append(traceback.format_exc())
    unreal.log_error(report["errors"][-1])
out = project / "Saved/AssetValidation/TrainingDummy/placement.json"
out.parent.mkdir(parents=True, exist_ok=True)
out.write_text(json.dumps(report, ensure_ascii=False, indent=2), encoding="utf-8")
unreal.log("TRAINING_PLACEMENT " + str(report.get("complete", False)))
