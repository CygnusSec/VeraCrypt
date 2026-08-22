param(
    [Parameter(Mandatory = $true)]
    [string] $ExternalModule,

    [string] $ExternalCipherName = "External",

    [ValidateSet("Debug", "Release", "Release_SkipOsDriverReqCheck")]
    [string] $Configuration = "Release",

    [ValidateSet("x64", "ARM64")]
    [string] $Platform = "x64",

    [string] $ExternalModuleCFlags = ""
)

$ErrorActionPreference = "Stop"
$solution = (Resolve-Path (Join-Path $PSScriptRoot "..\VeraCrypt.sln")).Path
$module = (Resolve-Path $ExternalModule).Path
$adapter = Join-Path $module "src\veracrypt_cipher_adapter.c"

if (-not (Test-Path $adapter -PathType Leaf)) {
    throw "External cipher adapter not found: $adapter"
}

$msbuildCommand = Get-Command msbuild.exe -ErrorAction SilentlyContinue
if (-not $msbuildCommand) {
    $vswhere = Join-Path ${env:ProgramFiles(x86)} "Microsoft Visual Studio\Installer\vswhere.exe"
    if (-not (Test-Path $vswhere -PathType Leaf)) {
        throw "MSBuild was not found. Run this script from a Visual Studio Developer PowerShell or install Visual Studio Build Tools."
    }
    $msbuildPath = & $vswhere -latest -products * -requires Microsoft.Component.MSBuild -find "MSBuild\**\Bin\MSBuild.exe" | Select-Object -First 1
    if (-not $msbuildPath) {
        throw "vswhere could not locate MSBuild.exe."
    }
} else {
    $msbuildPath = $msbuildCommand.Source
}

$arguments = @(
    $solution,
    "/m",
    "/t:Build",
    "/p:Configuration=$Configuration",
    "/p:Platform=$Platform",
    "/p:VCExternalModuleSourceDir=$module",
    "/p:VCExternalCipherName=$ExternalCipherName"
)

if ($ExternalModuleCFlags) {
    $arguments += "/p:VCExternalModuleCFlags=$ExternalModuleCFlags"
}

Write-Host "Static-linking external cipher from $module"
Write-Host "Cipher name: $ExternalCipherName"
& $msbuildPath @arguments
if ($LASTEXITCODE -ne 0) {
    throw "VeraCrypt Windows build failed with exit code $LASTEXITCODE."
}
