"""TutorialBoss 기본 무기 데이터 연결; 그래프·몽타주·기존 외형 수정 제외"""
import unreal

BOSS_PATH = '/Game/Characters/NPC/Boss/TutorialBoss/BP_TutorialBoss'
WEAPON_PATH = '/Game/Table/Weapons/NPC/E1/DT_Weapon_E1'
ROW_NAME = 'E1_Weapon_R'

blueprint = unreal.load_asset(BOSS_PATH)
weapon_table = unreal.load_asset(WEAPON_PATH)
if not blueprint or not weapon_table:
    raise RuntimeError('TutorialBoss or E1 weapon table load failed')
if ROW_NAME not in [str(name) for name in unreal.DataTableFunctionLibrary.get_data_table_row_names(weapon_table)]:
    raise RuntimeError('E1_Weapon_R row missing')
owner = unreal.get_default_object(blueprint.generated_class())
weapon = owner.get_component_by_class(unreal.MVWeaponComponent)
if not weapon:
    raise RuntimeError('TutorialBoss WeaponComponent missing')

if '-VerifyOnly' not in unreal.SystemLibrary.get_command_line():
    weapon.set_editor_property('default_weapon_row', unreal.DataTableRowHandle(data_table=weapon_table, row_name=ROW_NAME))
    # 기존 Blueprint 부착 무기 외형 유지, 런타임 무기 메시 중복 생성 방지
    weapon.set_editor_property('manage_weapon_mesh', False)
    if not unreal.EditorAssetLibrary.save_asset(BOSS_PATH, only_if_is_dirty=False):
        raise RuntimeError('TutorialBoss weapon settings save failed')

row = weapon.get_editor_property('default_weapon_row')
if row.get_editor_property('data_table') != weapon_table or str(row.get_editor_property('row_name')) != ROW_NAME:
    raise RuntimeError('TutorialBoss default weapon readback mismatch')
if weapon.get_editor_property('manage_weapon_mesh'):
    raise RuntimeError('TutorialBoss existing weapon visual preservation mismatch')
unreal.log('[BossBasicWeaponSetup] PASS DefaultWeapon=DT_Weapon_E1/E1_Weapon_R ManageWeaponMesh=False')
