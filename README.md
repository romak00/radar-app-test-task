# Сборка и запуск

## Зависимости

- **CMake** ≥ 3.16  
- **C++ компилятор** с поддержкой C++20  
- **Qt 5.15+** (Widgets, Network, Core, Gui, OpenGL)  
- **Asio** (standalone, в `thirdparty/asio/include`)  
- **Eigen** (в `thirdparty/eigen/eigen-master`)

## Windows

```powershell
# Сборка
cd <project_root>
./scripts/build-win.ps1

# Запуск (сервер и GUI)
./scripts/run-win.ps1
```

- Папка билдов: `bin/win/`  
- Запускаются `radar_server.exe` и `radar_ui.exe` (GUI без консоли).

### Запуск на чистом ПК
Для этого нужно собрать при помощи `./scripts/build-win-static.ps1` указав при этом в `CMakePresets.json` в данном пресете путь до статически собранной Qt. Также уже собранные бинарники лежат в `/bin/win-static/` и в релизе.

## Linux/macOS

```bash
# Сборка
cd <project_root>
bash scripts/build-linux.sh

# Запуск (сервер и GUI)
./scripst/run-unix.sh
```

- Папка билдов: `bin/unix/`

---

# Нереализовано / ограничения

- Под Linux/macOS окно не frameless.
- Под Windows оно frameless, но не перетаскивается (но меняется размер).
- Цвета целей генерируются случайно, могут сливаться с фоном.
