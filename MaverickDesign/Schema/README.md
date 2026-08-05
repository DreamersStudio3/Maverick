# 테이블 JSON 스키마

이 폴더는 `MaverickDesign/Json`에 생성되는 테이블 JSON의 공통 형식을 설명합니다.

JSON은 런타임이 직접 읽는 최종 데이터가 아니라, 에디터에서 `UDataTable`을 생성하기 위한 중간 산출물입니다.

## 기본 구조

CSV 파일 하나는 보통 JSON 파일 하나로 변환됩니다.

예: `Item.csv` -> `Item.json`

```json
{
    "version": 1,
    "source": {
        "converter": "CsvToJsonConverter",
        "file": "Item.csv"
    },
    "tables": {
        "Item": {
            "key": "ItemId",
            "rows": [
                {
                    "ItemId": 1001,
                    "Name": "Potion",
                    "Heal": 50
                }
            ]
        }
    }
}
```

## 필드 설명

- `version`: JSON 스키마 버전입니다. 현재 값은 `1`입니다.
- `source`: 변환기와 원본 파일 정보를 담는 선택 필드입니다.
- `tables`: 테이블 이름과 테이블 데이터를 매핑하는 객체입니다.
- `tables.<TableName>.key`: 행을 식별하는 key 컬럼명입니다.
- `tables.<TableName>.rows`: 실제 데이터 행 배열입니다.

`rows` 안의 각 행은 JSON object여야 합니다. 필드 값은 문자열, 숫자, 불리언, 배열, 객체, `null`을 사용할 수 있습니다.

## SheetRecipe.json

CSV 변환기는 `MaverickDesign/Json/SheetRecipe.json`도 함께 생성합니다.

```json
{
    "Item": "ItemId",
    "Skill": "SkillId"
}
```

이 파일은 테이블 이름과 key 컬럼명을 빠르게 찾기 위한 보조 파일입니다. DataTable 생성기는 이 정보를 사용해서 테이블별 row struct와 key 컬럼을 연결합니다.

## DataTable 생성

에디터에서 다음 메뉴를 실행하면 JSON이 `UDataTable` 에셋으로 변환됩니다.

```text
Tools > Maverick > Generate Data Tables
```

콘솔 명령으로도 실행할 수 있습니다.

```text
MV.Table.GenerateDataTables
```

생성 결과:

- `/Game/Table/DT_<TableName>`: 테이블별 DataTable
- `/Game/Table/DT_MVTableManifest`: 런타임 로드용 manifest

런타임의 `UMVTableManager`는 JSON을 직접 읽지 않고, manifest에 기록된 DataTable 에셋을 로드합니다.

## Typed Row Struct 연결

테이블에 대응하는 C++ row struct가 있으면 typed DataTable이 생성됩니다.

```cpp
USTRUCT(BlueprintType, meta = (MVTable = "Item"))
struct FMVItemRow : public FMVTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 ItemId = 0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FString Name;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 Heal = 0;

    virtual void PostRead() override
    {
        RowId = ItemId;
    }
};
```

연결 규칙:

- `MVTable` 값은 JSON의 테이블 이름과 같아야 합니다.
- C++ 필드명은 JSON 필드명과 같아야 자동 변환됩니다.
- `PostRead()`에서 key 필드를 `RowId`에 복사해야 `TMVPropTable` 기반 조회 맵을 만들 수 있습니다.

## Generic Fallback

아직 대응하는 C++ row struct가 없는 테이블은 `FMVGenericTableRow`로 생성됩니다.

- `Key`: JSON key 값
- `RowJson`: 전체 행 JSON 문자열

이 fallback은 임시 확인용입니다. 실제 게임 로직에서 자주 쓰는 테이블은 typed row struct를 추가하는 편이 좋습니다.
