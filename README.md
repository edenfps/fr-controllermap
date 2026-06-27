# FR Controllermap

Xbox controller support for Free Realms on PC. A DLL that hooks the game's input system to translate XInput controller input into keyboard and mouse actions — giving you native-feeling gamepad control.

## Features

- **Analog left stick** — Smooth WASD movement with walk-weighting for partial tilt
- **Analog right stick** — Camera control (press R3 / right stick click to toggle look mode)
- **Full button mapping** — Jump, Interact, Tab target, Auto run, action bar, docks, and more
- **Synthetic right mouse button** — Camera look mode works seamlessly with the game's hold-right-mouse-to-look system
- **Configurable deadzones** — Separate deadzones for left and right sticks
- **Adjustable camera sensitivity** — Tune right-stick camera speed
- **INIConfig file** — All bindings adjustable via `FRController.ini`
- **Analog movement memory patch** — Optional direct-memory analog movement for true 360° control (requires address scan)

## Controller Layout

| Control | Default Action |
|---------|---------------|
| Left Stick | Move (WASD); partial tilt = walk |
| Right Stick | Camera (when look mode is on) |
| R3 (Right Stick Click) | Toggle camera look / cursor mode |
| A | Jump |
| X | Interact |
| Y | Tab target |
| B | Auto run |
| LB | Alternate action bar (tilde) |
| RB | Inventory |
| LT | Action bar slot 1 |
| RT | Action bar slot 2 |
| D-pad Up | Atlas |
| D-pad Left | Character |
| D-pad Down | Jobs |
| D-pad Right | Quests |
| Back | Action bar slot 9 |
| Start | Escape / Menu |

## Installation

1. [Download the latest `version.dll`](https://github.com/edenfps/fr-controllermap/releases) (or build from source)
2. Place `version.dll` and [`FRController.ini`](src/FRController.ini) next to `FreeRealms.exe` in your client folder
3. Launch Free Realms — the DLL loads automatically

To disable: set `Enabled=0` in `FRController.ini` or remove `version.dll`.

## Building from Source

Requires Visual Studio 2022 Build Tools (or any VS edition with C++ x86 compiler):

```bash
build.bat
```

Outputs `Client/version.dll`.

## Configuration (`FRController.ini`)

```ini
[General]
Enabled=1                  ; 0 to disable
ControllerIndex=0          ; XInput player index (0-3)
Deadzone=0.20              ; Right stick deadzone
MoveDeadzone=0.12          ; Left stick deadzone

[Camera]
Sensitivity=10.0           ; Higher = faster camera
InvertY=0                  ; 1 to invert vertical
RequireRightMouseButton=1  ; 1 = hold-right-mouse camera mode

[AnalogMovement]
UseMemory=0                ; 1 to enable direct-memory analog movement
MoveForwardAddress=0       ; Hex address for forward axis
StrafeAddress=0            ; Hex address for strafe axis
SpeedScaleAddress=0        ; Hex address for movement speed scale

[Bindings]
; Virtual-key codes in decimal. Defaults match standard Free Realms bindings.
MoveForward=87
MoveBackward=83
StrafeLeft=65
StrafeRight=68
Jump=32
Interact=69
; ... (see FRController.ini for full list)
```

### Enabling Analog Movement (Memory Mode)

The left stick normally sends WASD key presses (8-directional digital movement). For true 360° analog movement:

1. Run `tools/find_movement_addresses.py` to scan for movement addresses in the Free Realms client
2. Copy the found addresses into the `[AnalogMovement]` section of `FRController.ini`
3. Set `UseMemory=1`

## How It Works

`version.dll` is a proxy DLL loaded by the Free Realms client at startup. It hooks three Windows API functions:

- `GetAsyncKeyState` — Injects gamepad button presses as keyboard input
- `GetKeyState` — Synthetic right mouse button for camera look mode
- `GetKeyboardState` — Batch keyboard state injection

The hooks are installed via IAT (Import Address Table) patching on `USER32.dll` imports, so they only affect the Free Realms process. No global hooks or system-wide changes.

## Project Structure

```
fr-controllermap/
├── build.bat              # Build script
├── src/
│   ├── dllmain.cpp        # DLL entry point
│   ├── hooks.cpp          # IAT hook installation & hooked API functions
│   ├── hooks.h
│   ├── input_state.cpp    # XInput polling & synthetic key generation
│   ├── input_state.h
│   ├── config.cpp         # INI config reader
│   ├── config.h
│   ├── version.def        # DLL export definitions
│   ├── version.dll        # Pre-built DLL (in Client/)
│   └── FRController.ini   # Default configuration
└── tools/
    ├── find_movement_addresses.py  # Memory scanner for analog movement
    ├── analyze_input.py            # Input analysis/debugging
    └── test32.c                    # Input test utility
```

## License

AGPL-3.0
