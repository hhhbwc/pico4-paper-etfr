$u = New-Object System.Net.Sockets.UdpClient
try {
  $u.Client.Bind((New-Object System.Net.IPEndPoint([Net.IPAddress]::Loopback, 9100)))
  Write-Host "bound 9100 OK"
  $u.Client.ReceiveTimeout = 8000
  $ep = New-Object System.Net.IPEndPoint([Net.IPAddress]::Any, 0)
  $data = $u.Receive([ref]$ep)
  Write-Host "RECV $($data.Length) bytes from $ep"
  $bytes = $data | Select-Object -First 48
  $hex = ""
  foreach ($b in $bytes) { $hex += $b.ToString('x2') + ' ' }
  Write-Host "hex: $hex"
  $chars = $data | Select-Object -First 96
  $asc = ""
  foreach ($b in $chars) { if ($b -ge 32 -and $b -lt 127) { $asc += [char]$b } else { $asc += '.' } }
  Write-Host "ascii: $asc"
} catch {
  Write-Host "no data (timeout or err): $($_.Exception.Message)"
}
$u.Close()
