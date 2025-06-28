Write-Host "==> Windows static build started."

cmake --preset "win"
cmake --build --preset "win" -- /m

Write-Host "==> Windows static build finished."