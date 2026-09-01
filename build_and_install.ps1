<#
.SYNOPSIS
    Builds, installs, or uninstalls the Music Player SDK & Synthesizer Application.

.DESCRIPTION
    Compiles shared libraries, creates C++ SDK headers and import libraries,
    and deploys or removes the desktop application, business logic layer, and web UI assets.

.PARAMETER Action
    Action to perform: Build, Install, Rebuild, Clean, or Uninstall (Default: Install).

.PARAMETER Config
    Build configuration: Debug, Release, or RelWithDebInfo (Default: Release).

.PARAMETER InstallPrefix
    Destination installation directory (Default: ./install).

.PARAMETER BuildDir
    Custom build output directory (Default: ./build).

.PARAMETER RunAppAfterInstall
    Runs the installed app upon successful build/installation.

.PARAMETER Help
    Displays the help usage and examples.

.EXAMPLE
    .\build_and_install.ps1 -Help
    .\build_and_install.ps1 -Action Install -Config Release -InstallPrefix "C:\Program Files\SMusicSystem"
    .\build_and_install.ps1 -Action Uninstall -InstallPrefix "C:\Program Files\SMusicSystem"
    .\build_and_install.ps1 -Action Rebuild -RunAppAfterInstall
#>

[CmdletBinding()]
param (
    [ValidateSet("Build", "Install", "Rebuild", "Clean", "Uninstall")]
    [string]$Action = "Install",

    [ValidateSet("Debug", "Release", "RelWithDebInfo")]
    [string]$Config = "Release",

    [string]$InstallPrefix = "$PSScriptRoot/install",

    [string]$BuildDir = "$PSScriptRoot/build",

    [switch]$RunAppAfterInstall,

    [Alias("h", "-h", "-help")]
    [switch]$Help
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

function Show-HelpMessage {
    Write-Host @"
========================================================================
 Music Player SDK & Synthesizer Automation Engine
========================================================================

Usage:
  .\build_and_install.ps1 [-Action <Action>] [-Config <Config>] 
                          [-InstallPrefix <Path>] [-BuildDir <Path>]
                          [-RunAppAfterInstall] [-Help]

Actions (-Action):
  Build         Configure and compile all targets (SDK libs + App).
  Install       Build targets and deploy to destination folder (Default).
  Rebuild       Wipe build directory, recompile from scratch, and install.
  Clean         Delete build artifacts and local install directory.
  Uninstall     Remove installed files using install_manifest.txt.

Options:
  -Config               Build configuration: Debug | Release | RelWithDebInfo (Default: Release).
  -InstallPrefix        Destination install directory (Default: ./install).
  -BuildDir             CMake build directory (Default: ./build).
  -RunAppAfterInstall   Launch the app executable immediately upon completion.
  -Help, -h, --h, --help Display this help menu.

Examples:
  # Standard build and install to default ./install
  .\build_and_install.ps1

  # Install to Program Files as Administrator
  .\build_and_install.ps1 -Action Install -InstallPrefix "C:\Program Files\SMusicSystem"

  # Clean Rebuild in Debug mode and run immediately
  .\build_and_install.ps1 -Action Rebuild -Config Debug -RunAppAfterInstall

  # Uninstall deployed application and SDK
  .\build_and_install.ps1 -Action Uninstall -InstallPrefix "C:\Program Files\SMusicSystem"

  # Wipe build folder
  .\build_and_install.ps1 -Action Clean
========================================================================
"@ -ForegroundColor Cyan
}

# Intercept help flags
if ($Help -or ($args -contains "--h") -or ($args -contains "--help") -or ($args -contains "-h") -or ($args -contains "-help")) {
    Show-HelpMessage
    exit 0
}

Write-Host "==========================================================" -ForegroundColor Cyan
Write-Host " Music Player SDK & Synthesizer Automation Engine ($Action)" -ForegroundColor Cyan
Write-Host "==========================================================" -ForegroundColor Cyan

$absoluteInstallPrefix = [System.IO.Path]::GetFullPath($InstallPrefix)
$manifestPath = Join-Path $BuildDir "install_manifest.txt"

function Assert-AdminPrivileges {
    $isAdmin = ([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
    if (-not $isAdmin -and ($absoluteInstallPrefix -like "$env:ProgramFiles*" -or $absoluteInstallPrefix -like "$env:SystemDrive\*")) {
        Write-Warning "Target path '$absoluteInstallPrefix' requires Administrator privileges. Run PowerShell as Administrator if writes fail."
    }
}

function Assert-CMakeInstalled {
    $cmakePath = Get-Command "cmake" -ErrorAction SilentlyContinue
    if (-not $cmakePath) {
        Write-Error "CMake is not found in PATH. Please install CMake or add it to system PATH."
    }
}

switch ($Action) {
    "Clean" {
        Write-Host "==> Cleaning build and local install artifacts..." -ForegroundColor Yellow
        if (Test-Path $BuildDir) {
            Remove-Item -Path $BuildDir -Recurse -Force
            Write-Host "  - Removed: $BuildDir" -ForegroundColor Gray
        }
        if (Test-Path $absoluteInstallPrefix) {
            Remove-Item -Path $absoluteInstallPrefix -Recurse -Force
            Write-Host "  - Removed: $absoluteInstallPrefix" -ForegroundColor Gray
        }
        Write-Host "Clean completed successfully." -ForegroundColor Green
    }

    "Uninstall" {
        Assert-AdminPrivileges
        Write-Host "==> Uninstalling files deployed to: $absoluteInstallPrefix..." -ForegroundColor Yellow

        if (-not (Test-Path $manifestPath)) {
            Write-Error "Cannot find install manifest at '$manifestPath'. Ensure the project has been built/installed before uninstalling."
        }

        $installedFiles = Get-Content $manifestPath
        foreach ($file in $installedFiles) {
            if (Test-Path $file) {
                Write-Host "  - Removing: $file" -ForegroundColor Gray
                Remove-Item $file -Force -ErrorAction SilentlyContinue
            }
        }

        # Clean empty folders left behind (from deepest to root)
        $subDirs = @(
            "$absoluteInstallPrefix\lib\cmake\MusicPlayerSDK",
            "$absoluteInstallPrefix\lib\cmake",
            "$absoluteInstallPrefix\lib",
            "$absoluteInstallPrefix\include\MusicBuilderBL",
            "$absoluteInstallPrefix\include\MusicInstrument",
            "$absoluteInstallPrefix\include\MusicPlayerSystem",
            "$absoluteInstallPrefix\include",
            "$absoluteInstallPrefix\bin",
            "$absoluteInstallPrefix"
        )

        foreach ($dir in $subDirs) {
            if ((Test-Path $dir) -and (Get-ChildItem -Path $dir -Recurse -Force -ErrorAction SilentlyContinue | Measure-Object).Count -eq 0) {
                Write-Host "  - Removing empty folder: $dir" -ForegroundColor Gray
                Remove-Item -Path $dir -Force -Recurse -ErrorAction SilentlyContinue
            }
        }

        Write-Host "Uninstallation completed successfully." -ForegroundColor Green
    }

    Default { # Build, Install, or Rebuild
        Assert-CMakeInstalled
        Assert-AdminPrivileges

        if ($Action -eq "Rebuild") {
            Write-Host "[1/3] Rebuild requested: Wiping build directory..." -ForegroundColor Yellow
            if (Test-Path $BuildDir) { Remove-Item -Path $BuildDir -Recurse -Force }
        }

        # 1. Configure CMake
        Write-Host "[1/3] Configuring Project ($Config)..." -ForegroundColor Green
        $cmakeArgs = @(
            "-S", $PSScriptRoot,
            "-B", $BuildDir,
            "-DCMAKE_BUILD_TYPE=$Config",
            "-DCMAKE_INSTALL_PREFIX=$absoluteInstallPrefix"
        )

        if (Get-Command "ninja" -ErrorAction SilentlyContinue) {
            $cmakeArgs += "-G", "Ninja"
        }

        & cmake @cmakeArgs

        # 2. Build Targets
        Write-Host "[2/3] Compiling Modules, Business Logic Layer, and Application..." -ForegroundColor Green
        & cmake --build $BuildDir --config $Config --parallel

        # 3. Install if Action is Install or Rebuild
        if ($Action -eq "Install" -or $Action -eq "Rebuild") {
            Write-Host "[3/3] Deploying Install Package to: $absoluteInstallPrefix" -ForegroundColor Green
            & cmake --install $BuildDir --config $Config

            Write-Host ""
            Write-Host "==========================================================" -ForegroundColor Cyan
            Write-Host " Build & Installation Completed Successfully!" -ForegroundColor Green
            Write-Host "==========================================================" -ForegroundColor Cyan
            Write-Host "Installed Layout:" -ForegroundColor White
            Write-Host "  - Executable:   $absoluteInstallPrefix\bin\MusicPlayerTesterApp.exe" -ForegroundColor Gray
            Write-Host "  - UI Frontend:  $absoluteInstallPrefix\bin\index.html" -ForegroundColor Gray
            Write-Host "  - Shared Libs:  $absoluteInstallPrefix\bin\*.dll" -ForegroundColor Gray
            Write-Host "  - SDK Headers:  $absoluteInstallPrefix\include\" -ForegroundColor Gray
            Write-Host "  - SDK Libs:     $absoluteInstallPrefix\lib\*.lib / *.a" -ForegroundColor Gray
            Write-Host "  - CMake SDK:    $absoluteInstallPrefix\lib\cmake\MusicPlayerSDK\" -ForegroundColor Gray
            Write-Host ""

            if ($RunAppAfterInstall) {
                $exePath = "$absoluteInstallPrefix\bin\MusicPlayerTesterApp.exe"
                if (Test-Path $exePath) {
                    Write-Host "Launching Installed Synthesizer Application..." -ForegroundColor Yellow
                    Start-Process -FilePath $exePath -WorkingDirectory (Split-Path $exePath)
                }
            }
        }
    }
}