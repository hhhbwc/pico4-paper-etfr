$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$stage = Join-Path $root 'module'
$bin = Join-Path $stage 'system/bin'
$zygisk = Join-Path $stage 'zygisk'
New-Item -ItemType Directory -Force $bin, $zygisk | Out-Null
Copy-Item (Join-Path $root 'build-android/paper_bridge_daemon') (Join-Path $bin 'paper_bridge_daemon') -Force
Copy-Item (Join-Path $root 'build-android/libpaper_bridge_hook_stub.so') (Join-Path $zygisk 'arm64-v8a.so') -Force
$zip = Join-Path $root 'paper-pico-bridge-safe-skeleton.zip'
if (Test-Path $zip) { Remove-Item $zip -Force }
Compress-Archive -Path (Join-Path $stage '*') -DestinationPath $zip
Write-Host "Package: $zip"
