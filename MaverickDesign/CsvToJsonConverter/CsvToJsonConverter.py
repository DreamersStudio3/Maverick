# -*- coding: utf-8 -*-
"""
Maverick CSV -> JSON table converter.

Input:
  MaverickDesign/Csv/**/*.csv

Output:
  MaverickDesign/Json/<RelativeCsvPathWithoutExt>.json
  MaverickDesign/Json/SheetRecipe.json

Supported CSV layouts:
  1) Client-flag layout
     - Column A is a label column.
     - A row with "Client" in column A controls included columns.
     - The row after "Client" contains field names from column B.
     - Data starts on the next row.
     - The first included field is used as the row key.

  2) Plain CSV layout
     - The first non-empty row is the header.
     - All non-empty header columns are included.
     - The first header field is used as the row key.
"""

import argparse
import csv
import json
import re
import sys
from pathlib import Path
from typing import Any, Dict, Iterable, List, Optional, Tuple


INT_RE = re.compile(r"^[+-]?(0|[1-9][0-9]*)$")
FLOAT_RE = re.compile(
    r"^[+-]?((0|[1-9][0-9]*)\.[0-9]+|[0-9]+[eE][+-]?[0-9]+|((0|[1-9][0-9]*)\.[0-9]+)[eE][+-]?[0-9]+)$"
)


def get_base_dir() -> Path:
    if getattr(sys, "frozen", False):
        return Path(sys.executable).resolve().parent.parent
    return Path(__file__).resolve().parent.parent


def read_csv(path: Path) -> List[List[str]]:
    encodings = ("utf-8-sig", "utf-8", "cp949")
    last_error: Optional[UnicodeDecodeError] = None

    for encoding in encodings:
        try:
            with path.open("r", encoding=encoding, newline="") as f:
                return [row for row in csv.reader(f)]
        except UnicodeDecodeError as exc:
            last_error = exc

    if last_error:
        raise last_error
    return []


def to_int_flag(value: str) -> bool:
    try:
        return int(str(value).strip()) >= 1
    except (TypeError, ValueError):
        return False


def parse_value(value: str) -> Any:
    text = value.strip()
    if text == "":
        return None

    lower = text.lower()
    if lower == "true":
        return True
    if lower == "false":
        return False
    if lower == "null":
        return None

    if INT_RE.match(text):
        try:
            return int(text)
        except ValueError:
            pass

    if FLOAT_RE.match(text):
        try:
            return float(text)
        except ValueError:
            pass

    if text[0] in "[{\"":
        try:
            return json.loads(text)
        except json.JSONDecodeError:
            pass

    return value


def is_empty_row(row: Iterable[str]) -> bool:
    return all(str(cell).strip() == "" for cell in row)


def find_client_row(rows: List[List[str]]) -> Optional[int]:
    for index, row in enumerate(rows):
        if row and row[0].strip() == "Client":
            return index
    return None


def normalize_header(value: str) -> Optional[str]:
    text = str(value).strip()
    return text if text else None


def convert_client_layout(rows: List[List[str]]) -> Tuple[List[Dict[str, Any]], Optional[str]]:
    client_row_index = find_client_row(rows)
    if client_row_index is None:
        return [], None

    header_row_index = client_row_index + 1
    data_start_index = header_row_index + 1
    if header_row_index >= len(rows):
        return [], None

    flags = [to_int_flag(value) for value in rows[client_row_index][1:]]
    headers = [normalize_header(value) for value in rows[header_row_index][1:]]

    key_column = next((name for use, name in zip(flags, headers) if use and name), None)
    output_rows: List[Dict[str, Any]] = []

    for raw in rows[data_start_index:]:
        if is_empty_row(raw):
            continue

        record: Dict[str, Any] = {}
        cells = raw[1:]
        for index, (use, name) in enumerate(zip(flags, headers)):
            if not use or not name:
                continue
            raw_value = cells[index] if index < len(cells) else ""
            value = parse_value(raw_value)
            if value is None:
                continue
            record[name] = value

        if record:
            output_rows.append(record)

    return output_rows, key_column


def convert_plain_layout(rows: List[List[str]]) -> Tuple[List[Dict[str, Any]], Optional[str]]:
    header_row_index = next((index for index, row in enumerate(rows) if not is_empty_row(row)), None)
    if header_row_index is None:
        return [], None

    headers = [normalize_header(value) for value in rows[header_row_index]]
    key_column = next((name for name in headers if name), None)
    output_rows: List[Dict[str, Any]] = []

    for raw in rows[header_row_index + 1:]:
        if is_empty_row(raw):
            continue

        record: Dict[str, Any] = {}
        for index, name in enumerate(headers):
            if not name:
                continue
            raw_value = raw[index] if index < len(raw) else ""
            value = parse_value(raw_value)
            if value is None:
                continue
            record[name] = value

        if record:
            output_rows.append(record)

    return output_rows, key_column


def convert_rows(rows: List[List[str]]) -> Tuple[List[Dict[str, Any]], Optional[str]]:
    if find_client_row(rows) is not None:
        return convert_client_layout(rows)
    return convert_plain_layout(rows)


def build_table_document(table_name: str, key_column: str, rows: List[Dict[str, Any]], source_file: str) -> Dict[str, Any]:
    return {
        "version": 1,
        "source": {
            "converter": "CsvToJsonConverter",
            "file": source_file,
        },
        "tables": {
            table_name: {
                "key": key_column,
                "rows": rows,
            }
        },
    }


def convert_all(input_dir: Path, output_dir: Path) -> int:
    input_dir.mkdir(parents=True, exist_ok=True)
    output_dir.mkdir(parents=True, exist_ok=True)

    csv_files = sorted(path for path in input_dir.rglob("*.csv") if not path.name.startswith("~$"))
    if not csv_files:
        print(f"[MVConverter] No .csv files found: {input_dir}")
        return 0

    recipe: Dict[str, str] = {}

    for csv_path in csv_files:
        relative_csv_path = csv_path.relative_to(input_dir)
        print(f"[MVConverter] Processing {relative_csv_path.as_posix()}")
        rows = read_csv(csv_path)
        table_rows, key_column = convert_rows(rows)

        if not table_rows:
            print(f"[MVConverter]   Empty output. Skipped.")
            continue
        if not key_column:
            print(f"[MVConverter]   No key column. Skipped.", file=sys.stderr)
            continue

        table_name = csv_path.stem
        document = build_table_document(table_name, key_column, table_rows, relative_csv_path.as_posix())

        output_path = output_dir / relative_csv_path.with_suffix(".json")
        output_path.parent.mkdir(parents=True, exist_ok=True)
        with output_path.open("w", encoding="utf-8", newline="\n") as f:
            json.dump(document, f, ensure_ascii=False, indent=4)

        recipe[table_name] = key_column
        print(f"[MVConverter]   Saved {output_path} ({len(table_rows)} rows, key=\"{key_column}\").")

    recipe_path = output_dir / "SheetRecipe.json"
    with recipe_path.open("w", encoding="utf-8", newline="\n") as f:
        json.dump(recipe, f, ensure_ascii=False, indent=4)
    print(f"[MVConverter] Saved recipe {recipe_path} ({len(recipe)} tables).")

    return 0


def main() -> int:
    base_dir = get_base_dir()
    parser = argparse.ArgumentParser(description="Convert MaverickDesign/Csv/*.csv to MaverickDesign/Json/*.json.")
    parser.add_argument("--input", type=Path, default=base_dir / "Csv", help="CSV input directory.")
    parser.add_argument("--output", type=Path, default=base_dir / "Json", help="JSON output directory.")
    args = parser.parse_args()

    return convert_all(args.input.resolve(), args.output.resolve())


if __name__ == "__main__":
    sys.exit(main())
