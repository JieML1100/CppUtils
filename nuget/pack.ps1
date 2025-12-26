param(
  [string]$Version = "1.0.0",
  [string]$OutDir = (Join-Path $PSScriptRoot "out")
)

$ErrorActionPreference = "Stop"

$pkgDir = Join-Path $PSScriptRoot "CppUtils"
$nuspec = Join-Path $pkgDir "CppUtils.nuspec"
$toolsDir = Join-Path $PSScriptRoot "tools"
$nugetExe = Join-Path $toolsDir "nuget.exe"

if (!(Test-Path $nuspec)) {
  throw "Nuspec not found: $nuspec"
}

if (!(Test-Path $nugetExe)) {
  New-Item -ItemType Directory -Force -Path $toolsDir | Out-Null
  $url = "https://dist.nuget.org/win-x86-commandline/latest/nuget.exe"
  Write-Host "Downloading nuget.exe from $url"
  Invoke-WebRequest -Uri $url -OutFile $nugetExe
}

New-Item -ItemType Directory -Force -Path $OutDir | Out-Null

& $nugetExe pack $nuspec -Version $Version -OutputDirectory $OutDir -NoDefaultExcludes | Write-Host
Write-Host "Packed: $OutDir"
