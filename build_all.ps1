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

# Find MSBuild
$msbuildPath = $null
$vsWhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"

if (Test-Path $vsWhere) {
    $vsPath = & $vsWhere -latest -products * -requires Microsoft.Component.MSBuild -property installationPath
    if ($vsPath) {
        $msbuildPath = Join-Path $vsPath "MSBuild\Current\Bin\MSBuild.exe"
        if (-not (Test-Path $msbuildPath)) {
            $msbuildPath = Join-Path $vsPath "MSBuild\15.0\Bin\MSBuild.exe"
        }
    }
}

if (-not $msbuildPath -or -not (Test-Path $msbuildPath)) {
    Write-Error-Custom "MSBuild not found. Please install Visual Studio or Build Tools."
    exit 1
}

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
