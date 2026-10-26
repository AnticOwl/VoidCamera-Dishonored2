$ErrorActionPreference = 'Stop'
$project = Split-Path -Parent $PSScriptRoot
$destination = Join-Path $project 'dist\VoidCamera_1.16'
New-Item -ItemType Directory -Force -Path $destination | Out-Null
Copy-Item -LiteralPath (Join-Path $project 'bin\v1.16\Release\VoidCamera.dll') -Destination $destination
Copy-Item -LiteralPath (Join-Path $project 'bin\v1.16\Release\VoidCameraInjector.exe') -Destination $destination
Copy-Item -LiteralPath (Join-Path $project 'README.md') -Destination $destination
Copy-Item -LiteralPath (Join-Path $project 'tools\Dishonored2_PostFX_Retail_1.77.9.0.CT') -Destination $destination
Copy-Item -LiteralPath (Join-Path $project 'third_party\imgui\LICENSE.txt') -Destination (Join-Path $destination 'LICENSE-ImGui.txt')
Copy-Item -LiteralPath (Join-Path $project 'third_party\minhook\LICENSE.txt') -Destination (Join-Path $destination 'LICENSE-MinHook.txt')
Get-FileHash -Algorithm SHA256 -LiteralPath (Join-Path $destination 'VoidCamera.dll'),(Join-Path $destination 'VoidCameraInjector.exe') | ForEach-Object { '{0}  {1}' -f $_.Hash,(Split-Path -Leaf $_.Path) } | Set-Content -LiteralPath (Join-Path $destination 'SHA256SUMS.txt') -Encoding ascii
Compress-Archive -LiteralPath $destination -DestinationPath (Join-Path $project 'dist\VoidCamera_1.16.zip') -Force
