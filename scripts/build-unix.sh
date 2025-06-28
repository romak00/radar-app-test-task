#!/usr/bin/bash

echo "==> Unix build started."

cmake --preset "unix"
cmake --build --preset "unix" -- -j$(nproc)

echo "==> Unix build finished."