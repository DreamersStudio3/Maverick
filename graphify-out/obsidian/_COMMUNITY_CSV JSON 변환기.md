---
type: community
cohesion: 0.31
members: 16
---

# CSV JSON 변환기

**Cohesion:** 0.31 - loosely connected
**Members:** 16 nodes

## Members
- [[Any]] - code
- [[CsvToJsonConverter.py]] - code - MaverickDesign/CsvToJsonConverter/CsvToJsonConverter.py
- [[Path]] - code
- [[build_table_document()]] - code - MaverickDesign/CsvToJsonConverter/CsvToJsonConverter.py
- [[convert_all()]] - code - MaverickDesign/CsvToJsonConverter/CsvToJsonConverter.py
- [[convert_client_layout()]] - code - MaverickDesign/CsvToJsonConverter/CsvToJsonConverter.py
- [[convert_plain_layout()]] - code - MaverickDesign/CsvToJsonConverter/CsvToJsonConverter.py
- [[convert_rows()]] - code - MaverickDesign/CsvToJsonConverter/CsvToJsonConverter.py
- [[find_client_row()]] - code - MaverickDesign/CsvToJsonConverter/CsvToJsonConverter.py
- [[get_base_dir()]] - code - MaverickDesign/CsvToJsonConverter/CsvToJsonConverter.py
- [[is_empty_row()]] - code - MaverickDesign/CsvToJsonConverter/CsvToJsonConverter.py
- [[main()]] - code - MaverickDesign/CsvToJsonConverter/CsvToJsonConverter.py
- [[normalize_header()]] - code - MaverickDesign/CsvToJsonConverter/CsvToJsonConverter.py
- [[parse_value()]] - code - MaverickDesign/CsvToJsonConverter/CsvToJsonConverter.py
- [[read_csv()]] - code - MaverickDesign/CsvToJsonConverter/CsvToJsonConverter.py
- [[to_int_flag()]] - code - MaverickDesign/CsvToJsonConverter/CsvToJsonConverter.py

## Live Query (requires Dataview plugin)

```dataview
TABLE source_file, type FROM #community/CSV_JSON_
SORT file.name ASC
```
