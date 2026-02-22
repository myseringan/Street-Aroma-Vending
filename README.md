# Street Aroma — Perfume Vending Machine

**Automated perfume dispenser with touchscreen UI, QR payment, and remote management**

[![ESP32](https://img.shields.io/badge/ESP32-Firmware-green?style=flat-square&logo=espressif)](https://www.espressif.com/)
[![LVGL](https://img.shields.io/badge/LVGL-v8.3-red?style=flat-square)](https://lvgl.io/)
[![Python](https://img.shields.io/badge/Python-Flask-blue?style=flat-square&logo=python)](https://python.org/)
[![MQTT](https://img.shields.io/badge/MQTT-HiveMQ-purple?style=flat-square&logo=mqtt)](https://www.hivemq.com/)
[![Payme](https://img.shields.io/badge/Payme-Payment-00B2FF?style=flat-square)](https://payme.uz/)
[![License](https://img.shields.io/badge/License-MIT-yellow?style=flat-square)](LICENSE)

---

## Overview

A production vending machine that sells perfume doses on the street. The customer presses a physical button to select a fragrance, a Payme QR code appears on the TFT display, they scan and pay, and the machine dispenses 2 spray doses via servo-controlled nozzles.

This is a complete commercial product — firmware, payment backend, remote price management, and hardware control.

**What it does:**

* 4 perfume slots with physical button selection and LVGL touchscreen UI
* Generates Payme QR codes directly on a 320×480 TFT display
* Processes payments in real time via MQTT (HiveMQ)
* Dispenses product via servo-controlled spray nozzles (2 doses per payment)
* Syncs prices from server every 30 seconds
* Caches prices in NVS for offline operation
* 5-minute payment timeout with on-screen countdown
* Error handling: WiFi/MQTT disconnect screens, order cancellation

---

## System Architecture

```
┌──────────────────────────────┐
│      Vending Machine         │
│                              │
│  ┌────────┐  ┌────────────┐  │         ┌──────────────┐
│  │ 4 Btns │  │ TFT 320x480│  │  HTTPS  │ Flask Server │
│  │ Select │  │ LVGL UI    │  │────────►│              │
│  └────────┘  │ QR Display │  │         │ /create-order│
│              └────────────┘  │◄──MQTT──│ /payme       │
│  ┌────────┐  ┌────────────┐  │         │ /api/prices  │
│  │ 4 Btns │  │ 4 Servos   │  │         └──────┬───────┘
│  │ Spray  │  │ Nozzles    │  │                │
│  └────────┘  └────────────┘  │         ┌──────┴───────┐
│                              │         │  Payme API   │
│  ESP32 + WiFi + MQTT         │         │  (Webhook)   │
└──────────────────────────────┘         └──────────────┘
```

---

## User Flow

1. **Customer** presses one of 4 buttons (Tom Ford / Lanvin / Dior / Dolce&Gabbana)
2. **Screen** highlights the selected fragrance, shows loading animation (3 sec)
3. **ESP32** sends `POST /create-order` to the server with product ID and price
4. **Server** generates Payme checkout URL, publishes via MQTT
5. **ESP32** renders QR code on TFT display
6. **Customer** scans QR with Payme app and pays
7. **Server** receives Payme webhook → publishes `"confirmed"` via MQTT
8. **ESP32** shows success screen, grants 2 spray doses
9. **Customer** presses the spray button to dispense (servo activates per press)
10. After 2 doses or 5-min timeout → machine resets to idle

---

## Hardware

| Component | Qty | Purpose |
|-----------|-----|---------|
| ESP32 DevKit | 1 | Main controller |
| TFT ILI9488 320×480 | 1 | LVGL UI + QR display |
| Servo SG90 | 4 | Spray nozzle control |
| Push buttons | 9 | 4 select + 4 spray + 1 cancel |
| Perfume tanks + nozzles | 4 | Product storage |

### Pin Map

```
Buttons (select):    GPIO 22, 23, 32, 33
Buttons (spray):     GPIO 19, 21, 26, 27
Button (cancel):     GPIO 25
Servo:               GPIO 18 (+ 3 more slots)
TFT:                 SPI (TFT_eSPI config)
```

---

## Software Stack

| Module | File | Description |
|--------|------|-------------|
| Main loop | `main.cpp` | Setup, LVGL tick, state machine, timeouts |
| Payment | `payment.cpp` | WiFi, MQTT, QR generation, order lifecycle, price sync |
| Buttons | `buttons_control.cpp` | Product selection, cancel, spray trigger |
| Servos | `Servo_controll.cpp` | Dose dispensing (servo rotate per button press) |
| Display | `tft_draw.cpp` | LVGL flush callback for TFT_eSPI |
| Server | `app.py` | Flask: Payme webhook, MQTT publisher, price API |

---

## LVGL Screens

| Screen | Purpose |
|--------|---------|
| Screen1 | Main menu — 4 products with prices |
| Screen2 | QR code display — waiting for payment |
| Screen3 | Error — no MQTT connection |
| Screen4 | Loading animation |
| Screen5 | Payment success |
| Screen6 | Payment cancelled |
| Screen7 | Timeout countdown (9 sec) |

---

## Project Structure

```
Street-Aroma-Vending/
├── firmware/
│   ├── main.cpp              # Entry point, state machine
│   ├── payment.cpp           # WiFi, MQTT, QR, orders, prices
│   ├── buttons_control.cpp   # Button handling, product selection
│   ├── Servo_controll.cpp    # Servo dispensing control
│   ├── tft_draw.cpp          # LVGL display driver
│   └── Globals.h             # Shared defines, pins, variables
│
├── server/
│   └── app.py                # Flask: Payme webhook + MQTT + prices API
│
├── ui/                        # LVGL UI assets (SquareLine Studio)
├── screenshots/
└── README.md
```

---

## Key Features

**QR Code Generation on Device** — Payme checkout URLs are rendered as QR codes directly on the ESP32 using RGB565 pixel buffer in PSRAM. No external QR service needed.

**Remote Price Management** — Prices are fetched from the server every 30 seconds and cached in ESP32 NVS. If the server is unreachable, cached prices are used.

**Dose Counting** — Each payment grants exactly 2 spray doses. A counter on screen shows remaining doses. The servo only activates when the spray button is pressed and doses remain.

**Payment Timeout** — If the customer doesn't pay within 5 minutes, a 9-second countdown appears. Any button press extends the timer. After countdown, the order is cancelled automatically.

**Error Recovery** — If MQTT disconnects, the machine shows an error screen and retries every 3 seconds. When connection is restored, it returns to the main menu.

---

## Server

The backend (`app.py`) handles:

- **Payme JSON-RPC webhook** — full transaction lifecycle (CheckPerform, Create, Perform, Cancel)
- **MQTT publishing** — sends `created`/`confirmed`/`cancelled` to the ESP32
- **Price management** — `GET/POST /api/prices` for remote updates
- **Order tracking** — stores all orders in `orders.json`

See [Payme-QR-Payment-Terminal](https://github.com/myseringan/Payme-QR-Payment-Terminal) for the standalone payment server documentation.

---

## Quick Start

### 1. Server

```bash
pip install flask paho-mqtt python-dotenv

# Configure .env
MERCHANT_ID=your_merchant_id
PAYME_KEY=your_secret_key
MQTT_BROKER=broker.hivemq.com

python app.py
```

### 2. Firmware

Update `Globals.h`:
```cpp
#define WIFI_SSID       "Your_WiFi"
#define WIFI_PASSWORD   "Your_Password"
#define MQTT_SERVER     "broker.hivemq.com"
#define MQTT_TOPIC      "payments/your_merchant_id"
#define SERVER_URL      "https://your-server.com/api"
#define DEVICE_ID       "street-aroma-01"
```

Flash with Arduino IDE (ESP32 board, PSRAM enabled).

### 3. Payme

Set webhook URL in Payme merchant dashboard:
```
https://your-server.com/payme
```

---

## Dependencies

| Library | Purpose |
|---------|---------|
| TFT_eSPI | TFT display driver |
| LVGL 8.3 | UI framework |
| PubSubClient | MQTT client |
| ArduinoJson | JSON parsing |
| ESP32Servo | Servo control |
| qrcode | QR code generation |
| WiFiClientSecure | HTTPS requests |
| Preferences | NVS price caching |

---

## Author

**Temur Eshmurodov** — [@myseringan](https://github.com/myseringan)

## License

MIT License — free to use and modify.
