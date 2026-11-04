[CmdletBinding()]
param(
    [Parameter(Mandatory = $true, Position = 0)]
    [ValidateSet("query", "path", "explain")]
    [string]$Operation,

    [Parameter(Mandatory = $true, Position = 1, ValueFromRemainingArguments = $true)]
    [string[]]$InputValue,

    [ValidateSet("codex", "claude", "antigravity")]
    [string]$HostName = "codex"
)

$ErrorActionPreference = "Stop"

$ready = & (Join-Path $PSScriptRoot "Ensure-GraphifyReady.ps1") -HostName $HostName
$projectRoot = (Resolve-Path (Join-Path $PSScriptRoot "../..")).Path

Push-Location $projectRoot
try {
    & $ready.Python -m graphify $Operation @InputValue --graph $ready.Graph
    exit $LASTEXITCODE
}
finally {
    Pop-Location
}
