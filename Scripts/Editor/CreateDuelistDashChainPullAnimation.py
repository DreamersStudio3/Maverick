"""기존 Duelist 동작으로 3초 돌진·사슬 공격 에셋 생성

Unreal Editor Python: py D:/Project/Maverick/Scripts/Editor/CreateDuelistDashChainPullAnimation.py
생성 대상 2개만 갱신, 기존 _2s 경로·원본 동작·현재 공격 DataTable 보존
"""

import json
from pathlib import Path
import unreal


def create_duelist_dash_chain_pull():
    root = '/Game/DuelistComplete/Animations'
    sequence_root = root + '/AnimationsSequence'
    sequence_path = sequence_root + '/AS_Duelist_DashChainPull_2s'
    montage_path = root + '/AM_Duelist_DashChainPull_2s'
    mesh = unreal.load_asset('/Game/DuelistComplete/Mesh/SK_Duelist_TwinHammers_NoCoat')
    idle = unreal.load_asset(sequence_root + '/AS_Duelist_UpIdle')
    run = unreal.load_asset(sequence_root + '/AS_Duelist_Move_Run_Fwd')
    attack = unreal.load_asset(sequence_root + '/AS_Duelist_W')
    assert mesh and idle and run and attack
    options = unreal.AnimPoseEvaluationOptions()
    options.set_editor_property('evaluation_type', unreal.AnimDataEvalType.RAW)
    options.set_editor_property('should_retarget', False)
    options.set_editor_property('optional_skeletal_mesh', mesh)
    idle_pose = unreal.AnimPoseExtensions.get_anim_pose_at_time(idle, 0.0, options)
    bones = list(unreal.AnimationLibrary.get_animation_track_names(idle))

    def smooth(value):
        value = max(0.0, min(1.0, value))
        return value * value * (3.0 - 2.0 * value)

    # 첫 0.5초를 1.5초로 연장, 후속 동작은 1초 이동·기존 속도 유지
    # 발사 원본 시점 2.5190904초 → 새 동작 1.85초
    def source_poses(time):
        time = time / 3.0 if time <= 1.5 else time - 1.0
        if time <= 0.55:
            run_time = 0.12 + time * 0.95
            run_pose = unreal.AnimPoseExtensions.get_anim_pose_at_time(run, run_time, options)
            if time < 0.12:
                return idle_pose, run_pose, smooth(time / 0.12)
            if time < 0.35:
                return run_pose, run_pose, 0.0
            attack_time = 1.95 + (time - 0.35) / 0.20 * 0.17
            attack_pose = unreal.AnimPoseExtensions.get_anim_pose_at_time(attack, attack_time, options)
            return run_pose, attack_pose, smooth((time - 0.35) / 0.20)
        if time <= 0.85:
            source_time = 2.12 + (time - 0.55) / 0.30 * (2.5190904 - 2.12)
        elif time <= 1.55:
            source_time = 2.5190904 + (time - 0.85) / 0.70 * (4.0 - 2.5190904)
        else:
            source_time = 4.0 + (time - 1.55) / 0.45 * (attack.get_play_length() - 4.0)
        attack_pose = unreal.AnimPoseExtensions.get_anim_pose_at_time(attack, source_time, options)
        return attack_pose, idle_pose, smooth((time - 1.60) / 0.35)

    # 로컬 뼈 자세 보간 후 60 fps 키 저장, 이동 루트는 220 cm 전진·정지 유지
    positions = {str(b): [] for b in bones}
    rotations = {str(b): [] for b in bones}
    scales = {str(b): [] for b in bones}
    for frame in range(181):
        time = frame / 60.0
        first, second, weight = source_poses(time)
        for bone in bones:
            name = str(bone)
            a = unreal.AnimPoseExtensions.get_bone_pose(first, bone, unreal.AnimPoseSpaces.LOCAL)
            b = unreal.AnimPoseExtensions.get_bone_pose(second, bone, unreal.AnimPoseSpaces.LOCAL)
            transform = a.lerp(b, weight)
            if name.lower() == 'c3400-armature':
                root_reference = unreal.AnimPoseExtensions.get_bone_pose(idle_pose, bone, unreal.AnimPoseSpaces.LOCAL)
                transform = root_reference
                transform.translation = unreal.Vector(0.0, 220.0 * smooth(time / 1.50), 0.0)
            positions[name].append(transform.translation)
            rotations[name].append(transform.rotation)
            scales[name].append(transform.scale3d)
    # 압축된 회수 구간의 급격한 회전 완화, 시작·끝 자세와 이동 루트 보존
    weights = [1, 2, 3, 4, 3, 2, 1]
    for bone in bones:
        name = str(bone)
        if name.lower() == 'c3400-armature':
            continue
        original_positions = list(positions[name])
        original_rotations = list(rotations[name])
        for frame in range(1, 180):
            p = unreal.Vector()
            q = original_rotations[max(0, frame - 3)]
            total = 0.0
            for offset, weight in zip(range(-3, 4), weights):
                index = max(0, min(180, frame + offset))
                p = p + original_positions[index] * (weight / 16.0)
                total += weight
                q = q.slerp_quat(original_rotations[index], weight / total)
            positions[name][frame] = p
            rotations[name][frame] = q
    sequence = unreal.load_asset(sequence_path) if unreal.EditorAssetLibrary.does_asset_exist(sequence_path) else unreal.EditorAssetLibrary.duplicate_asset(idle.get_path_name(), sequence_path)
    assert sequence
    unreal.AnimationLibrary.remove_all_animation_notify_tracks(sequence)
    target_rate = sequence.get_editor_property('platform_target_frame_rate')
    target_rate.set_editor_property('default', unreal.FrameRate(60, 1))
    controller = sequence.get_editor_property('controller')
    controller.open_bracket('Duelist 3 second dash chain pull', False)
    try:
        controller.set_frame_rate(unreal.FrameRate(60, 1), False)
        controller.set_number_of_frames(unreal.FrameNumber(180), False)
        for bone in bones:
            name = str(bone)
            assert controller.set_bone_track_keys(bone, positions[name], rotations[name], scales[name], False), name
    finally:
        controller.close_bracket(False)
    sequence.set_editor_property('rate_scale', 1.0)
    sequence.set_editor_property('enable_root_motion', True)
    sequence.set_editor_property('root_motion_root_lock', unreal.RootMotionRootLock.ANIM_FIRST_FRAME)
    sequence.set_editor_property('force_root_lock', False)
    sequence.set_editor_property('use_normalized_root_motion_scale', False)
    assert unreal.EditorAssetLibrary.save_loaded_asset(sequence, False)

    if unreal.EditorAssetLibrary.does_asset_exist(montage_path):
        montage = unreal.load_asset(montage_path)
    else:
        factory = unreal.AnimMontageFactory()
        factory.set_editor_property('source_animation', sequence)
        montage = unreal.AssetToolsHelpers.get_asset_tools().create_asset('AM_Duelist_DashChainPull_2s', root, unreal.AnimMontage, factory)
    assert montage
    tracks = json.loads(unreal.ToolsetLibrary.get_object_properties(montage, ['SlotAnimTracks']))
    segment = tracks['SlotAnimTracks'][0]['animTrack']['animSegments'][0]
    segment['animEndTime'] = 3.0
    segment['cachedPlayLength'] = 3.0
    assert unreal.ToolsetLibrary.set_object_properties(montage, json.dumps(tracks))
    unreal.AnimationLibrary.remove_all_animation_notify_tracks(montage)
    # 종료 Notify의 상대 링크 생성 전에 몽타주 내부 길이 갱신
    assert unreal.EditorAssetLibrary.save_loaded_asset(montage, False)
    reloaded, message = unreal.EditorLoadingAndSavingUtils.reload_packages([montage.get_outermost()])
    assert reloaded, str(message)
    montage = unreal.load_asset(montage_path)
    assert montage.get_play_length() == 3.0
    unreal.AnimationLibrary.add_animation_notify_track(montage, 'RootMotionNormalize')
    root_state = unreal.AnimationLibrary.add_animation_notify_state_event(
        montage, 'RootMotionNormalize', 0.0, 3.0, unreal.AnimNotifyState_MotionWarping)
    root_modifier = unreal.new_object(unreal.RootMotionModifier_Scale, outer=root_state)
    root_modifier.set_editor_property('scale', unreal.Vector(1, 1, 1))
    root_state.set_editor_property('root_motion_modifier', root_modifier)
    unreal.AnimationLibrary.add_animation_notify_track(montage, 'ChainAbility')
    ability_state = unreal.AnimationLibrary.add_animation_notify_state_event(
        montage, 'ChainAbility', 0.001, 2.998, unreal.load_class(None, '/Script/Maverick.MVAnimNotifyState_Ability'))
    ability_state.set_editor_property('ability_class', unreal.load_class(None, '/Script/Maverick.MVDuelistChainPullAbility'))
    ability_state.set_editor_property('ability_index', 0)
    unreal.AnimationLibrary.add_animation_notify_track(montage, 'ChainFire')
    unreal.AnimationLibrary.add_animation_notify_event(
        montage, 'ChainFire', 1.85, unreal.load_class(None, '/Script/Maverick.MVAnimNotify_DuelistFireChain'))
    assert unreal.ToolsetLibrary.set_object_properties(montage, json.dumps({
        'BlendIn': {'blendTime': 0.08, 'blendOption': 'HermiteCubic'},
        'BlendOut': {'blendTime': 0.12, 'blendOption': 'HermiteCubic'}, 'RateScale': 1.0}))
    assert unreal.EditorAssetLibrary.save_loaded_asset(montage, False)
    reloaded, message = unreal.EditorLoadingAndSavingUtils.reload_packages([montage.get_outermost()])
    assert reloaded, str(message)
    montage = unreal.load_asset(montage_path)
    assert montage.get_play_length() == 3.0
    states = {str(event.get_editor_property('notify_name')): unreal.AnimationLibrary.get_anim_notify_event_duration(event)
              for event in unreal.AnimationLibrary.get_animation_notify_events(montage)}
    assert abs(states['MotionWarping'] - 3.0) < 0.0001
    assert abs(states['Activate Ability'] - 2.998) < 0.0001
    report = {'sequence': sequence.get_path_name(), 'montage': montage.get_path_name(),
              'duration_seconds': sequence.get_play_length(), 'sample_rate': 60, 'frames': 180, 'keys_per_bone': 181,
              'bone_tracks': len(bones), 'dash_cm': 220, 'fire_time_seconds': 1.85,
              'sources': [idle.get_path_name(), run.get_path_name(), attack.get_path_name()],
              'phases': {'dash': [0.0, 1.5], 'fire': [1.5, 1.85], 'retrieve': [1.85, 2.6], 'idle_return': [2.6, 3.0]},
              'data_table_changed': False}
    out = Path(unreal.Paths.project_saved_dir()) / 'DuelistChainDuration'
    out.mkdir(parents=True, exist_ok=True)
    (out / 'created.json').write_text(json.dumps(report, indent=2), encoding='utf-8')
    unreal.log('DuelistChainDuration: 3 second sequence and montage created')


create_duelist_dash_chain_pull()
