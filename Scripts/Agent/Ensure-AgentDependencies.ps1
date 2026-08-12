[CmdletBinding()]
param(
    [ValidateSet("codex", "claude", "antigravity")]
    [string]$HostName = "codex"
)

$ErrorActionPreference = "Stop"

$projectRoot = (Resolve-Path (Join-Path $PSScriptRoot "../..")).Path
$dependencyFile = Join-Path $projectRoot "Config/AgentDependencies.json"
$dependencies = Get-Content -Raw $dependencyFile | ConvertFrom-Json
$userRoot = [Environment]::GetFolderPath([Environment+SpecialFolder]::UserProfile)

function Invoke-CheckedCommand {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Command,

        [Parameter(ValueFromRemainingArguments = $true)]
        [string[]]$Arguments
    )

    & $Command @Arguments
    if ($LASTEXITCODE -ne 0) {
        throw "명령 실패: $Command $($Arguments -join ' ')"
    }
}

$hiveVersion = [string]$dependencies.hive.version
$hiveCommand = Get-Command hive -ErrorAction SilentlyContinue
if (-not $hiveCommand -or -not ((& hive --version) -match [regex]::Escape($hiveVersion))) {
    if (-not (Get-Command npm -ErrorAction SilentlyContinue)) {
        throw "Hive $hiveVersion 자동 설치에 필요한 npm을 찾을 수 없습니다."
    }

    Invoke-CheckedCommand npm install --global "$($dependencies.hive.package)@$hiveVersion"
}

& hive install --scope user --host $HostName --validate --user-root $userRoot --output json *> $null
if ($LASTEXITCODE -ne 0) {
    Invoke-CheckedCommand hive install --scope user --host $HostName --apply --user-root $userRoot --output json
    Invoke-CheckedCommand hive install --scope user --host $HostName --validate --user-root $userRoot --output json
}

if (-not (Test-Path (Join-Path $projectRoot ".hive/config/harness.toml"))) {
    throw "프로젝트 Hive harness 누락: .hive/config/harness.toml"
}

$graphifyVersion = [string]$dependencies.graphify.version
$graphifyEnvironment = Join-Path $projectRoot ([string]$dependencies.graphify.environment)
$isWindowsPlatform = [Environment]::OSVersion.Platform -eq [PlatformID]::Win32NT
$graphifyPython = if ($isWindowsPlatform) {
    Join-Path $graphifyEnvironment "Scripts/python.exe"
}
else {
    Join-Path $graphifyEnvironment "bin/python"
}

if (-not (Test-Path $graphifyPython)) {
    $pythonLauncher = if ($isWindowsPlatform) {
        Get-Command py -ErrorAction SilentlyContinue
    }
    else {
        Get-Command python3 -ErrorAction SilentlyContinue
    }

    if (-not $pythonLauncher) {
        throw "Graphify $graphifyVersion 자동 설치에 필요한 Python launcher를 찾을 수 없습니다."
    }

    Invoke-CheckedCommand $pythonLauncher.Source -m venv $graphifyEnvironment
}

$installedGraphifyVersion = & $graphifyPython -c "import importlib.metadata; print(importlib.metadata.version('graphifyy'))" 2>$null
if ($LASTEXITCODE -ne 0 -or $installedGraphifyVersion.Trim() -ne $graphifyVersion) {
    Invoke-CheckedCommand $graphifyPython -m pip install --disable-pip-version-check --upgrade "$($dependencies.graphify.package)==$graphifyVersion"
}

$graphifySkill = Join-Path $userRoot ".agents/skills/graphify/SKILL.md"
$skillStamp = Join-Path $graphifyEnvironment ".agents-skill-version"
$stampedVersion = if (Test-Path $skillStamp) { (Get-Content -Raw $skillStamp).Trim() } else { "" }
if (-not (Test-Path $graphifySkill) -or $stampedVersion -ne $graphifyVersion) {
    $skillInstallRoot = Join-Path ([IO.Path]::GetTempPath()) ("maverick-graphify-skill-" + [guid]::NewGuid().ToString("N"))
    New-Item -ItemType Directory -Path $skillInstallRoot | Out-Null
    try {
        Push-Location $skillInstallRoot
        Invoke-CheckedCommand $graphifyPython -m graphify agents install
    }
    finally {
        Pop-Location
        $resolvedTempRoot = [IO.Path]::GetFullPath([IO.Path]::GetTempPath())
        $resolvedInstallRoot = [IO.Path]::GetFullPath($skillInstallRoot)
        if ($resolvedInstallRoot.StartsWith($resolvedTempRoot, [StringComparison]::OrdinalIgnoreCase)) {
            Remove-Item -LiteralPath $resolvedInstallRoot -Recurse -Force
        }
    }

    Set-Content -LiteralPath $skillStamp -Value $graphifyVersion -NoNewline
}

[pscustomobject]@{
    Hive = $hiveVersion
    Graphify = $graphifyVersion
    GraphifyPython = $graphifyPython
    Host = $HostName
}
