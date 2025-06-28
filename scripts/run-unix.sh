#!/usr/bin/bash

set -euo pipefail

script_dir = "$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
project_root = "$(dirname "$script_dir")"
bin_dir = "$project_root/bin/unix"

server_exe = "$bin_dir/radar_server"
ui_exe = "$bin_dir/radar_ui"

if [[ ! -x "$server_exe" ]]; then
  echo "Error: server not found or not executable: $server_exe" >&2
  exit 1
fi
if [[ ! -x "$ui_exe" ]]; then
  echo "Error: UI not found or not executable: $ui_exe" >&2
  exit 1
fi

echo "Starting server: $server_exe"
"$server_exe" &
server_pid=$!

sleep 0.5

echo "Starting UI: $ui_exe"
"$ui_exe" &

echo "==> Bot Server and UI running (server PID = $server_pid)."