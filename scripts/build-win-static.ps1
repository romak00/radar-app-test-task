Write-Host "==> Windows static build started."

cmake --preset "win-static"
cmake --build --preset "win-static" -- /m

Write-Host "==> Windows static build finished."
