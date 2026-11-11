# For Visual Studio 2026 users
$ErrorActionPreference='Stop'
Set-Location $PSScriptRoot
cmake -S . -B build-win -G 'Visual Studio 18 2026' -A x64 -DSUNDRIFT_WITH_NAM=ON
if ($LASTEXITCODE -ne 0) { throw 'CMake configuration failed' }
cmake --build build-win --config Release --target Sundrift_VST3 -j 4
if ($LASTEXITCODE -ne 0) { throw 'Build failed' }
Get-ChildItem -Path build-win -Recurse -Directory -Filter Sundrift.vst3 | Select-Object -ExpandProperty FullName
