# V2X Accident Detection and Alert System

An STM32-based vehicle safety prototype with two nodes. The **transmitter node** detects a collision using an accelerometer/gyroscope and an ultrasonic sensor, reads its GPS location, sends SMS alerts through a SIM900A GSM module, and broadcasts a warning over an NRF24L01 radio link. The **receiver node** in a nearby vehicle picks up that warning and sounds a buzzer.

![Hardware prototype](docs/photos/system.png)
*Right board: transmitter node (SIM900A, NRF24L01, STM32, ultrasonic sensor, GY-87, GPS). Left board: receiver node (NRF24L01, STM32, buzzer).*

---

## Project context and credits

This repository documents the hardware and firmware of the project **"V2X Communication After Accident Detection"**

| Team member | Role |
|---|---|
| Nikhil Vinayak Kamble | Project Author |
| Shivraj Sanjay Thorat | Project Author |
| Sumit Kamble | Project Author |

## How it works

1. The transmitter continuously decodes GPS data (TinyGPS++) and keeps the last valid latitude and longitude.
2. It reads acceleration and rotation from the MPU6050 and the distance from the ultrasonic sensor.
3. If any threshold is crossed (see table below), it declares an accident.
4. It broadcasts `Accident Detected` over NRF24L01.
5. It then repeats 10 times: an SMS with a Google Maps link goes to phone 1 and an `ALERT` SMS goes to phone 2, with another radio broadcast in between.
6. It broadcasts the radio warning 30 more times, once per second.
7. The receiver node receives the message and sounds the buzzer four times (2 s on, 1 s off).

## System architecture

![Block diagram](docs/block_diagram.png)

## Detection logic

An accident is flagged when **any** of these is true:

| Sensor | Condition | Threshold |
|---|---|---|
| MPU6050 accelerometer | Total acceleration magnitude, sqrt(ax² + ay² + az²) | above 15 m/s² (about 1.5 g) |
| Ultrasonic sensor | Distance to obstacle | below 7 cm |
| MPU6050 gyroscope | Rotation rate on X, Y or Z axis | above 30 °/s |

Distance is computed from the echo pulse as `duration × 0.034 / 2` (cm).

## Hardware

**Transmitter node**

| Component | Interface | Purpose |
|---|---|---|
| STM32F411CEU6 (Black Pill) | - | Main controller |
| GY-87 module (MPU6050 used) | I2C | Impact and rotation sensing |
| Ultrasonic sensor (HC-SR04 type) | GPIO | Obstacle distance |
| NEO-M8N GNSS module with antenna | UART | Location |
| SIM900A GSM module + SIM card + antenna | UART | SMS alerts |
| NRF24L01 with external antenna | SPI | V2V broadcast |
| Perf board, supply capacitor on NRF24L01, wires | - | Assembly |

**Receiver node**

| Component | Interface | Purpose |
|---|---|---|
| STM32F411CEU6 (Black Pill) | - | Main controller |
| NRF24L01 with external antenna | SPI | Receives the warning |
| Buzzer | GPIO | Audible alert |
| 16x2 I2C LCD (address 0x27) | I2C | Initialised in code, but the display lines are commented out, so it is optional |

**Power:** [TODO: how each board and the SIM900A were powered. The SIM900A needs a supply that can deliver about 2 A peaks.]

## Pin connections

**Transmitter**

| Device | Signal | STM32 pin |
|---|---|---|
| MPU6050 (address 0x68) | SDA / SCL | PB9 / PB8 |
| Ultrasonic | TRIG / ECHO | PA0 / PA1 |
| NEO-M8N GPS (9600 baud) | GPS TX → STM32 RX / GPS RX ← STM32 TX | PA10 / PA9 (USART1) |
| SIM900A (9600 baud) | SIM900A TX → STM32 RX / SIM900A RX ← STM32 TX | PA3 / PA2 (USART2) |
| NRF24L01 | CE / CSN | PB0 / PB1 |
| NRF24L01 | SCK / MISO / MOSI | default SPI1 pins of the Arduino core |

**Receiver**

| Device | Signal | STM32 pin |
|---|---|---|
| NRF24L01 | CE / CSN | PB0 / PB1 |
| NRF24L01 | SCK / MISO / MOSI | default SPI1 pins of the Arduino core |
| Buzzer | Control | PC14 |
| Debug serial (9600 baud) | TX / RX | PA2 / PA3 (`Serial2`) |
| On-board LED | - | PC13 (initialised, active low) |

## Radio link settings

Both nodes use the same settings: pipe address `00001`, channel 108, 1 Mbps, low power level, auto-acknowledge disabled. The payload is the text `Accident Detected`.

## Cloud alerting (Flask + Twilio)

The project report describes a path where the SIM900A sends the accident data over GPRS to a Flask server, which then triggers SMS or calls through the Twilio API. **The firmware in this repo does not include that path.** It sends SMS directly through the SIM900A with AT commands (`AT+CMGF=1`, `AT+CMGS`).

[TODO: either add your Flask/Twilio server code under `cloud/` with credentials removed, or state here what the second `ALERT` SMS triggers, for example an automatic call.]

## Repository structure

```
v2x-accident-alert/
├── README.md
├── .gitignore
├── firmware/
│   ├── transmitter/
│   │   ├── transmitter.ino
│   │   └── config.example.h
│   └── receiver/
│       └── receiver.ino
└── docs/
    ├── block_diagram.png
    └── photos/
        └── system.png
```

## Configuration

Phone numbers are not stored in the repo. Before building the transmitter:

1. Copy `firmware/transmitter/config.example.h` to `firmware/transmitter/config.h`.
2. Put your own two numbers in international format (for example `+911234567890`).

`config.h` is listed in `.gitignore`, so it is never committed.

## Build and flash

1. Install the Arduino IDE and add STM32 board support (STM32duino, "STM32 MCU based boards" in Boards Manager).
2. Select board: Generic STM32F4 series, then BlackPill F411CE.
3. Install these libraries from Library Manager:
   - Adafruit MPU6050 (with Adafruit Unified Sensor and BusIO)
   - RF24 by TMRh20
   - TinyGPSPlus by Mikal Hart
   - LiquidCrystal I2C (receiver only)
4. Flash `firmware/transmitter/transmitter.ino` to the transmitter board and `firmware/receiver/receiver.ino` to the receiver board. Upload method: [TODO: ST-Link or USB DFU].
5. Insert a working 2G-capable SIM into the SIM900A. Power both nodes and wait for the GPS to get a fix. The serial monitor shows `Waiting for GPS fix...` until then.

## Testing

[TODO: describe what you actually tested, for example impact detection by shaking the board, GPS fix time outdoors, SMS delivery, and NRF24L01 range in metres.]

## Known limitations

- Thresholds are fixed values and need calibration for a real vehicle.
- The gyroscope check uses `> 30` without an absolute value, so rotation in the negative direction is not detected.
- The ultrasonic read has no timeout. A missed echo or disconnected sensor reads 0 cm, which is below the 7 cm threshold and raises a false alert.
- One alert sequence uses blocking delays and takes about 2.5 minutes, during which the sensors are not polled.
- Each incident sends 10 SMS to each number, which uses a lot of SMS credit.
- If the GPS never gets a fix, the SMS contains the coordinates 0.000000,0.000000.
- With auto-acknowledge disabled there is no delivery confirmation on the radio link, and the low power level limits the range.
- The receiver alerts only once and ignores further messages until it is reset.
- The SIM900A is a 2G module, and 2G networks are being phased out in many regions.

## Future work

- Send one alert SMS (with retry on failure) instead of repeating it ten times.
- Replace blocking delays with a non-blocking state machine.
- Add a timeout and plausibility check to the distance measurement.
- Use acknowledged radio packets and a longer-range, higher-power radio setting.
- Upgrade to a 4G module and add the cloud dashboard path.
- Enable the LCD display on the receiver.

## Contact

**Sumit Kamble**, Electronics Engineering, Walchand College of Engineering, Sangli
[LinkedIn](https://www.linkedin.com/in/sumit-kamble-24b489292) | sumitkamble1844@gmail.com
