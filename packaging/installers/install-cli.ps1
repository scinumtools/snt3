# Install or uninstall the precompiled Windows snt CLI without administrator privileges.
param(
    [string]$Version = 'latest',
    [string]$Prefix = (Join-Path $env:LOCALAPPDATA 'Programs\snt'),
    [string]$Archive,
    [switch]$Uninstall
)

$ErrorActionPreference = 'Stop'
if ($Version -ne 'latest' -and $Version -notmatch '^v[0-9]+\.[0-9]+\.[0-9]+$') {
    throw "Invalid release tag: $Version"
}

$Prefix = [IO.Path]::GetFullPath($Prefix)
$versions = Join-Path $Prefix 'versions'
$bin = Join-Path $Prefix 'bin'
$shim = Join-Path $bin 'snt.cmd'
$pathMarker = Join-Path $Prefix '.snt-path-added'
$defaultPrefix = [IO.Path]::GetFullPath((Join-Path $env:LOCALAPPDATA 'Programs\snt'))
$shimMarker = '@REM Managed by scinumtools3 install-cli.ps1'

if ($Uninstall) {
    if ($Archive) {
        throw '-Archive cannot be used with -Uninstall.'
    }
    if (-not (Test-Path -LiteralPath $Prefix -PathType Container)) {
        Write-Host "No snt CLI installation found under $Prefix"
        return
    }

    $managedShim = $false
    $activeVersion = $null
    if (Test-Path -LiteralPath $shim -PathType Leaf) {
        $shimLines = @(Get-Content -LiteralPath $shim -TotalCount 3)
        if ($shimLines.Count -gt 0 -and $shimLines[0] -eq $shimMarker) {
            $managedShim = $true
            if ($shimLines.Count -gt 2 -and $shimLines[2] -match '\\versions\\(v[0-9]+\.[0-9]+\.[0-9]+)\\snt\.exe') {
                $activeVersion = $Matches[1]
            }
        }
    }
    $removed = $false
    if ($managedShim -and ($Version -eq 'latest' -or $activeVersion -eq $Version)) {
        Remove-Item -LiteralPath $shim -Force
        $removed = $true
    }

    if ($Version -eq 'latest') {
        if (Test-Path -LiteralPath $versions -PathType Container) {
            foreach ($entry in (Get-ChildItem -LiteralPath $versions -Directory)) {
                if ($entry.Name -match '^v[0-9]+\.[0-9]+\.[0-9]+$') {
                    Remove-Item -LiteralPath $entry.FullName -Recurse -Force
                    $removed = $true
                }
            }
        }
    } else {
        $target = Join-Path $versions $Version
        if (Test-Path -LiteralPath $target -PathType Container) {
            Remove-Item -LiteralPath $target -Recurse -Force
            $removed = $true
        }
    }

    if (-not (Test-Path -LiteralPath $shim) -and
        ((Test-Path -LiteralPath $pathMarker) -or $Prefix -eq $defaultPrefix)) {
        $userPath = [Environment]::GetEnvironmentVariable('Path', 'User')
        $remainingPath = @(($userPath -split ';') | Where-Object { $_ -ne $bin }) -join ';'
        if ($remainingPath -ne $userPath) {
            [Environment]::SetEnvironmentVariable('Path', $remainingPath, 'User')
        }
        if (Test-Path -LiteralPath $pathMarker) {
            Remove-Item -LiteralPath $pathMarker -Force
        }
    }
    foreach ($directory in @($bin, $versions, $Prefix)) {
        if ((Test-Path -LiteralPath $directory -PathType Container) -and
            -not (Get-ChildItem -LiteralPath $directory -Force | Select-Object -First 1)) {
            Remove-Item -LiteralPath $directory
        }
    }
    if ($removed) {
        Write-Host "Uninstalled snt CLI from $Prefix"
    } else {
        Write-Host "No matching snt CLI release found under $Prefix"
    }
    return
}

if ($env:PROCESSOR_ARCHITECTURE -ne 'AMD64') {
    throw 'No precompiled Windows CLI archive is available for this architecture.'
}

if ($Archive) {
    $Archive = (Resolve-Path -LiteralPath $Archive).Path
    $name = [IO.Path]::GetFileName($Archive)
    if ($name -notmatch '^snt-(v[0-9]+\.[0-9]+\.[0-9]+)-windows-x86_64\.zip$') {
        throw 'Expected an snt-<version>-windows-x86_64.zip archive.'
    }
    $archiveTag = $Matches[1]
    if ($Version -ne 'latest' -and $Version -ne $archiveTag) {
        throw "Archive version $archiveTag does not match $Version."
    }
    $Version = $archiveTag
} elseif ($Version -eq 'latest') {
    $Version = (Invoke-RestMethod 'https://api.github.com/repos/scinumtools/snt3/releases/latest').tag_name
}

if ($Version -notmatch '^v[0-9]+\.[0-9]+\.[0-9]+$') {
    throw "Invalid release tag: $Version"
}

$name = "snt-$Version-windows-x86_64.zip"
$target = Join-Path $versions $Version
New-Item -ItemType Directory -Force -Path $versions, $bin | Out-Null

if (Test-Path -LiteralPath $shim) {
    $firstLine = Get-Content -LiteralPath $shim -TotalCount 1
    if ($firstLine -ne $shimMarker) {
        throw "Will not replace an existing command: $shim"
    }
}

if (-not (Test-Path -LiteralPath $target)) {
    $stage = Join-Path $versions ('.install-' + [guid]::NewGuid().ToString('N'))
    New-Item -ItemType Directory -Path $stage | Out-Null
    try {
        if (-not $Archive) {
            $Archive = Join-Path $stage $name
            $url = "https://github.com/scinumtools/snt3/releases/download/$Version/$name"
            Invoke-WebRequest -Uri $url -OutFile $Archive -UseBasicParsing
        }
        Expand-Archive -LiteralPath $Archive -DestinationPath $stage
        $extracted = Join-Path $stage "snt-$Version-windows-x86_64"
        $executable = Join-Path $extracted 'snt.exe'
        if (-not (Test-Path -LiteralPath $executable)) {
            throw 'Archive does not contain snt.exe.'
        }
        & $executable --version | Out-Null
        if ($LASTEXITCODE -ne 0) {
            throw 'snt could not run; check system runtime libraries.'
        }
        Move-Item -LiteralPath $extracted -Destination $target
    } finally {
        Remove-Item -LiteralPath $stage -Recurse -Force -ErrorAction SilentlyContinue
    }
}

if (-not (Test-Path -LiteralPath (Join-Path $target 'snt.exe'))) {
    throw "Installation is incomplete: $target"
}

@(
    $shimMarker
    '@echo off'
    "`"%~dp0..\versions\$Version\snt.exe`" %*"
) | Set-Content -LiteralPath $shim -Encoding Ascii

$userPath = [Environment]::GetEnvironmentVariable('Path', 'User')
if (($userPath -split ';') -notcontains $bin) {
    $newPath = if ($userPath) { "$userPath;$bin" } else { $bin }
    [Environment]::SetEnvironmentVariable('Path', $newPath, 'User')
    Set-Content -LiteralPath $pathMarker -Value 'Added by scinumtools3 install-cli.ps1' -Encoding Ascii
}
if (($env:Path -split ';') -notcontains $bin) {
    $env:Path = "$bin;$env:Path"
}
Write-Host "Installed $shim"
Write-Host 'Run snt --version in a new terminal.'
