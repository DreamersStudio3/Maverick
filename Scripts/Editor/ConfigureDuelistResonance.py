"""Duelist 공명 폭주 연출 에셋·BP 컴포넌트 구성, 공격 트리 연결은 MaverickBossStateTree -ResonanceOnly 경유"""
import json
from pathlib import Path
import unreal

def configure_duelist_resonance():
    assets = unreal.AssetToolsHelpers.get_asset_tools()
    root = '/Game/DuelistComplete/Resonance'
    def material(name, color, opacity):
        path = root + '/' + name
        mat = unreal.load_asset(path) if unreal.EditorAssetLibrary.does_asset_exist(path) else assets.create_asset(name, root, unreal.Material, unreal.MaterialFactoryNew())
        unreal.MaterialEditingLibrary.delete_all_material_expressions(mat)
        mat.set_editor_property('blend_mode', unreal.BlendMode.BLEND_TRANSLUCENT)
        mat.set_editor_property('shading_model', unreal.MaterialShadingModel.MSM_UNLIT)
        mat.set_editor_property('two_sided', True)
        emissive = unreal.MaterialEditingLibrary.create_material_expression(mat, unreal.MaterialExpressionConstant3Vector, -300, 0)
        emissive.set_editor_property('constant', unreal.LinearColor(*color))
        alpha = unreal.MaterialEditingLibrary.create_material_expression(mat, unreal.MaterialExpressionConstant, -300, 180)
        alpha.set_editor_property('r', opacity)
        unreal.MaterialEditingLibrary.connect_material_property(emissive, '', unreal.MaterialProperty.MP_EMISSIVE_COLOR)
        unreal.MaterialEditingLibrary.connect_material_property(alpha, '', unreal.MaterialProperty.MP_OPACITY)
        unreal.MaterialEditingLibrary.recompile_material(mat)
        assert unreal.EditorAssetLibrary.save_loaded_asset(mat, False)
        return mat
    warning = material('M_ResonanceWarning', (2.5, 0.015, 0.035, 1), 0.22)
    energy = material('M_ResonanceToxicEnergy', (0.35, 6, 0.8, 1), 0.6)
    sequences = '/Game/DuelistComplete/Animations/AnimationsSequence/'
    def montage(name, source_name, duration):
        sequence = unreal.load_asset(sequences + source_name)
        assert sequence, source_name
        path = root + '/' + name
        if unreal.EditorAssetLibrary.does_asset_exist(path):
            result = unreal.load_asset(path)
        else:
            factory = unreal.AnimMontageFactory()
            factory.set_editor_property('source_animation', sequence)
            result = assets.create_asset(name, root, unreal.AnimMontage, factory)
        unreal.AnimationLibrary.remove_all_animation_notify_tracks(result)
        properties = json.loads(unreal.ToolsetLibrary.get_object_properties(result, ['SlotAnimTracks']))
        segment = properties['SlotAnimTracks'][0]['animTrack']['animSegments'][0]
        segment['animPlayRate'] = sequence.get_play_length() / duration
        segment['animEndTime'] = sequence.get_play_length()
        segment['cachedPlayLength'] = sequence.get_play_length()
        assert unreal.ToolsetLibrary.set_object_properties(result, json.dumps(properties))
        assert unreal.ToolsetLibrary.set_object_properties(result, json.dumps({'bEnableAutoBlendOut': False, 'BlendIn': {'blendTime': 0.15}, 'BlendOut': {'blendTime': 0.15}}))
        assert unreal.EditorAssetLibrary.save_loaded_asset(result, False)
        return result
    jump = montage('AM_Duelist_ResonanceJump', 'AS_Duelist_Evasion', 0.75)
    charge = montage('AM_Duelist_ResonanceChannel', 'AS_Duelist_PazeChange', 5)
    groggy = montage('AM_Duelist_ResonanceGroggy', 'AS_Duelist_DownIdle', 10)
    bp = unreal.load_asset('/Game/DuelistComplete/BP_Duelist')
    system = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
    handles = system.k2_gather_subobject_data_for_blueprint(bp)
    lib = unreal.SubobjectDataBlueprintFunctionLibrary
    components = [lib.get_associated_object(lib.get_data(handle)) for handle in handles]
    component_class = unreal.load_class(None, '/Script/Maverick.MVDuelistResonanceComponent')
    component = next((obj for obj in components if isinstance(obj, unreal.MVDuelistResonanceComponent)), None)
    if not component:
        params = unreal.AddNewSubobjectParams(parent_handle=handles[0], new_class=component_class, blueprint_context=bp)
        handle, reason = system.add_new_subobject(params)
        assert not str(reason), str(reason)
        system.rename_subobject(handle, unreal.Text('Resonance'))
        component = lib.get_associated_object(lib.get_data(handle))
    component.set_editor_property('arena_center', unreal.Vector(0, 0, 0))
    component.set_editor_property('jump_montage', jump)
    component.set_editor_property('channel_montage', charge)
    component.set_editor_property('groggy_montage', groggy)
    component.set_editor_property('warning_material', warning)
    component.set_editor_property('energy_material', energy)
    unreal.BlueprintEditorLibrary.compile_blueprint(bp)
    assert unreal.EditorAssetLibrary.save_loaded_asset(bp, False)
    report = {'component': component.get_path_name(), 'center': [0, 0, 0], 'channel_seconds': 5, 'poise': 40,
              'max_hp_damage_ratio': 0.8, 'blackout_seconds': 10, 'arena_radius_cm': 4000,
              'presentation_assets': [asset.get_path_name() for asset in [jump, charge, groggy, warning, energy]]}
    Path('D:/Project/Maverick/Saved/DuelistResonance/assets.json').write_text(json.dumps(report, indent=2), encoding='utf-8')
    unreal.log('[ResonanceSetup] BP and presentation assets saved')

configure_duelist_resonance()
