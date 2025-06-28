Write-Host "==> Windows build started."

cmake --preset "win-static"
cmake --build --preset "win-static" -- /m

Write-Host "==> Windows build finished."
