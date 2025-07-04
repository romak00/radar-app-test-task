$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$projectRoot = Split-Path $scriptDir -Parent

$serverExe = Join-Path $projectRoot "bin\win-static\radar_server.exe"
$clientExe = Join-Path $projectRoot "bin\win-static\radar_ui.exe"

if (-Not (Test-Path $serverExe)) {
    Write-Error "Server not found: $serverExe"
    exit 1
}
if (-Not (Test-Path $clientExe)) {
    Write-Error "Client not found: $clientExe"
    exit 1
}

Write-Host "Starting server: $serverExe"
$serverProc = Start-Process -FilePath $serverExe -WindowStyle Normal -PassThru

Start-Sleep -Milliseconds 500

Write-Host "Starting UI: $clientExe"
Start-Process -FilePath $clientExe -WindowStyle Hidden

Write-Host "==> Both server and client running. Server PID=$($serverProc.Id)"
