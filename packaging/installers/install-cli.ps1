# Install the precompiled Windows snt CLI without administrator privileges.
param(
    [string]$Version = 'latest',
    [string]$Prefix = (Join-Path $env:LOCALAPPDATA 'Programs\snt'),
    [string]$Archive
)

$ErrorActionPreference = 'Stop'
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
$Prefix = [IO.Path]::GetFullPath($Prefix)
$versions = Join-Path $Prefix 'versions'
$target = Join-Path $versions $Version
$bin = Join-Path $Prefix 'bin'
$shim = Join-Path $bin 'snt.cmd'
New-Item -ItemType Directory -Force -Path $versions, $bin | Out-Null

if (Test-Path -LiteralPath $shim) {
    $firstLine = Get-Content -LiteralPath $shim -TotalCount 1
    if ($firstLine -ne '@REM Managed by scinumtools3 install-cli.ps1') {
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
    '@REM Managed by scinumtools3 install-cli.ps1'
    '@echo off'
    "`"%~dp0..\versions\$Version\snt.exe`" %*"
) | Set-Content -LiteralPath $shim -Encoding Ascii

$userPath = [Environment]::GetEnvironmentVariable('Path', 'User')
if (($userPath -split ';') -notcontains $bin) {
    $newPath = if ($userPath) { "$userPath;$bin" } else { $bin }
    [Environment]::SetEnvironmentVariable('Path', $newPath, 'User')
}
if (($env:Path -split ';') -notcontains $bin) {
    $env:Path = "$bin;$env:Path"
}
Write-Host "Installed $shim"
Write-Host 'Run snt --version in a new terminal.'
