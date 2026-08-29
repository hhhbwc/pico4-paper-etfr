$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
if (-not $env:ANDROID_NDK_HOME) {
  throw 'Set ANDROID_NDK_HOME to an Android NDK root before building.'
}
$ndk = $env:ANDROID_NDK_HOME
cmake -S $root -B (Join-Path $root 'build-android') -G Ninja `
  -DCMAKE_TOOLCHAIN_FILE=(Join-Path $ndk 'build/cmake/android.toolchain.cmake') `
  -DANDROID_ABI=arm64-v8a -DANDROID_PLATFORM=android-29 -DCMAKE_BUILD_TYPE=Release
cmake --build (Join-Path $root 'build-android') --parallel
Write-Host 'Android arm64 build complete.'
