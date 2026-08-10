# Captures loopback UDP on 9000/9001/9100 to see PaperTracker OSC data.
# Run in ADMIN PowerShell:  powershell -ExecutionPolicy Bypass -File cap_osc.ps1
$dur = 12
$etl = "C:\Users\wzy\AppData\Local\Temp\osc_cap.etl"
Write-Host "Stopping old capture..."
netsh trace stop 2>$null | Out-Null
Start-Sleep -Milliseconds 800
Write-Host "Starting capture ${dur}s (loopback 9000/9001/9100)..."
netsh trace start capture=yes report=no maxsize=100 persist=no overwrite=yes tracefile=$etl
Start-Sleep -Seconds $dur
Write-Host "Stopping..."
netsh trace stop
Write-Host "=== DONE. ETL: $etl size: $((Get-Item $etl -ErrorAction SilentlyContinue).Length) bytes"
