"""Duelist 최초 등장 예제 구성

Unreal Editor: py D:/Project/Maverick/Scripts/Editor/ConfigureDuelistIntro.py
현재 BaseBossTestLevel의 기존 트리거 재사용, 기존 등장 시퀀스의 재생 트랙 보존
"""
import json
from pathlib import Path
import unreal


def configure_duelist_intro():
    actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
    if world.get_path_name() != '/Game/Characters/NPC/BaseBoss/Level/BaseBossTestLevel.BaseBossTestLevel':
        raise RuntimeError('BaseBossTestLevel에서 실행 필요')
    bosses = [a for a in actors.get_all_level_actors() if 'BP_Duelist_C' in a.get_class().get_path_name()]
    if len(bosses) != 1:
        raise RuntimeError('현재 레벨의 Duelist 보스 1개 필요')
    boss = bosses[0]
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    sequence_path = '/Game/DuelistComplete/Intro/LS_Duelist_Intro'
    blueprint_path = '/Game/DuelistComplete/BP_BossIntroTrigger'
    start = boss.get_actor_location()
    end = start + unreal.Vector(500, 0, 0)

    def transform_track(proxy, keyframes):
        track = proxy.add_track(unreal.MovieScene3DTransformTrack)
        section = track.add_section()
        section.set_range(0, 180)
        section.set_completion_mode(unreal.MovieSceneCompletionMode.RESTORE_STATE)
        channels = section.get_all_channels()
        for index, channel in enumerate(channels):
            channel.set_default(1.0 if index >= 6 else 0.0)
        for frame, location, orientation in keyframes:
            values = [location.x, location.y, location.z, orientation.roll, orientation.pitch, orientation.yaw]
            for index, value in enumerate(values):
                channels[index].add_key(unreal.FrameNumber(frame), value, interpolation=unreal.MovieSceneKeyInterpolation.LINEAR)
        return section

    sequence = unreal.load_asset(sequence_path)
    if not sequence:
        sequence = asset_tools.create_asset('LS_Duelist_Intro', '/Game/DuelistComplete/Intro', unreal.LevelSequence, unreal.LevelSequenceFactoryNew())
        sequence.set_display_rate(unreal.FrameRate(30, 1))
        sequence.set_playback_start(0)
        sequence.set_playback_end(180)
        binding = sequence.add_possessable(boss)
        binding.set_name('Boss')
        unreal.MovieSceneBindingTagExtensions.tag_binding(binding, 'Boss')
        rotation = boss.get_actor_rotation()
        transform_track(binding, [(0, start, rotation), (120, end, rotation), (179, end, rotation)])
        mesh_binding = sequence.add_possessable(boss.mesh)
        mesh_binding.set_parent(binding)
        animation_track = mesh_binding.add_track(unreal.MovieSceneSkeletalAnimationTrack)
        animation_section = animation_track.add_section()
        animation_section.set_range(0, 180)
        animation_section.set_completion_mode(unreal.MovieSceneCompletionMode.RESTORE_STATE)
        animation = unreal.load_asset('/Game/DuelistComplete/Animations/AnimationsSequence/AS_Duelist_PazeChange')
        params = animation_section.get_editor_property('params')
        params.set_editor_property('animation', animation)
        animation_section.set_editor_property('params', params)
    if not any(isinstance(t, unreal.MovieSceneCameraCutTrack) for t in sequence.get_tracks()):
        camera = actors.spawn_actor_from_class(unreal.CineCameraActor, start + unreal.Vector(-450, -600, 350))
        try:
            camera.get_cine_camera_component().set_editor_property('current_focal_length', 32.0)
            camera_binding = sequence.add_spawnable_from_instance(camera)
            camera_binding.set_name('Intro Camera')
            camera_start = start + unreal.Vector(-450, -600, 350)
            camera_end = end + unreal.Vector(-500, -500, 300)
            look_start = unreal.MathLibrary.find_look_at_rotation(camera_start, start + unreal.Vector(0, 0, 130))
            look_end = unreal.MathLibrary.find_look_at_rotation(camera_end, end + unreal.Vector(0, 0, 130))
            transform_track(camera_binding, [(0, camera_start, look_start), (120, camera_end, look_end), (179, camera_end, look_end)])
            cut_track = sequence.add_track(unreal.MovieSceneCameraCutTrack)
            cut_section = cut_track.add_section()
            cut_section.set_range(0, 180)
            camera_id = unreal.MovieSceneObjectBindingID()
            camera_id.set_editor_property('guid', camera_binding.get_id())
            cut_section.set_camera_binding_id(camera_id)
        finally:
            actors.destroy_actor(camera)
    unreal.EditorAssetLibrary.save_loaded_asset(sequence)
    blueprint = unreal.load_asset(blueprint_path)
    if not blueprint:
        factory = unreal.BlueprintFactory()
        factory.set_editor_property('parent_class', unreal.load_class(None, '/Script/Maverick.MVBossIntroTrigger'))
        blueprint = asset_tools.create_asset('BP_BossIntroTrigger', '/Game/DuelistComplete', unreal.Blueprint, factory)
        unreal.EditorAssetLibrary.save_loaded_asset(blueprint)
    trigger_class = unreal.load_class(None, blueprint_path + '.BP_BossIntroTrigger_C')
    existing = [a for a in actors.get_all_level_actors() if a.get_class() == trigger_class]
    trigger = existing[0] if existing else actors.spawn_actor_from_class(trigger_class, unreal.Vector(220, -60, 130))
    if not existing:
        trigger.set_actor_label('BP_BossIntroTrigger')
    trigger.set_editor_property('Boss', boss)
    trigger.set_editor_property('IntroSequence', sequence)
    trigger.set_editor_property('IntroId', 'Duelist_001')
    unreal.EditorLoadingAndSavingUtils.save_map(world, '/Game/Characters/NPC/BaseBoss/Level/BaseBossTestLevel')
    result = {'sequence': sequence.get_path_name(), 'trigger': trigger.get_path_name(), 'blueprint': blueprint.get_path_name(), 'intro_id': 'Duelist_001', 'boss': boss.get_path_name()}
    Path('D:/Project/Maverick/Saved/BossIntro/assets.json').write_text(json.dumps(result, indent=2), encoding='utf-8')
    actors.set_selected_level_actors([trigger])
    print(json.dumps(result))


configure_duelist_intro()
