# Temperature Monitoring and Alarm System (LM35 + Arduino + LabVIEW)

A PC-based data-acquisition and alarm system. An **LM35** sensor measures temperature, an **Arduino Uno** digitises it and streams it over USB, and a **LabVIEW** program converts, filters, displays, alarms on, and logs the data. LabVIEW also sends a command back to the Arduino to switch a physical buzzer and LED.

> **Author:** `<Your Name>` | **Tools:** LabVIEW 2026, NI-VISA, Arduino IDE | **Hardware:** Arduino Uno, LM35 (TO-92)

---

## Table of contents
1. [Features](#features)
2. [How it works](#how-it-works)
3. [Hardware](#hardware)
4. [Software requirements](#software-requirements)
5. [Repository structure](#repository-structure)
6. [Setup and usage](#setup-and-usage)
7. [LabVIEW program walkthrough](#labview-program-walkthrough)
8. [Key concepts explained](#key-concepts-explained)
9. [Relevance and real-world use cases](#relevance-and-real-world-use-cases)
10. [Testing and results](#testing-and-results)
11. [Troubleshooting](#troubleshooting)
12. [Known limitations](#known-limitations)
13. [Future improvements](#future-improvements)
14. [License](#license)

---

## Features

- **Real-time acquisition** of an analog LM35 signal through the Arduino's 10-bit ADC.
- **Unit conversion** from raw ADC counts to volts to degrees Celsius (10 mV/°C).
- **Moving-average filtering** with a user-adjustable window (`Averaging N`) to reduce noise.
- **Live display** on a thermometer indicator and a **two-trace trend chart** (raw vs averaged).
- **User-configurable high-limit alarm** with:
  - an on-screen LED,
  - an audible PC beep,
  - a physical buzzer and LED on the Arduino, driven by a command sent from LabVIEW.
- **Timestamped event log** (on screen and appended to a CSV file) recording `SYSTEM STARTED`, `ALARM ON` and `ALARM CLEARED`, with the temperature and limit at the time of each event, for fault diagnosis and root-cause analysis.
- **Clean shutdown**: buzzer switched off and serial port released when the program stops.

---

## How it works

```
                  analog voltage                 USB serial, 9600 baud
   LM35  ───────────────────────►  Arduino Uno  ─────────────────────────►  LabVIEW (PC)
 (10 mV/°C)                         A0, 10-bit ADC     "68\r\n" per line      read, convert, filter,
                                         ▲                                    compare, log
                                         │       '1' = alarm ON, '0' = OFF       │
                                         └───────────────────────────────────────┘
                                         drives buzzer (D8) and LED (D7)
```

1. The LM35 outputs a voltage proportional to temperature: **10 mV per °C**.
2. The Arduino reads that voltage on **A0** (0 to 5 V mapped to 0 to 1023) about 10 times per second and sends each raw count as a line of text.
3. LabVIEW reads each line, converts it to °C, and applies a moving average.
4. If the averaged temperature exceeds the **High Limit**, LabVIEW lights an alarm LED, beeps, and sends the character `1` to the Arduino, which turns on its buzzer and LED. When the temperature falls back, LabVIEW sends `0`.
5. Every alarm state change (and program start) is stamped with date and time, shown in the Event Log, and appended to a CSV file.

---

## Hardware

### Bill of materials

| Qty | Part | Notes |
|---|---|---|
| 1 | Arduino Uno (or compatible) + USB cable | Clones with a CH340 chip need the CH340 driver |
| 1 | LM35 temperature sensor, **TO-92** package | Output 10 mV/°C |
| 1 | Active buzzer, 5 V | "Active" = has its own oscillator and sounds on plain DC |
| 1 | LED (any colour) | |
| 1 | 220 Ω resistor | LED current limiting |
| 1 | Breadboard + jumper wires | |

### Wiring

**LM35 (TO-92): hold with the flat face toward you, pins pointing down. Left to right:**

| LM35 pin | Connects to |
|---|---|
| +Vs (left) | Arduino **5V** |
| Vout (middle) | Arduino **A0** |
| GND (right) | Arduino **GND** |

| Component | Connection |
|---|---|
| Buzzer long leg (+) | Arduino **D8** |
| Buzzer short leg (−) | GND |
| LED anode (long leg) | Arduino **D7** through the **220 Ω** resistor |
| LED cathode (short leg) | GND |

```
            Arduino Uno
         ┌───────────────┐
   5V ───┤ 5V        D8  ├─── Buzzer (+)    Buzzer (−) ─── GND
  GND ───┤ GND       D7  ├─── 220Ω ─── LED(+)   LED(−) ─── GND
 LM35 ───┤ A0            │
 Vout    └───────────────┘
 LM35 +Vs ── 5V      LM35 GND ── GND
```

> ⚠️ **Polarity warning.** Reversing the LM35's supply pins heats the chip within seconds and can permanently damage it. Always confirm the flat face and the red/blue rail assignment **before** connecting USB. If the sensor ever feels warm on its own, unplug immediately.

---

## Software requirements

| Software | Purpose |
|---|---|
| **LabVIEW** (developed in LabVIEW 2026) | The monitoring program (`.vi`) |
| **NI-VISA** | Driver that lets LabVIEW talk to serial ports |
| **Arduino IDE** | To upload the firmware |
| *(Windows only, clones)* CH340 USB driver | Needed if no COM port appears |

> The VI is saved in LabVIEW 2026. Older LabVIEW versions cannot open it directly. Use **File → Save for Previous Version** if you need to share it with an older install.

---

## Repository structure

```
temperature-monitor/
├── README.md                  ← this file
├── REPORT.md                  ← detailed technical report
├── LICENSE
├── .gitignore
├── arduino/
│   └── temp_monitor/
│       └── temp_monitor.ino   ← firmware
├── labview/
│   └── temperature control.vi ← LabVIEW program
├── docs/
│   ├── block_diagram.png      ← screenshot of the full block diagram
│   ├── front_panel.png        ← screenshot of the front panel running
│   ├── wiring_diagram.png     ← breadboard photo or Fritzing diagram
│   ├── circuit_photo.jpg      ← photo of the real build
│   └── trend_chart.png        ← screenshot showing raw vs averaged trace
└── data/
    └── sample_alarm_log.csv   ← example log produced by a real run
```

---

## Setup and usage

### 1. Build the circuit
Wire the parts as shown above with the USB **unplugged**. Double-check the LM35 orientation.

### 2. Upload the firmware
1. Open `arduino/temp_monitor/temp_monitor.ino` in the Arduino IDE.
2. Select **Tools → Board → Arduino Uno** and the correct **Port**.
3. Click **Upload**.
4. Open the **Serial Monitor** at **9600 baud**. At room temperature you should see numbers around **45 to 70** (roughly 22 to 34 °C). Pinching the sensor should raise them.
5. **Close the Serial Monitor.** It holds the COM port and blocks LabVIEW.

### 3. Run the LabVIEW program
1. Open `temperature control.vi` and go to the Front Panel.
2. Choose the Arduino's port in **COM Port** (for example `ASRL4::INSTR` for COM4).
3. Set **High Limit** (for example 30 °C), **Averaging N** (for example 10), and **Log File Path** (for example `C:\Users\<you>\Documents\TempMonitor\alarm_log.csv`; the folder must already exist).
4. Click **Run** (single arrow). Wait about 2 seconds for the Arduino to reboot.
5. Pinch the sensor until the temperature passes the limit: the alarm LED lights, the PC beeps, and the buzzer and LED on the breadboard switch on.
6. Release the sensor. The alarm clears when the average falls below the limit.
7. Press **stop**. The buzzer is switched off and the port is closed.

### Arduino firmware

```cpp
const int SENSOR_PIN = A0;
const int BUZZER_PIN = 8;
const int LED_PIN    = 7;

void setup() {
  Serial.begin(9600);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  Serial.println(analogRead(SENSOR_PIN));   // e.g. "48" + CR/LF
  while (Serial.available() > 0) {
    char c = Serial.read();
    if (c == '1') { digitalWrite(BUZZER_PIN, HIGH); digitalWrite(LED_PIN, HIGH); }
    if (c == '0') { digitalWrite(BUZZER_PIN, LOW);  digitalWrite(LED_PIN, LOW);  }
  }
  delay(100);   // about 10 samples per second
}
```

---

## LabVIEW program walkthrough

### Front Panel

| Element | Type | Purpose |
|---|---|---|
| COM Port | VISA resource name | Selects the serial port |
| High Limit | Numeric control (DBL) | Alarm threshold in °C |
| Averaging N | Numeric control (I32) | Moving-average window length |
| Log File Path | Path control | Where the CSV is written |
| stop | Boolean button | Ends the loop |
| Raw ADC | Numeric indicator | Raw 0 to 1023 count (useful for debugging) |
| Temperature °C | Thermometer | Averaged temperature |
| Trend | Waveform chart | Raw and averaged traces |
| Alarm | Round LED | Alarm state |
| Event Log | String indicator | Scrolling event history |

### Block Diagram, in execution order

| Stage | What it does | LabVIEW blocks |
|---|---|---|
| **1. Open port** | Opens the serial session at 9600 baud, then waits 2 s for the Arduino to finish rebooting | VISA Configure Serial Port, Wait (ms), inside a Flat Sequence Structure |
| **2. Main loop** | Repeats until stop is pressed | While Loop, iteration counter `i` |
| **3. Read and parse** | Reads one text line, converts to a number | VISA Read (20 bytes), Scan From String (`%f`) |
| **4. Convert** | counts → volts → °C: `raw × 5 ÷ 1023 × 100` | Multiply, Divide |
| **5. Average** | Keeps the last N samples and takes their mean | Shift register, Build Array (concatenate), Array Size, Subtract, Max & Min, Array Subset, Mean |
| **6. Display** | Shows the average and plots raw + average | Thermometer, Bundle, Waveform Chart |
| **7. Alarm** | Compares average with limit; lights LED; beeps; sends `1`/`0` to Arduino | Greater Than?, Select, VISA Write, Case Structure + Beep |
| **8. Detect events** | Flags a change of alarm state or the first pass | Boolean shift register (previous alarm), Not Equal?, Equal to 0?, Or |
| **9. Log** | Builds a timestamped CSV line, prepends it to the on-screen log, appends it to the file | Get Date/Time String, Select, Format Into String, Concatenate Strings, Open/Create/Replace File, Set File Position, Write to Text File, Close File |
| **10. Shutdown** | Sends `0`, closes the port, reports errors | VISA Write, VISA Close, Simple Error Handler |

See [`REPORT.md`](REPORT.md) for a block-by-block technical explanation.

### Log file format

```
10/26/2026,14:03:22,SYSTEM STARTED,23.4,30.0
10/26/2026,14:04:10,ALARM ON,30.2,30.0
10/26/2026,14:04:41,ALARM CLEARED,29.8,30.0
```

Columns: `date, time, event, averaged temperature (°C), high limit (°C)`.

---

## Key concepts explained

**Analog sensing (LM35).** The LM35 is a precision, linear, analog temperature sensor. Its output voltage is 10 mV per °C (0.25 V at 25 °C), so the conversion needs no lookup table and no calibration curve. It measures its **own die temperature**, so what you read is the sensor's body temperature, not the air's.

**Analog-to-digital conversion (ADC).** The Arduino Uno has a 10-bit ADC: it divides the 0 to 5 V range into 1024 steps. One step is 5 V ÷ 1023 ≈ 4.89 mV, which is about **0.49 °C per count**. That is the resolution limit of this design. Using the 1.1 V internal reference would give about 0.1 °C per count (up to about 110 °C).

**Serial communication.** The Arduino sends each reading as ASCII text ending in a carriage return and newline at 9600 baud, 8 data bits, no parity, 1 stop bit (8N1). The newline acts as a **message delimiter**, so LabVIEW's read returns exactly one reading at a time.

**VISA.** *Virtual Instrument Software Architecture* is a standard API for talking to instruments over serial, USB, GPIB, Ethernet and more. The program opens a *session* on a *resource name* (such as `ASRL4::INSTR`, where ASRL means asynchronous serial and 4 is COM4), then passes that session reference from block to block until it is closed.

**Dataflow programming.** LabVIEW blocks run when all their inputs have data, not line by line. Wires carry both the data and the execution order. Where two blocks share no data (like Configure and Wait), a **Flat Sequence Structure** or an error wire forces the order.

**Error clusters.** Each VISA and file block receives and outputs an error cluster (status, code, source). If an upstream block fails, downstream blocks skip their work, and the error surfaces at the end. This chaining also guarantees ordering.

**Shift registers.** A shift register is a loop's memory. It carries a value from one iteration to the next. This project uses three: the sample history (for averaging), the previous alarm state (for change detection), and the log text.

**Moving-average filter.** Averaging the last N samples reduces random noise by roughly **√N** but introduces a lag of about **(N−1)/2 samples** (about 0.45 s for N = 10 at 10 Hz). A larger N gives a steadier reading and a slower response. This is the basic noise-versus-responsiveness trade-off in signal conditioning.

**Event-driven logging.** Logging every sample would produce thousands of rows with little information. Comparing the current alarm state with the previous one produces a row only at transitions, so the log is short, readable and useful for tracing *when* a fault began and ended.

**Why the Arduino only measures and obeys.** All logic (conversion, filtering, thresholds, logging) lives in LabVIEW. The limit and filter length can be changed at runtime without reflashing firmware, and the Arduino code stays minimal and reliable.

**Why wait 2 seconds at start-up.** Opening the serial port toggles the Uno's DTR line, which resets the board. The bootloader then takes about 1 to 2 seconds. Without the wait, the first reads would catch partial or no data.

---

## Relevance and real-world use cases

Temperature monitoring with threshold alarms and event logs is one of the most common tasks in instrumentation and control. This project is a small-scale version of the same architecture used in industry: **sensor → signal conditioning → acquisition → processing → HMI/alarm → data historian**.

### Why it matters
- **Safety:** overheating is a leading cause of equipment failure and fire. Early alarms prevent damage.
- **Quality and compliance:** many processes (food, pharma, labs) must prove that temperatures stayed within limits. Timestamped logs provide that evidence.
- **Predictive maintenance:** trends and event history show *when* a system started drifting, enabling root-cause analysis.
- **Skills transfer:** the same patterns (VISA communication, filtering, state-change detection, file logging, shutdown handling) apply to any sensor, from pressure to vibration.

### Example applications
| Domain | Use |
|---|---|
| **Server rooms and data centres** | Alarm when ambient or rack temperature exceeds a limit |
| **Cold chain** (vaccines, food, medicines) | Log fridge or transport temperatures with timestamps for audit |
| **Laboratories** | Monitor incubators, ovens and reaction vessels |
| **Industrial machinery** | Watch motor, bearing or transformer temperature for early fault detection |
| **Electronics test benches** | Monitor heat-sink or component temperature during burn-in |
| **Greenhouses and agriculture** | Track and alarm on growing-area temperature |
| **HVAC and building management** | Zone temperature supervision and alarm history |
| **3D printing / battery packs** | Detect overheating of heaters or cells |
| **Education** | Teaching DAQ, signal conditioning, HMI design and troubleshooting |

### Concepts this project demonstrates
Sensor interfacing, ADC resolution, unit scaling, noise filtering, alarm design, HMI design, serial protocol design, state-change detection, file I/O, error handling, and hardware debugging.

---

## Testing and results

> Fill this section with your own measurements. Suggested table:

| Test | Expected | Observed |
|---|---|---|
| Room temperature raw count | ~45 to 70 | `<your value>` |
| Converted temperature vs reference thermometer | within about ±1 °C | `<your value>` |
| Pinch test: rise above limit | alarm triggers | `<result>` |
| Release: alarm clears | `ALARM CLEARED` logged | `<result>` |
| Averaging N = 1 vs 10 vs 30 | smoother and slower as N grows | `<screenshot>` |
| Stop button | buzzer off, port released | `<result>` |
| Re-run after stop | no "port busy" error | `<result>` |

Add `docs/trend_chart.png` showing the jittery raw trace against the smooth averaged trace.

---

## Troubleshooting

| Symptom | Likely cause and fix |
|---|---|
| COM Port drop-down empty | Missing CH340 driver, charge-only USB cable, or port already open elsewhere |
| Timeout error after about 10 s | Wrong port, Serial Monitor still open, or the sketch isn't running |
| Error 1 at Scan From String | No or partial data. Check baud (9600 on both sides). One error at start-up can be ignored |
| Readings wander or drift | A0 not connected to the sensor (floating pin) |
| Reading ~1023, sensor hot | LM35 wired backwards. **Unplug immediately**, re-check the flat face |
| Reading stuck high and unresponsive | Sensor damaged by reverse power. Replace it |
| LED works, buzzer silent | Check that the buzzer is *active*, its polarity, and the D8/GND connections. A passive buzzer needs `tone()` in the firmware |
| No CSV file | Folder in `Log File Path` doesn't exist, or the path has a typo |
| Alarm never triggers | Limit too high or N too large |

---

## Known limitations

- **Not closed-loop control in the strict sense.** The system monitors, alarms and signals back to a physical indicator, but it does not regulate the temperature (no heater or fan control).
- **No hysteresis.** If the temperature hovers at the limit, the alarm can toggle repeatedly.
- **Start-up alarm edge case.** If the temperature is already above the limit on the first pass, the first log line reads `SYSTEM STARTED`, not `ALARM ON`.
- **Resolution** is about 0.49 °C per count with the default 5 V reference.
- **Accuracy** depends on the LM35's tolerance and the stability of the USB 5 V supply.
- **Single-supply LM35 wiring** limits the range to roughly 2 to 100 °C or more depending on the variant, which suits this application.
- **No CSV header row.**
- The loop ends on the stop button only. A cable fault ends it after the VISA read timeout (10 s).

---

## Future improvements

- Add **hysteresis** (clear the alarm only below limit − 1 °C).
- Add a **low-limit alarm** and a **rate-of-change** alarm.
- Add **sensor-fault detection** (readings stuck at 0 or 1023) and log `SENSOR FAULT`.
- Use `analogReference(INTERNAL)` for about 0.1 °C resolution.
- Add a **CSV header row** and log periodic temperature samples alongside events.
- Add a **mute/acknowledge** button.
- Add **heater/fan control** (relay or MOSFET) with a PID loop to make it a true closed-loop controller.
- Add **Stop-on-error** by combining the VISA error status with the stop button.
- Move to a wireless link (Bluetooth, Wi-Fi) or publish data via MQTT to a dashboard.

---

## License

Released under the MIT License. See `LICENSE`.
