# CsvToJsonConverter

`CsvToJsonConverter.py` reads `MaverickDesign/Csv/*.csv` and writes table JSON files to
`MaverickDesign/Json`.

Run from the project root:

```powershell
python MaverickDesign\CsvToJsonConverter\CsvToJsonConverter.py
```

The converter supports two layouts:

- CSV with a `Client` row in column A, a header row below it, and data rows after that.
- Plain CSV where the first non-empty row is the header and the first header column is the row key.

No third-party Python packages are required.

After conversion, `UMVTableAssetGenerator::GenerateDataTables` imports the
JSON files into `UDataTable` assets under `/Game/Table` and writes
`/Game/Table/DT_MVTableManifest`.

Runtime code loads the manifest and generated `UDataTable` assets through
`UMVTableManager`. `MaverickDesign/Json` is an editor-side intermediate output,
not the runtime data source.
