# Maverick 테이블 시스템 사용법

이 폴더는 게임 데이터 테이블의 원본 CSV, JSON 중간 산출물, 변환 도구를 보관합니다.

## 전체 흐름

1. `MaverickDesign/Csv`에 CSV 파일을 추가하거나 수정합니다.
2. 에디터의 `Tools > Maverick > Generate Data Tables` 메뉴를 실행합니다.
3. 변환기가 CSV를 `MaverickDesign/Json`으로 변환합니다.
4. 생성기가 JSON을 `/Game/Table/DT_*` `UDataTable` 에셋으로 가져옵니다.
5. 생성기가 `/Game/Table/DT_MVTableManifest`를 갱신합니다.
6. 런타임에서는 `UMVTableManager`가 manifest를 읽고 생성된 `UDataTable`들을 로드합니다.

`MaverickDesign/Json`은 에디터 생성용 중간 산출물입니다. 런타임의 기준 데이터는 `/Game/Table` 아래 생성된 `UDataTable` 에셋입니다.

## CSV 작성

CSV 파일 하나는 기본적으로 같은 이름의 테이블 하나가 됩니다.

예: `MaverickDesign/Csv/Item.csv`

```csv
ItemId,Name,HealAmount
1001,SmallPotion,50
1002,LargePotion,150
```

일반 CSV 형식에서는 첫 번째 비어 있지 않은 행을 헤더로 사용하고, 첫 번째 헤더 컬럼을 key 컬럼으로 사용합니다.

다음과 같은 Client 플래그 형식도 지원합니다.

```csv
,ItemId,Name,InternalMemo,HealAmount
Client,1,1,0,1
,ItemId,Name,InternalMemo,HealAmount
,1001,SmallPotion,not exported,50
,1002,LargePotion,not exported,150
```

이 형식에서는 `Client` 행에서 값이 `1` 이상인 컬럼만 JSON과 DataTable에 포함됩니다. 첫 번째 포함 컬럼이 key 컬럼입니다.

`Client` 플래그는 원본 CSV에는 남겨두고 싶지만 런타임 데이터에는 포함하지 않을 컬럼을 걸러내기 위한 용도입니다. 예를 들어 기획자 메모, 검수 상태, 작업자, 임시 계산값, 서버 전용 값처럼 에디터 작업에는 유용하지만 클라이언트 런타임에는 필요 없는 컬럼을 제외할 수 있습니다.

```csv
,ItemId,Name,HealAmount,DesignerMemo,ReviewStatus
Client,1,1,1,0,0
,ItemId,Name,HealAmount,DesignerMemo,ReviewStatus
,1001,SmallPotion,50,초반 지급용,Done
```

위 예시에서는 `DesignerMemo`와 `ReviewStatus`가 원본 CSV에는 남지만 JSON과 DataTable에는 포함되지 않습니다.

## 수동 CSV 변환

에디터 생성 명령은 CSV 변환을 자동으로 실행합니다. 변환만 따로 확인하려면 프로젝트 루트에서 실행합니다.

```powershell
python MaverickDesign\CsvToJsonConverter\CsvToJsonConverter.py
```

결과는 `MaverickDesign/Json`에 저장됩니다.

## Typed Row Struct 추가

테이블을 C++ 구조체로 사용하려면 `FMVTableRowBase`를 상속하고 `MVTable` 메타데이터에 테이블 이름을 적습니다.

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
    int32 HealAmount = 0;

    virtual void PostRead() override
    {
        RowId = ItemId;
    }
};
```

`MVTable` 값은 CSV 파일명에서 확장자를 뺀 테이블 이름과 같아야 합니다. 위 예시는 `Item.csv`와 연결됩니다.

`PostRead()`에서 `RowId`에 key 값을 복사해야 `TMVPropTable` 기반 조회 맵을 만들 수 있습니다.

### `RowId`와 `PostRead()`

Unreal의 `FTableRowBase`는 `Tid` 같은 ID 필드를 제공하지 않습니다. Unreal DataTable은 행을 `RowName`(`FName`)으로 식별합니다.

`FMVTableRowBase::RowId`는 이 테이블 시스템에서 추가한 공통 런타임 조회 key입니다. 각 테이블의 실제 key 컬럼명은 `ItemId`, `SkillId`, `MonsterId`처럼 서로 다를 수 있으므로, 공통 조회 컨테이너가 어떤 필드를 key로 써야 하는지 알 수 없습니다.

그래서 각 row struct는 자신의 key 컬럼을 알고 있는 `PostRead()`에서 `RowId`에 값을 복사합니다.

```cpp
virtual void PostRead() override
{
    RowId = ItemId;
}
```

이후 `TMVPropTable`은 모든 row의 `PostRead()`를 호출한 다음 `RowId`를 기준으로 `TMap<int32, Row*>`를 만듭니다.

```cpp
TMVPropTable<FMVItemRow> ItemTable;
if (TableManager->BuildPropTable(TEXT("Item"), ItemTable))
{
    const FMVItemRow* Row = ItemTable.Find(1001);
}
```

주의할 점:

- `PostRead()`를 구현하지 않으면 `RowId` 기본값인 `0`으로 맵이 만들어질 수 있습니다.
- key 컬럼이 `int32`가 아니라면 `TMVPropTable<RowType, KeyType>` 사용 방식도 함께 검토해야 합니다.
- `RowId`는 `Transient` 값이므로 DataTable 에셋에 저장되는 원본 컬럼이 아니라 런타임에서 계산되는 공통 key입니다.
- CSV의 key 컬럼, row struct의 key 필드, `PostRead()`의 대입 대상은 항상 같은 의미여야 합니다.

## DataTable 생성

에디터 메뉴에서 실행합니다.

```text
Tools > Maverick > Generate Data Tables
```

콘솔 명령으로도 실행할 수 있습니다.

```text
MV.Table.GenerateDataTables
```

생성 결과:

- `/Game/Table/DT_<TableName>`: 테이블별 `UDataTable`
- `/Game/Table/DT_MVTableManifest`: 런타임 로드용 manifest

매칭되는 C++ row struct가 있으면 typed `UDataTable`이 생성됩니다. 아직 row struct가 없으면 `FMVGenericTableRow`로 fallback 테이블이 생성됩니다.

## 런타임 조회

런타임에서는 `UMVTableManager`를 통해 생성된 DataTable을 조회합니다.

```cpp
const UMVTableManager* TableManager = UMVTableManager::Get(this);
const FMVItemRow* ItemRow = TableManager
    ? TableManager->FindRow<FMVItemRow>(TEXT("Item"), TEXT("1001"))
    : nullptr;
```

Blueprint나 임시 호출부에서는 필드 단위 조회 API도 사용할 수 있습니다.

- `GetString`
- `GetInt`
- `GetFloat`
- `GetBool`
- `GetRowJson`

## 변경 시 주의사항

CSV나 row struct를 변경했다면 `MV.Table.GenerateDataTables`를 다시 실행해야 합니다.

row struct의 필드명은 JSON 필드명과 같아야 자동 변환됩니다. key 컬럼이 바뀌면 CSV 헤더, row struct 필드, `PostRead()`의 `RowId` 대입을 함께 맞춰야 합니다.
