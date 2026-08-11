[CmdletBinding()]
param(
    [ValidateSet("codex", "claude", "antigravity")]
    [string]$HostName = "codex"
)

$ErrorActionPreference = "Stop"

$projectRoot = (Resolve-Path (Join-Path $PSScriptRoot "../..")).Path
$dependencyResult = & (Join-Path $projectRoot "Scripts/Agent/Ensure-AgentDependencies.ps1") -HostName $HostName
$graphifyPython = $dependencyResult.GraphifyPython
$graphRoot = Join-Path $projectRoot "graphify-out"
$graphPath = Join-Path $graphRoot "graph.json"
$modeStamp = Join-Path $projectRoot ".agent-tools/graphify/.maverick-graph-mode"
$expectedMode = "$($dependencyResult.Graphify):code-only:no-cluster"
$currentMode = if (Test-Path $modeStamp) { (Get-Content -Raw $modeStamp).Trim() } else { "" }

if ($currentMode -ne $expectedMode -and (Test-Path $graphRoot)) {
    $resolvedProjectRoot = [IO.Path]::GetFullPath($projectRoot).TrimEnd([IO.Path]::DirectorySeparatorChar) + [IO.Path]::DirectorySeparatorChar
    $resolvedGraphRoot = [IO.Path]::GetFullPath($graphRoot)
    if (-not $resolvedGraphRoot.StartsWith($resolvedProjectRoot, [StringComparison]::OrdinalIgnoreCase)) {
        throw "Graphify cache 경로가 프로젝트 외부를 가리킵니다: $resolvedGraphRoot"
    }

    Remove-Item -LiteralPath $resolvedGraphRoot -Recurse -Force
}

Push-Location $projectRoot
try {
    if (Test-Path $graphPath) {
        & $graphifyPython -m graphify update . --no-cluster
    }
    else {
        & $graphifyPython -m graphify extract . --code-only --no-cluster
    }

    if ($LASTEXITCODE -ne 0) {
        throw "Graphify 로컬 코드 그래프 준비 실패"
    }

    Set-Content -LiteralPath $modeStamp -Value $expectedMode -NoNewline
}
finally {
    Pop-Location
}

[pscustomobject]@{
    Graph = $graphPath
    Python = $graphifyPython
}
