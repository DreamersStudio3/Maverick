"""추출·변환 완료된 Lies of P 훈련용 허수아비의 Unreal 가져오기

UnrealEditor-Cmd Maverick.uproject -run=pythonscript -script=<본 파일>
원본 기본 경로: ../AssetReviews/LiesOfPTrainingDummy
원본 경로 재정의: MAVERICK_TRAINING_SOURCE 환경 변수
"""
import json
import math
import os
import traceback
from pathlib import Path

import unreal

PROJECT = Path(unreal.Paths.project_dir()).resolve()
SOURCE = Path(os.environ.get("MAVERICK_TRAINING_SOURCE", PROJECT.parent / "AssetReviews/LiesOfPTrainingDummy"))
REPORT = PROJECT / "Saved/AssetValidation/TrainingDummy/import.json"
DEST = "/Game/Characters/NPC/TrainingDummy"
lib = unreal.EditorAssetLibrary
tools = unreal.AssetToolsHelpers.get_asset_tools()
ml = unreal.MaterialEditingLibrary
report = {"engine": unreal.SystemLibrary.get_engine_version(), "animations": [], "sounds": [], "errors": []}


def checkpoint():
    REPORT.parent.mkdir(parents=True, exist_ok=True)
    REPORT.write_text(json.dumps(report, ensure_ascii=False, indent=2), encoding="utf-8")


def import_file(path, folder, name, options=None):
    existing = unreal.load_asset(folder + "/" + name) if lib.does_asset_exist(folder + "/" + name) else None
    if existing:
        return existing
    task = unreal.AssetImportTask()
    task.filename, task.destination_path, task.destination_name = str(path), folder, name
    task.automated, task.save, task.replace_existing = True, True, True
    if options:
        task.options, task.factory = options, unreal.FbxFactory()
    tools.import_asset_tasks([task])
    if not task.imported_object_paths:
        raise RuntimeError("Import failed: " + str(path))
    return unreal.load_asset(task.imported_object_paths[0])


try:
    world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
    unreal.SystemLibrary.execute_console_command(world, "Interchange.FeatureFlags.Import.FBX 0")
    data = json.loads((SOURCE / "Reports/blender-validation.json").read_text(encoding="utf-8"))
    opts = unreal.FbxImportUI()
    opts.automated_import_should_detect_type = False
    opts.import_as_skeletal, opts.import_mesh, opts.import_animations = True, True, False
    opts.mesh_type_to_import = unreal.FBXImportType.FBXIT_SKELETAL_MESH
    opts.import_materials, opts.import_textures, opts.create_physics_asset = False, False, False
    mesh = import_file(SOURCE / "Review/SK_TrainingDummy.fbx", DEST + "/Mesh", "SK_TrainingDummy", opts)
    skeleton = mesh.get_editor_property("skeleton")
    lib.save_loaded_asset(skeleton)
    report["mesh"], report["skeleton"] = mesh.get_path_name(), skeleton.get_path_name()
    textures = {}
    for source in sorted((SOURCE / "Converted/LiesofP/Content/ArtAsset/CH/MOB/Training/SK/Tex").glob("T_CH_MOB_Training_A02_*.png")):
        texture = import_file(source, DEST + "/Textures", source.stem)
        suffix = source.stem.rsplit("_", 1)[1]
        texture.set_editor_property("srgb", suffix == "BC")
        if suffix in ("N", "BN"):
            texture.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_NORMALMAP)
        elif suffix in ("ARM", "SMH"):
            texture.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_MASKS)
        lib.save_loaded_asset(texture)
        textures[suffix] = texture
    mat_path = DEST + "/Materials/M_TrainingDummy"
    material = unreal.load_asset(mat_path) if lib.does_asset_exist(mat_path) else tools.create_asset(
        "M_TrainingDummy", DEST + "/Materials", unreal.Material, unreal.MaterialFactoryNew())
    ml.delete_all_material_expressions(material)
    for index, suffix in enumerate(("BC", "N", "ARM")):
        node = ml.create_material_expression(material, unreal.MaterialExpressionTextureSample, -450, index * 250)
        node.texture = textures[suffix]
        node.sampler_type = {"BC": unreal.MaterialSamplerType.SAMPLERTYPE_COLOR,
                             "N": unreal.MaterialSamplerType.SAMPLERTYPE_NORMAL,
                             "ARM": unreal.MaterialSamplerType.SAMPLERTYPE_MASKS}[suffix]
        if suffix == "ARM":
            for channel, prop in (("R", unreal.MaterialProperty.MP_AMBIENT_OCCLUSION),
                                  ("G", unreal.MaterialProperty.MP_ROUGHNESS),
                                  ("B", unreal.MaterialProperty.MP_METALLIC)):
                ml.connect_material_property(node, channel, prop)
        else:
            ml.connect_material_property(node, "RGB", unreal.MaterialProperty.MP_BASE_COLOR if suffix == "BC" else unreal.MaterialProperty.MP_NORMAL)
    ml.recompile_material(material)
    lib.save_loaded_asset(material)
    slots = mesh.get_editor_property("materials")
    for slot in slots:
        slot.set_editor_property("material_interface", material)
    mesh.set_editor_property("materials", slots)
    lib.save_loaded_asset(mesh)
    animations = {}
    pose_options = unreal.AnimPoseEvaluationOptions()
    pose_options.optional_skeletal_mesh = mesh
    pose_options.evaluation_type = unreal.AnimDataEvalType.COMPRESSED
    for row in data["animations"]:
        if "name" not in row:
            continue
        opts = unreal.FbxImportUI()
        opts.automated_import_should_detect_type = False
        opts.import_mesh, opts.import_as_skeletal, opts.import_animations = False, True, True
        opts.mesh_type_to_import, opts.skeleton = unreal.FBXImportType.FBXIT_ANIMATION, skeleton
        opts.import_materials, opts.import_textures = False, False
        opts.anim_sequence_import_data.set_editor_property("custom_sample_rate", 60)
        opts.anim_sequence_import_data.set_editor_property("snap_to_closest_frame_boundary", True)
        anim = import_file(SOURCE / "Review/Animations" / (row["name"] + ".fbx"), DEST + "/Animations", row["name"], opts)
        length = anim.get_play_length()
        assert abs(length - row["duration"]) < 1 / 60 + .001
        assert anim.get_editor_property("skeleton") == skeleton
        samples = []
        for time in (0, length / 2, length):
            pose = unreal.AnimPoseExtensions.get_anim_pose_at_time(anim, time, pose_options)
            assert unreal.AnimPoseExtensions.is_valid(pose)
            names = unreal.AnimPoseExtensions.get_bone_names(pose)
            assert len(names) >= data["bones"]
            for name in names:
                tr = unreal.AnimPoseExtensions.get_bone_pose(pose, name, unreal.AnimPoseSpaces.WORLD)
                assert all(math.isfinite(value) for value in (tr.translation.x, tr.translation.y, tr.translation.z,
                    tr.rotation.x, tr.rotation.y, tr.rotation.z, tr.rotation.w, tr.scale3d.x, tr.scale3d.y, tr.scale3d.z))
            samples.append({"time": time, "bones": len(names)})
        animations[row["name"]] = anim
        report["animations"].append({"name": row["name"], "duration": length, "samples": samples})
        checkpoint()
    sounds = {}
    for source in sorted((SOURCE / "Review/Sounds").glob("*.wav")):
        sound = import_file(source, DEST + "/Sounds", source.stem)
        sounds[source.stem] = sound
        report["sounds"].append({"name": source.stem, "duration": sound.get_editor_property("duration")})
    bp_path = DEST + "/BP_TrainingDummy"
    if lib.does_asset_exist(bp_path):
        blueprint = unreal.load_asset(bp_path)
    else:
        factory = unreal.BlueprintFactory()
        factory.set_editor_property("parent_class", unreal.MVTrainingDummy)
        blueprint = tools.create_asset("BP_TrainingDummy", DEST, unreal.Blueprint, factory)
    default = unreal.get_default_object(blueprint.generated_class())
    default.get_editor_property("mesh").set_skeletal_mesh_asset(mesh)
    default.get_editor_property("mesh").override_animation_data(animations["AS_Idle_C"], True, True)
    default.set_editor_property("idle_animation", animations["AS_Idle_C"])
    default.set_editor_property("hit_animation", animations["AS_Additive_Endure"])
    default.set_editor_property("hit_sounds", [sounds["SE_NPC_Servant02_MT_Dmg_%02d" % index] for index in range(3)])
    unreal.BlueprintEditorLibrary.compile_blueprint(blueprint)
    lib.save_loaded_asset(blueprint)
    lib.save_directory(DEST, only_if_is_dirty=True, recursive=True)
    report["mesh_bounds"] = str(mesh.get_bounds())
    report["complete"] = True
except Exception:
    report["errors"].append(traceback.format_exc())
    unreal.log_error(report["errors"][-1])
checkpoint()
unreal.log("TRAINING_IMPORT " + str(report.get("complete", False)))
