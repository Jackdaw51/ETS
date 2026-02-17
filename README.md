# Embedded Arcade Project — MSP432 + LCD + ESP32

This repository contains the final project developed for the *Embedded Systems for the Internet of Things* course.

The project targets the **TI MSP-EXP432P401R LaunchPad (MSP432P401R)** and runs a set of arcade-style mini-games on a **160×128 SPI LCD**. User input is provided through an **analog joystick** sampled via **ADC14 interrupts**. The display refresh is optimized using **SPI + DMA** with **ping-pong buffering** for higher frame rates. The system tick is generated using **hardware timers**, with calibrated delays used where required (e.g., LCD reset timing).

An **ESP32** module is connected via **UART** to support command-based communication and optional online features (see the `server/` folder if enabled in your setup). A PC-side setup may also be available to run and test the same codebase without the physical board, depending on the provided display backend.

---

## 1) Requirements (What is needed to run the project)

### Hardware Requirements
- **Development board:** Texas Instruments **MSP-EXP432P401R LaunchPad**
- **MCU:** **MSP432P401R** (ARM Cortex-M4F)

- **Display:** **160×128 LCD**
  - Rendering uses an in-RAM **2 bpp** framebuffer (`frame_buffer[5120]`) plus a per-pixel **4-bit** palette buffer (`palette_buffer[10240]`)
  - At the end of each frame, the display backend triggers a transfer via `DMA_send_frame(frame_buffer, palette_buffer)` (DMA-based LCD update path)
  - A **1024-byte** DMA staging buffer is allocated (`dma_buffer[1024]`)

- **Input:** **Analog joystick + button**
  - X/Y sampling through **ADC14** in multi-sequence mode (MEM0..MEM1) with completion via **interrupt**
  - Low-power wait until conversion completes (`__WFI()`)
  - Pins:
    - X-axis: **P6.0**
    - Y-axis: **P4.4**
    - Button: **P4.1** (input with pull-up)

- **Connectivity:** **ESP32** via **UART (eUSCI_A2)**
  - UART pins: **P3.2 (RX)**, **P3.3 (TX)**
  - Baud rate: **115200** (SMCLK @ 24 MHz)
  - Interrupt-driven RX; newline-terminated (`\n`) messages buffered in a 128-byte RX buffer

- **LED:** On-board LED (debug/status)

#### MCU peripherals used
- **ADC14 (interrupt):** analog joystick sampling (P6.0, P4.4)
- **UART (eUSCI_A2, RX interrupt):** communication with ESP32 @ 115200 baud (P3.2 RX, P3.3 TX)
- **DMA:** used by the LCD update path (frame transfer)

> Note: a proximity sensor interface exists (`get_proximity()`), but in the shown file it is currently a **stub** (artificial increment) and the real call is commented out (`trigger_hcsr04()`). For this reason, the README describes proximity as “supported/designed for” unless the real driver is included.

---

### Software Requirements

#### MCU (build/flash)
- **TI Code Composer Studio (CCS):** **v12.8.0**
- CCS integrated toolchain (course setup)
- USB connection to the LaunchPad for programming/debugging
- Install driverlib. To do that:
1. Download the simplelink_msp432p4_sdk_3_40_01_02.zip file from TI official website. 
2. Open CSS and left click on Project Folder to select Properties
3. Select CSS Build
4. Click ARM Compiler and then Include Options
4.1 Add "simplelink_msp432p4_sdk_3_40_01_02/source" directory to "Add dir to #include search path" window.  
5. Click ARM Linker and File Search Path
5.1 Add "simplelink_msp432p4_sdk_3_40_01_02/source/ti/devices/msp432p4xx/driverlib/ccs/msp432p4xx_driverlib.lib" to "Include library file..." window


#### Server (online scoreboard)
- **Python 3.x**
- Python packages:
  - `flask`
  - `paho-mqtt`
- Local MQTT broker:
  - `mosquitto`
  - `mosquitto-clients`


## 2) Project Layout

```text
.
├── games/                 # Embedded application (games + display + sprites)
│   ├── game_files/        # Main source code
│   │   ├── display/       # Display abstraction + MCU LCD backend (SPI/DMA) + utilities
│   │   ├── sprites/       # Sprite runtime + palettes + asset conversion tools + PNG sources
│   │   └── *.c/*.h        # Games (Dino, Pong, Snake, Space Invaders) + menu + entry points
│   └── game_exes/         # built executables
│
├── server/                # Optional scoreboard server (MQTT + Flask) + web UI templates
│   └── templates/         # HTML templates for Flask
│
├── incl/                  # Project-wide shared headers/includes
├── targetConfigs/         # Code Composer Studio target configs (flash/debug)
│
├── Debug/                 # Generated build output (IDE)
└── .idea/ .vscode/ ...    # IDE/editor settings (not required)

```
--- 

## 3) How to build, burn (flash) and run

### 3.1 Build & Flash on the MCU (Code Composer Studio)

This project follows the standard embedded workflow: the **host system** (your PC running CCS) builds the firmware image, then the image is **downloaded to the target** (MSP432 LaunchPad) through the on-board programmer/debugger. The same connection can also be used for debugging.

#### Prerequisites
- Install **TI Code Composer Studio (CCS) v12.8.0**
- Connect the **MSP-EXP432P401R LaunchPad** to the PC via USB

#### Steps (CCS)
1. Open **CCS**.
2. Import/open the project workspace.
3. Select the correct target configuration from `targetConfigs/` (flash/debug).
4. Build the project (**Project → Build Project**).
5. Flash & run:
  - **Debug**: start a debug session (CCS programs the MCU and runs the firmware).
  - **Run** (if configured): flash and start execution without stepping.

> Notes:
> - Firmware is stored in the MCU **Flash**, while runtime data is stored in **SRAM**.
> - The on-board LED is available for quick sanity checks (GPIO).

---

### 3.2 Run the Scoreboard Server (MQTT + Flask)

The `server/` folder contains a Python server that:
- subscribes to MQTT score messages (topic `esp32/score`),
- stores scores persistently in `scores.json`,
- serves a small web page and an API endpoint for scores.

#### IoT Score Reporting (MSP432 → ESP32 → MQTT → Python Server)
At the end of each match, the **MSP432** sends a structured packet to the **ESP32** over **UART**.  
The ESP32 then publishes data using **MQTT** to a **Mosquitto broker** running on the PC.  
A **Flask-based Python server** subscribes to MQTT topics and records scores, exposing them via a web UI and a JSON API.

MQTT topics:
- `esp32/score` — match scores (includes player name, score value, and game identifier/type)
- *(optional / if enabled)* `esp32/status` — connection/status messages (handshake)
- *(optional / if enabled)* `esp32/ack` — server acknowledgement

#### Install dependencies (Linux/WSL example)
```bash
pip install paho-mqtt flask
sudo apt install mosquitto mosquitto-clients

```
Start the local MQTT broker
```bash
cd server
chmod +x broker.sh
./broker.sh
```

Run the Python server
```bash
cd server
python3 server.py
```

If you are using a Python virtual environment (useful e.g. on WSL):
```bash
cd server
python3 -m venv venv
source venv/bin/activate
pip install paho-mqtt flask
python3 server.py
```
Endpoints

- Web UI: http://localhost:5000/

- Scores (JSON): http://localhost:5000/scores
---
## 4) User Guide

### 4.1 Controls (MCU)

The project is designed to run on the target MCU board. Input is provided through an **analog joystick**.

- **Move (Up/Down/Left/Right):** used to navigate menus and to control the games (depending on the game).
- **Joystick press:** used as **SELECT / CONFIRM** (menu selection and in-menu confirmations).

---

### 4.2 Start the project (Menu)

At startup the firmware shows a **menu** on the LCD.

1. Use the joystick directions to navigate between games.
2. Press the joystick to **select** and start the highlighted game.

After any game ends, the project enters the **score publishing flow** (see below) and then returns to the menu.

---

### 4.3 End of game: Online score publishing flow

At the end of a match, the user is asked whether they want to **publish the score online**.

- If **No**: the project returns directly to the **menu**.
- If **Yes**:
  1. the user is asked to enter a **player name**,
  2. the project sends the score to the server together with the **game identifier/type**,
  3. the project returns to the **menu**.

On the server side, scores are **grouped by game** (each game has its own leaderboard/list).

---

### 4.4 Games

#### Dino Runner
**Goal:** survive as long as possible by avoiding obstacles.  
**Gameplay:** the character runs automatically; the player reacts to obstacles.

Typical controls:
- **Jump / Avoid**: joystick up
- **Duck / Lower**: joystick down

Game Over:
- collision with an obstacle.

> After game over, the **online score publishing flow** is shown, then the project returns to the menu.
---

#### Pong Wall
**Goal:** keep the ball in play by moving the paddle.  
**Gameplay:** you control a paddle and bounce the ball back.

Typical controls:
- **Move paddle**: joystick left/right 
Game Over:
- the ball passes the paddle.

> After game over, the **online score publishing flow** is shown, then the project returns to the menu.
---

#### Snake
**Goal:** grow the snake by collecting items while avoiding collisions.  
**Gameplay:** the snake moves continuously; the player changes direction.

Controls:
- **Change direction**: joystick up/down/left/right

Game Over:
- collision with walls and/or the snake body (depending on rules).

> After the game ends (win or lose), the **online score publishing flow** is shown, then the project returns to the menu.
---

#### Space Invaders
**Goal:** destroy all aliens and avoid being hit.  
**Gameplay:** move horizontally and shoot enemies.

Typical controls:
- **Move**: joystick left/right

Game Over:
- player is hit or enemies reach the player area.

> After game over, the **online score publishing flow** is shown, then the project returns to the menu.
---


## 5) Youtube presentation video

```

```

---

## 6) Team Members and Contributions

| Member            | Contributions (main tasks/features) | Main files/folders                             |
|-------------------|--------------|------------------------------------------------|
| Malchiodi Massimo | games        | dino_runner.c , pong_wall.c , space_invaders.c |
| Francesco Bogni   |              |                                                |
| Rowan Li          |              |                                                |
| Leonardo Sandrini |              |                                                |
