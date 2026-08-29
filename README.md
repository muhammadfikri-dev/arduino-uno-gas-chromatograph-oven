# ⚡ Arduino Uno Gas Chromatography Capillary Column Temperature Ramp

[![Lisensi: MIT](https://img.shields.io/badge/Lisensi-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Platform: Arduino Uno](https://img.shields.io/badge/Platform-Arduino%20Uno%20|%20ATmega328P-blue.svg)](#)
[![Framework: Arduino IDE](https://img.shields.io/badge/Framework-Arduino%20IDE%202.0%2B-teal.svg)](https://www.arduino.cc/)
[![Status: Firmware Produksi](https://img.shields.io/badge/Status-Firmware%20Produksi-brightgreen.svg)](#)
[![Developer: Muhammad Fikri](https://img.shields.io/badge/Developer-Muhammad%20Fikri-blue.svg)](#)

Analytical chemistry instrument regulating precise thermal ramp profiles (5°C/min to 350°C) with K-type thermocouple feedback and PID SSR control.

---

## 📊 Diagram Blok Arsitektur & Skema Rangkaian

Berikut adalah visualisasi alur daya, interaksi sensor, logika pemrosesan internal, dan aktuasi perlindungan perangkat:

```mermaid
graph TD
    subgraph Power_Supply ["⚡ Sumber Daya Listrik (Power Supply)"]
        PSU["Adapter / Catu Daya 5V-12V"] --> REG["Voltage Regulator / Step-Down 5V & 3.3V"]
        REG --> MCU["🧠 Arduino Uno (ATmega328P)"]
    end

    subgraph Inputs_Sensors ["📥 Input & Sensor Presisi"]
        SENS["Sensor Analog / Digital Front-End"] -->|"Sinyal / Bus Data"| MCU
        ESTOP["Emergency Stop Button (INT)"] -->|"Interupsi Kritis"| MCU
        ENC["Magnetic Encoder / User Input"] -->|"I2C / SPI / Pulse"| MCU
    end

    subgraph Controller_Core ["⚙️ Pemrosesan & Logika Sistem"]
        MCU -->|"DSP / Kalman Filter"| DSP["Filtering & Kalibrasi"]
        MCU -->|"Non-Volatile"| NVS["EEPROM Storage"]
        MCU -->|"State Machine"| FSM["Failsafe & Control Loop"]
    end

    subgraph Outputs_Actuators ["📤 Output, Aktuator & Proteksi"]
        MCU -->|"PWM / Digital Out"| RELAY["Modul Relay / Power MOSFET (Beban Kritis)"]
        MCU -->|"High-Speed I2C"| DISP["Layar OLED / LCD Display"]
        MCU -->|"Alarm Trigger"| BUZZ["Acoustic Buzzer & Status LED"]
    end

    subgraph Communication_Telemetry ["📡 Jaringan & Telemetri"]
        MCU -->|"Serial UART / RS485 Modbus"| TELEM["Telemetry Stream / Cloud Dashboard"]
    end

    style MCU fill:#1e88e5,stroke:#0d47a1,stroke-width:2px,color:#ffffff
    style PSU fill:#f4511e,stroke:#bf360c,stroke-width:2px,color:#ffffff
    style RELAY fill:#43a047,stroke:#1b5e20,stroke-width:2px,color:#ffffff
    style SENS fill:#8e24aa,stroke:#4a148c,stroke-width:2px,color:#ffffff
    style DISP fill:#00acc1,stroke:#006064,stroke-width:2px,color:#ffffff
```

---

## 📦 Daftar Komponen & Bahan Lengkap (Bill of Materials - BOM)

Seluruh bahan yang diperlukan untuk merakit dan mengoperasikan sistem ini secara penuh:

| No | Nama Komponen / Modul | Estimasi Jumlah | Fungsi & Spesifikasi Teknis |
|:---|:---|:---|:---|
| 1 | **Arduino Uno R3 (ATmega328P DIP/SMD)** | 1 Unit | Unit pemroses utama (Microcontroller Unit) |
| 2 | **Layar LCD 16x2 / 20x4 dengan I2C Backpack (PCF8574)** | 1 Unit | Tampilan alfanumerik metrik dan alarm secara real-time |
| 3 | **Modul Relay 1-Channel / 4-Channel dengan Optocoupler (5V/10A)** | 1-2 Unit | Isolasi optik pengendali beban tegangan tinggi / kontaktor |
| 4 | **Active Buzzer 5V & LED Indikator 5mm (Merah, Hijau, Biru)** | 1 Set | Indikator status operasional dan peringatan audio (*audible alarm*) |
| 5 | **Push Button Emergency Stop (E-Stop) / Tactile Switch** | 2-4 Unit | Tombol darurat dan navigasi menu kalibrasi |
| 6 | **Resistor Carbon Film (220Ω, 1kΩ, 4.7kΩ, 10kΩ 1/4W)** | 1 Set | Resistor pull-up/pull-down dan pembatas arus LED |
| 7 | **Kapasitor Keramik (100nF) & Elektrolit (100uF 25V)** | 1 Set | Peredam noise catu daya (*decoupling filter*) |
| 8 | **Breadboard MB-102 & Kabel Jumper Dupont (Male-Male, Male-Female)** | 1 Set | Kabel penghubung prototipe tanpa solder |
| 9 | **7-12V DC via DC Barrel Jack / 5V USB (Disarankan Adaptor 9V 1A)** | 1 Unit | Sumber daya listrik stabil untuk seluruh rangkaian |

---

## 🧠 Arsitektur Sistem & Fitur Utama

- **Deterministic Non-Blocking State Machine:** Memastikan kontrol loop real-time berkecepatan tinggi tanpa *jitter*.
- **Digital Signal Processing (DSP) & Filtering:** Dilengkapi algoritma Kalman filtering dan *oversampling* untuk eliminasi *noise* sinyal analog.
- **Non-Volatile Storage (EEPROM):** Parameter kalibrasi, *setpoint*, dan konfigurasi tersimpan secara persisten terhadap pemadaman daya.
- **Hardware Failsafe & Emergency Interlock:** Perlindungan otomatis jika terjadi anomali tegangan, arus berlebih, atau pemicuan *Emergency Stop*.
- **Industrial Telemetry & Diagnostics:** Pelaporan status operasional secara real-time via Serial/JSON stream.

---

## 🔌 Skema Pinout & Koneksi Hardware

| Komponen / Sinyal | Pin (Arduino Uno) | Deskripsi Fungsi |
|:---|:---|:---|
| **Sensor Analog Input** | `Pin A0` | Jalur pembacaan sensor utama berpresisi tinggi |
| **Emergency Stop (E-Stop)** | `Pin 2 (INT0)` | Pemicu pengaman darurat hardware interrupt |
| **Actuator / Relay Utama** | `Pin 9 (PWM) / Pin 7` | Pengendali beban daya tinggi / relay aktuator |
| **Acoustic Alarm Buzzer** | `Pin 8` | Indikator peringatan audible saat terjadi anomali |
| **Status / Heartbeat LED** | `Pin 13` | Indikator status aktivitas sistem real-time |

---

## 🛠️ Panduan Perakitan Hardware (Langkah Demi Langkah)

1. **Persiapan Catu Daya:** Hubungkan jalur 5V dan GND dari catu daya utama ke *power rail* breadboard. Pasang kapasitor decoupling 100nF dan 100uF secara paralel di dekat pin daya mikrokontroler.
2. **Pemasangan Sensor:** Hubungkan pin sinyal output sensor ke pin analog mikrokontroler (`Pin A0`). Pasang resistor pull-up 4.7kΩ jika menggunakan sensor bertipe I2C.
3. **Pemasangan Aktuator & Relay:** Hubungkan pin kontrol relay ke pin output (`Pin 9/7`). Pastikan dioda flyback (1N4007) terpasang paralel pada koil beban induktif untuk meredam lonjakan tegangan balik (*back-EMF*).
4. **Pemasangan Tombol Emergency Stop:** Hubungkan tombol ke pin interupsi (`Pin 2`) dengan konfigurasi *Active-LOW* menggunakan internal pull-up.
5. **Pemeriksaan Akhir:** Ukur tegangan semua jalur menggunakan multimeter sebelum menghubungkan sumber daya untuk mencegah korsleting listrik.

---

## 🚀 Panduan Kompilasi & Upload (Arduino IDE)

1. Buka **Arduino IDE 2.0+**.
2. Masuk ke menu **Tools > Board**:
   * Pilih **`Arduino Uno`**.
3. Pastikan dependensi pustaka terpasang via Library Manager:
   * `ArduinoJson` (v6 / v7)
   * `Wire` & `SPI`
   * `EEPROM`
4. Buka berkas [`arduino-uno-gas-chromatograph-oven.ino`](./arduino-uno-gas-chromatograph-oven.ino).
5. Klik tombol **Verify** (✓) kemudian **Upload** (➔).
6. Buka **Serial Monitor** pada baudrate **`115200`** untuk melihat streaming telemetri dan status operasional.

---

## 📄 Lisensi
Didistribusikan di bawah lisensi open-source **MIT License**. Dikembangkan oleh **Muhammad Fikri**.
