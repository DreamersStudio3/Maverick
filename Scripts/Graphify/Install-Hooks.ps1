[CmdletBinding()]
param(
    [ValidateSet("Install", "Status", "Uninstall")]
    [string]$Action = "Install",
    [string]$Python = "python"
)

$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

$marker = "MAVERICK_GRAPHIFY_PRE_PUSH_V1"

function Resolve-MaverickHookDirectory {
    param([string]$RepoRoot)

    $hookPathText = (& git rev-parse --git-path hooks).Trim()
    if ($LASTEXITCODE -ne 0 -or [string]::IsNullOrWhiteSpace($hookPathText)) {
        throw "Git hook 디렉터리를 찾을 수 없습니다."
    }
    $hookDirectory = if ([System.IO.Path]::IsPathRooted($hookPathText)) {
        [System.IO.Path]::GetFullPath($hookPathText)
    } else {
        [System.IO.Path]::GetFullPath((Join-Path $RepoRoot $hookPathText))
    }

    # Husky 9의 core.hooksPath=.husky/_ 는 generated dispatcher 경로다.
    # Graphify 공식 installer와 동일하게 실제 user hook이 있는 parent를 사용한다.
    if ((Split-Path $hookDirectory -Leaf) -eq "_" -and
        (Split-Path (Split-Path $hookDirectory -Parent) -Leaf) -eq ".husky") {
        $hookDirectory = Split-Path $hookDirectory -Parent
    }

    $repoPrefix = [System.IO.Path]::GetFullPath($RepoRoot).TrimEnd('\', '/') + [System.IO.Path]::DirectorySeparatorChar
    if (-not $hookDirectory.StartsWith($repoPrefix, [System.StringComparison]::OrdinalIgnoreCase)) {
        throw "저장소 밖의 shared core.hooksPath에는 Maverick 전용 훅을 설치하지 않습니다: $hookDirectory"
    }
    return $hookDirectory
}

function Assert-ShellHookCompatible {
    param([string]$Path)

    if (-not (Test-Path -LiteralPath $Path)) {
        return
    }
    $content = Get-Content -LiteralPath $Path -Raw
    if ($content -match "graphify-(hook|checkout)-start") {
        return
    }
    $firstLine = ($content -split "`r?`n", 2)[0]
    if ($firstLine -notmatch '^#!.*\b(sh|bash)\b') {
        throw "Graphify가 shell block을 안전하게 추가할 수 없는 기존 hook입니다: $Path"
    }
}

function Get-GraphifyOfficialState {
    param(
        [string]$RepoRoot,
        [string]$HookDirectory
    )

    $postCommitPath = Join-Path $HookDirectory "post-commit"
    $postCheckoutPath = Join-Path $HookDirectory "post-checkout"
    $postCommit = (Test-Path -LiteralPath $postCommitPath) -and
        ((Get-Content -LiteralPath $postCommitPath -Raw) -match '# graphify-hook-start')
    $postCheckout = (Test-Path -LiteralPath $postCheckoutPath) -and
        ((Get-Content -LiteralPath $postCheckoutPath -Raw) -match '# graphify-checkout-hook-start')
    $mergeDriver = -not [string]::IsNullOrWhiteSpace(
        ((& git config --get merge.graphify.driver 2>$null) -join "`n").Trim()
    )
    $attributesPath = Join-Path $RepoRoot ".gitattributes"
    $attributes = (Test-Path -LiteralPath $attributesPath) -and
        ((Get-Content -LiteralPath $attributesPath -Raw) -match '(?m)^\s*graphify-out/graph\.json\s+.*\bmerge=graphify\b')

    return [PSCustomObject]@{
        PostCommit = $postCommit
        PostCheckout = $postCheckout
        MergeDriver = $mergeDriver
        Attributes = $attributes
    }
}

function Assert-GraphifyOfficialInstalled {
    param([PSCustomObject]$State)

    $missing = @()
    if (-not $State.PostCommit) { $missing += "post-commit" }
    if (-not $State.PostCheckout) { $missing += "post-checkout" }
    if (-not $State.MergeDriver) { $missing += "merge.graphify.driver" }
    if (-not $State.Attributes) { $missing += ".gitattributes merge rule" }
    if ($missing.Count -gt 0) {
        throw "Graphify 공식 훅 설치가 불완전합니다: $($missing -join ', ')"
    }
}

$repoRoot = (& git rev-parse --show-toplevel).Trim()
if ($LASTEXITCODE -ne 0 -or [string]::IsNullOrWhiteSpace($repoRoot)) {
    throw "Git 저장소 안에서 실행해야 합니다."
}

Push-Location -LiteralPath $repoRoot
try {
    $hookDirectory = Resolve-MaverickHookDirectory -RepoRoot $repoRoot
    $target = Join-Path $hookDirectory "pre-push"

    if ($Action -eq "Status") {
        & $Python -m graphify hook status
        $officialState = Get-GraphifyOfficialState -RepoRoot $repoRoot -HookDirectory $hookDirectory
        $installed = (Test-Path -LiteralPath $target) -and
            ((Get-Content -LiteralPath $target -Raw) -match [regex]::Escape($marker))
        Write-Host ("Maverick pre-push: " + $(if ($installed) { "installed" } else { "not installed" }))
        try {
            Assert-GraphifyOfficialInstalled -State $officialState
        } catch {
            Write-Error $_
            exit 1
        }
        if (-not $installed) {
            exit 1
        }
        exit 0
    }

    if ($Action -eq "Uninstall") {
        if (Test-Path -LiteralPath $target) {
            $content = Get-Content -LiteralPath $target -Raw
            if ($content -notmatch [regex]::Escape($marker)) {
                throw "Maverick가 소유하지 않은 pre-push 훅은 제거하지 않습니다: $target"
            }
            Remove-Item -LiteralPath $target
            Write-Host "Maverick pre-push gate 제거 완료: $target"
        }
        & $Python -m graphify hook uninstall
        if ($LASTEXITCODE -ne 0) {
            throw "Graphify 공식 훅 제거에 실패했습니다."
        }
        exit 0
    }

    # 모든 충돌 검사를 공식 installer보다 먼저 수행해 부분 설치를 피한다.
    if (Test-Path -LiteralPath $target) {
        $current = Get-Content -LiteralPath $target -Raw
        if ($current -notmatch [regex]::Escape($marker)) {
            throw "기존 pre-push 훅이 있어 자동 설치하지 않습니다. 수동 통합이 필요합니다: $target"
        }
    }
    Assert-ShellHookCompatible -Path (Join-Path $hookDirectory "post-commit")
    Assert-ShellHookCompatible -Path (Join-Path $hookDirectory "post-checkout")

    $pythonPath = (& $Python -c "import sys; print(sys.executable)").Trim()
    if ($LASTEXITCODE -ne 0 -or [string]::IsNullOrWhiteSpace($pythonPath)) {
        throw "설치에 사용할 Python interpreter를 확인할 수 없습니다."
    }

    & $Python -m graphify hook install
    if ($LASTEXITCODE -ne 0) {
        throw "Graphify 공식 훅 설치에 실패했습니다."
    }
    $officialState = Get-GraphifyOfficialState -RepoRoot $repoRoot -HookDirectory $hookDirectory
    Assert-GraphifyOfficialInstalled -State $officialState

    New-Item -ItemType Directory -Path $hookDirectory -Force | Out-Null
    $template = Join-Path $repoRoot "Scripts/Graphify/hooks/pre-push"
    $temporary = Join-Path $hookDirectory "pre-push.maverick-new"
    Copy-Item -LiteralPath $template -Destination $temporary -Force
    try {
        if (Test-Path -LiteralPath $target) {
            $rollback = Join-Path $hookDirectory "pre-push.maverick-rollback"
            [System.IO.File]::Replace($temporary, $target, $rollback, $true)
            Remove-Item -LiteralPath $rollback -ErrorAction SilentlyContinue
        } else {
            Move-Item -LiteralPath $temporary -Destination $target
        }
    }
    finally {
        Remove-Item -LiteralPath $temporary -ErrorAction SilentlyContinue
    }

    if ($env:OS -ne "Windows_NT") {
        & chmod +x -- $target
        if ($LASTEXITCODE -ne 0) {
            throw "pre-push hook 실행 권한 설정에 실패했습니다."
        }
    }

    $graphifyOut = Join-Path $repoRoot "graphify-out"
    New-Item -ItemType Directory -Path $graphifyOut -Force | Out-Null
    $pythonHint = Join-Path $graphifyOut ".graphify_python"
    [System.IO.File]::WriteAllText(
        $pythonHint,
        $pythonPath + "`n",
        [System.Text.UTF8Encoding]::new($false)
    )

    Write-Host "Maverick Graphify pre-push gate 설치 완료: $target"
    Write-Host "설치 Python: $pythonPath"
    Write-Host "상태 확인: ./Scripts/Graphify/Install-Hooks.ps1 -Action Status"
}
finally {
    Pop-Location
}
