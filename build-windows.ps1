# Run from PowerShell with Visual Studio 2022/2026 C++ tools, CMake, and Git installed.
$ErrorActionPreference = 'Stop'
Set-Location $PSScriptRoot
Write-Host 'Configuring Sundrift VST3 (this downloads JUCE and NAM A2 source on first run)...'
cmake -S . -B build-win -G 'Visual Studio 17 2022' -A x64 -DSUNDRIFT_WITH_NAM=ON
if ($LASTEXITCODE -ne 0) { throw 'CMake configuration failed' }
cmake --build build-win --config Release --target Sundrift_VST3 -j 4
if ($LASTEXITCODE -ne 0) { throw 'VST3 build failed' }
$built = Get-ChildItem -Path build-win -Directory -Recurse -Filter Sundrift.vst3 | Select-Object -First 1
if (-not $built) { throw 'Build finished, but Sundrift.vst3 bundle was not located' }
$dest = Join-Path $env:CommonProgramFiles 'VST3\Sundrift.vst3'
Write-Host "Built plugin: $($built.FullName)"
Write-Host "Install by copying the .vst3 folder to: $dest"
Write-Host 'Your DAW may need to rescan VST3 plugins.'
