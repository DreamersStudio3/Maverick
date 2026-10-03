param(
    [string]$EngineRoot = 'C:\Program Files\Epic Games\UE_5.8',
    [string]$Python
)

$ErrorActionPreference = 'Stop'
$projectRoot = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
$reportRoot = Join-Path $projectRoot 'Saved\AssetValidation\TrainingDummy'
$editor = Join-Path $EngineRoot 'Engine\Binaries\Win64\UnrealEditor-Cmd.exe'
if (-not $Python) { $Python = Join-Path $projectRoot '.agent-tools\graphify\Scripts\python.exe' }
if (-not (Test-Path -LiteralPath $editor)) { throw "UnrealEditor-Cmd 누락: $editor" }
if (-not (Test-Path -LiteralPath $Python)) { throw 'Python 경로 지정 필요: -Python <python.exe>' }
New-Item -ItemType Directory -Path $reportRoot -Force | Out-Null

# 이전 실행의 결과가 새 검사로 오인되는 상황 방지
foreach ($name in @('Automation\index.json', 'training-hit-output.wav', 'capture-validation.json', 'training-idle.png', 'training-hit.png')) {
    $file = Join-Path $reportRoot $name
    if (Test-Path -LiteralPath $file) { Remove-Item -LiteralPath $file }
}

& $editor (Join-Path $projectRoot 'Maverick.uproject') `
    -unattended -nop4 -nosplash -NoLoadingScreen -AudioMixer -RenderOffscreen `
    '-ini:Engine:[Audio]:UnfocusedVolumeMultiplier=1.0' `
    '-ini:EditorPerProjectUserSettings:[/Script/UnrealEd.LevelEditorMiscSettings]:bAllowBackgroundAudio=True' `
    '-ExecCmds=t.MaxFPS 60,Automation RunTests Maverick.Combat.TrainingDummy.PlayerMeleePIE' `
    '-TestExit=Automation Test Queue Empty' `
    "-ReportExportPath=$reportRoot\Automation" "-abslog=$reportRoot\pie.log" *> (Join-Path $reportRoot 'pie-console.log')
if ($LASTEXITCODE -ne 0) { throw "Unreal 실행 실패: $reportRoot\pie.log" }
& $Python (Join-Path $PSScriptRoot 'Verify-TrainingDummyCapture.py')
if ($LASTEXITCODE -ne 0) { throw "허수아비 검증 실패: $reportRoot\capture-validation.json" }
