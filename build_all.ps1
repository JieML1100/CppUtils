# Build script for CppUtils and Graphics libraries
# Compiles all combinations: x86/x64 + MT/MTd/MD/MDd

param(
    [switch]$Clean = $false
)

# Color output functions
function Write-Success {
    param([string]$Message)
    Write-Host $Message -ForegroundColor Green
}

function Write-Error-Custom {
    param([string]$Message)
    Write-Host $Message -ForegroundColor Red
}

function Write-Info {
    param([string]$Message)
    Write-Host $Message -ForegroundColor Cyan
}

# Get the script directory
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$SolutionFile = Join-Path $ScriptDir "CppUtils.sln"
$BuildDir = Join-Path $ScriptDir "build"

# Check if solution file exists
if (-not (Test-Path $SolutionFile)) {
    Write-Error-Custom "Solution file not found: $SolutionFile"
    exit 1
}

# Create build directory if it doesn't exist
if (-not (Test-Path $BuildDir)) {
    New-Item -ItemType Directory -Path $BuildDir -Force | Out-Null
}

# Find all installed Visual Studio versions
$msbuildPath = $null
$vsWhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"

if (-not (Test-Path $vsWhere)) {
    Write-Error-Custom "vswhere.exe not found. Please install Visual Studio or Build Tools."
    exit 1
}

# Get all installed Visual Studio instances
$vsInstances = & $vsWhere -all -products * -requires Microsoft.Component.MSBuild -format json | ConvertFrom-Json

if (-not $vsInstances -or $vsInstances.Count -eq 0) {
    Write-Error-Custom "No Visual Studio installation found with MSBuild component."
    exit 1
}

# Display all available compilers
Write-Info "=" * 80
Write-Info "Available Visual Studio Installations:"
Write-Info "=" * 80
Write-Host ""

$vsOptions = @()
$index = 1

foreach ($vs in $vsInstances) {
    $vsVersion = $vs.installationVersion
    $vsName = $vs.displayName
    $vsPath = $vs.installationPath
    
    # Try to find MSBuild path (prefer 64-bit)
    $msbuild = Join-Path $vsPath "MSBuild\Current\Bin\amd64\MSBuild.exe"
    if (-not (Test-Path $msbuild)) {
        $msbuild = Join-Path $vsPath "MSBuild\Current\Bin\MSBuild.exe"
    }
    if (-not (Test-Path $msbuild)) {
        $msbuild = Join-Path $vsPath "MSBuild\15.0\Bin\amd64\MSBuild.exe"
    }
    if (-not (Test-Path $msbuild)) {
        $msbuild = Join-Path $vsPath "MSBuild\15.0\Bin\MSBuild.exe"
    }
    
    if (Test-Path $msbuild) {
        # Get toolset version
        $toolset = ""
        if ($vsVersion -match "^17\.") {
            $toolset = "v143 (Visual Studio 2022)"
        } elseif ($vsVersion -match "^16\.") {
            $toolset = "v142 (Visual Studio 2019)"
        } elseif ($vsVersion -match "^15\.") {
            $toolset = "v141 (Visual Studio 2017)"
        } else {
            $toolset = "v$($vsVersion.Split('.')[0])"
        }
        
        Write-Host "[$index] $vsName" -ForegroundColor Green
        Write-Host "    Version: $vsVersion" -ForegroundColor Gray
        Write-Host "    Toolset: $toolset" -ForegroundColor Gray
        Write-Host "    Path: $vsPath" -ForegroundColor Gray
        Write-Host "    MSBuild: $msbuild" -ForegroundColor Gray
        Write-Host ""
        
        $vsOptions += @{
            Index = $index
            Name = $vsName
            Version = $vsVersion
            Path = $vsPath
            MSBuild = $msbuild
            Toolset = $toolset
        }
        $index++
    }
}

if ($vsOptions.Count -eq 0) {
    Write-Error-Custom "No valid MSBuild installation found."
    exit 1
}

# Let user select
Write-Info "=" * 80
Write-Host "Please select a compiler (1-$($vsOptions.Count)), or press Enter to use the latest: " -NoNewline -ForegroundColor Cyan
$selection = Read-Host

$selectedVS = $null

if ([string]::IsNullOrWhiteSpace($selection)) {
    # Use the first one (usually the latest)
    $selectedVS = $vsOptions[0]
    Write-Info "Using default (latest): $($selectedVS.Name)"
} else {
    $selectionNum = 0
    if ([int]::TryParse($selection, [ref]$selectionNum) -and $selectionNum -ge 1 -and $selectionNum -le $vsOptions.Count) {
        $selectedVS = $vsOptions[$selectionNum - 1]
        Write-Success "Selected: $($selectedVS.Name)"
    } else {
        Write-Error-Custom "Invalid selection. Please enter a number between 1 and $($vsOptions.Count)."
        exit 1
    }
}

$msbuildPath = $selectedVS.MSBuild

Write-Host ""
Write-Info "Using MSBuild: $msbuildPath"
Write-Info "Solution: $SolutionFile"
Write-Info ""

# Define build configurations
# MT = MultiThreaded (Release, Static), MTd = MultiThreadedDebug (Debug, Static)
# MD = MultiThreadedDLL (Release, Dynamic), MDd = MultiThreadedDebugDLL (Debug, Dynamic)
$buildConfigs = @(
    @{Platform="Win32"; PlatformName="x86"; Config="Release"; Runtime="MT"; RuntimeLib="MultiThreaded"},
    @{Platform="Win32"; PlatformName="x86"; Config="Debug"; Runtime="MTd"; RuntimeLib="MultiThreadedDebug"},
    @{Platform="Win32"; PlatformName="x86"; Config="Release"; Runtime="MD"; RuntimeLib="MultiThreadedDLL"},
    @{Platform="Win32"; PlatformName="x86"; Config="Debug"; Runtime="MDd"; RuntimeLib="MultiThreadedDebugDLL"},
    @{Platform="x64"; PlatformName="x64"; Config="Release"; Runtime="MT"; RuntimeLib="MultiThreaded"},
    @{Platform="x64"; PlatformName="x64"; Config="Debug"; Runtime="MTd"; RuntimeLib="MultiThreadedDebug"},
    @{Platform="x64"; PlatformName="x64"; Config="Release"; Runtime="MD"; RuntimeLib="MultiThreadedDLL"},
    @{Platform="x64"; PlatformName="x64"; Config="Debug"; Runtime="MDd"; RuntimeLib="MultiThreadedDebugDLL"}
)

# Projects to build
$projects = @("CppUtils", "Graphics")

# Build counter
$totalBuilds = $buildConfigs.Count * $projects.Count
$currentBuild = 0
$successCount = 0
$failCount = 0

# Clean if requested
if ($Clean) {
    Write-Info "Cleaning build directory..."
    if (Test-Path $BuildDir) {
        Remove-Item -Path "$BuildDir\*.lib" -Force -ErrorAction SilentlyContinue
        Remove-Item -Path "$BuildDir\*.pdb" -Force -ErrorAction SilentlyContinue
    }
    Write-Success "Clean completed.`n"
}

Write-Info "Starting build process..."
Write-Info "Total configurations to build: $totalBuilds"
Write-Info "=" * 80
Write-Info ""

# Create a temporary props file to override RuntimeLibrary
$tempPropsFile = Join-Path $ScriptDir "build_override.props"

$buildResults = @()

foreach ($project in $projects) {
    foreach ($buildConfig in $buildConfigs) {
        $currentBuild++
        
        $platform = $buildConfig.Platform
        $platformName = $buildConfig.PlatformName
        $configuration = $buildConfig.Config
        $runtimeName = $buildConfig.Runtime
        $runtimeLib = $buildConfig.RuntimeLib
        
        # Determine output file name
        $outputFileName = "${project}_${platformName}_${runtimeName}.lib"
        $outputPath = Join-Path $BuildDir $outputFileName
        
        Write-Info "[$currentBuild/$totalBuilds] Building $project | $platformName | $runtimeName"
        Write-Host "  Configuration: $configuration"
        Write-Host "  Platform: $platform"
        Write-Host "  Runtime Library: $runtimeLib"
        Write-Host "  Output: $outputFileName"
        
        # Create props file to override RuntimeLibrary
        $propsContent = @"
<?xml version="1.0" encoding="utf-8"?>
<Project xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <ItemDefinitionGroup>
    <ClCompile>
      <RuntimeLibrary>$runtimeLib</RuntimeLibrary>
    </ClCompile>
  </ItemDefinitionGroup>
  <PropertyGroup>
    <TargetName>${project}_${platformName}_${runtimeName}</TargetName>
    <OutDir>$BuildDir\</OutDir>
    <IntDir>$BuildDir\temp\${project}_${platformName}_${runtimeName}\</IntDir>
  </PropertyGroup>
</Project>
"@
        Set-Content -Path $tempPropsFile -Value $propsContent -Encoding UTF8
        
        # Build the project with custom properties
        $projectFile = Join-Path $ScriptDir "${project}.vcxproj"
        
        $buildArgs = @(
            $projectFile,
            "/t:Rebuild",
            "/p:Configuration=$configuration",
            "/p:Platform=$platform",
            "/p:ForceImportBeforeCppTargets=$tempPropsFile",
            "/p:PreferredToolArchitecture=x64",
            "/v:minimal",
            "/nologo",
            "/m"
        )
        
        $buildStartTime = Get-Date
        
        # Execute build with retry logic for compiler internal errors
        $maxRetries = 3
        $retryCount = 0
        $exitCode = 1
        $output = $null
        
        while ($retryCount -lt $maxRetries -and $exitCode -ne 0) {
            if ($retryCount -gt 0) {
                Write-Host "  Retrying (attempt $($retryCount + 1)/$maxRetries)..." -ForegroundColor Yellow
                Start-Sleep -Seconds 2
            }
            
            # Add CL_MPCount=1 for retries to reduce memory pressure
            if ($retryCount -gt 0) {
                $output = & $msbuildPath $buildArgs "/p:CL_MPCount=1" 2>&1
            } else {
                $output = & $msbuildPath $buildArgs 2>&1
            }
            $exitCode = $LASTEXITCODE
            $retryCount++
            
            # If it's a compiler internal error (C1001, C1060) or D8040, retry
            if ($exitCode -ne 0) {
                $hasInternalError = $output | Where-Object { $_ -match "C1001|C1060|D8040" }
                if (-not $hasInternalError) {
                    # Not an internal error, don't retry
                    break
                }
            }
        }
        
        $buildEndTime = Get-Date
        $buildDuration = $buildEndTime - $buildStartTime
        
        if ($exitCode -eq 0) {
            if (Test-Path $outputPath) {
                $fileSize = (Get-Item $outputPath).Length
                $fileSizeKB = [math]::Round($fileSize / 1KB, 2)
                Write-Success "  ✓ SUCCESS - $outputFileName ($fileSizeKB KB) - Duration: $($buildDuration.TotalSeconds.ToString('0.00'))s"
                $successCount++
                $buildResults += [PSCustomObject]@{
                    Project = $project
                    Platform = $platformName
                    Runtime = $runtimeName
                    Config = $configuration
                    Status = "SUCCESS"
                    Output = $outputFileName
                    Size = "$fileSizeKB KB"
                    Duration = "$($buildDuration.TotalSeconds.ToString('0.00'))s"
                }
            } else {
                Write-Error-Custom "  ✗ FAILED - Output file not found: $outputFileName"
                $failCount++
                $buildResults += [PSCustomObject]@{
                    Project = $project
                    Platform = $platformName
                    Runtime = $runtimeName
                    Config = $configuration
                    Status = "FAILED"
                    Output = "File not found"
                    Size = "N/A"
                    Duration = "$($buildDuration.TotalSeconds.ToString('0.00'))s"
                }
            }
        } else {
            Write-Error-Custom "  ✗ FAILED - Build error (Exit code: $exitCode)"
            $failCount++
            $buildResults += [PSCustomObject]@{
                Project = $project
                Platform = $platformName
                Runtime = $runtimeName
                Config = $configuration
                Status = "FAILED"
                Output = "Build error"
                Size = "N/A"
                Duration = "$($buildDuration.TotalSeconds.ToString('0.00'))s"
            }
            # Show last few lines of error output
            $errorLines = $output | Where-Object { $_ -match "error" -or $_ -match "fatal" } | Select-Object -Last 5
            if ($errorLines) {
                $errorLines | ForEach-Object {
                    Write-Host "    $_" -ForegroundColor DarkRed
                }
            }
        }
        
        Write-Host ""
    }
}

# Clean up temporary props file
if (Test-Path $tempPropsFile) {
    Remove-Item $tempPropsFile -Force
}

Write-Info "=" * 80
Write-Info "Build Summary"
Write-Info "=" * 80
Write-Host ""

# Display results table
$buildResults | Format-Table -AutoSize

Write-Host ""
Write-Info "Total Builds: $totalBuilds"
Write-Success "Successful: $successCount"
if ($failCount -gt 0) {
    Write-Error-Custom "Failed: $failCount"
} else {
    Write-Success "Failed: $failCount"
}

Write-Host ""
Write-Info "Build output directory: $BuildDir"

# List all generated files
if (Test-Path $BuildDir) {
    Write-Host ""
    Write-Info "Generated library files:"
    Get-ChildItem -Path $BuildDir -Filter "*.lib" | Sort-Object Name | ForEach-Object {
        $fileSize = [math]::Round($_.Length / 1KB, 2)
        Write-Host "  $($_.Name) - $fileSize KB"
    }
}

# Exit with appropriate code
if ($failCount -gt 0) {
    exit 1
} else {
    exit 0
}
