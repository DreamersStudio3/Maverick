import os
import unreal


TABLE_PATH = "/Game/Characters/NPC/Boss/TutorialBoss/DT_NewDataTable.DT_NewDataTable"
CSV_PATH = os.path.join(os.path.dirname(__file__), "TutorialBossAttackRows.csv")

data_table = unreal.load_asset(TABLE_PATH)
if not data_table:
    raise RuntimeError("DataTable load failed: %s" % TABLE_PATH)

if not unreal.DataTableFunctionLibrary.fill_data_table_from_csv_file(data_table, CSV_PATH):
    raise RuntimeError("DataTable CSV import failed: %s" % CSV_PATH)

if not unreal.EditorAssetLibrary.save_loaded_asset(data_table, only_if_is_dirty=False):
    raise RuntimeError("DataTable save failed: %s" % TABLE_PATH)

unreal.log("TutorialBoss DataTable populated and saved: %s" % TABLE_PATH)
