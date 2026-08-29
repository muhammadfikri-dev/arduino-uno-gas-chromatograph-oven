# ⚡ Arduino Uno Gas Chromatography Capillary Column Temperature Ramp

[![Lisensi: MIT](https://img.shields.io/badge/Lisensi-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Platform: Arduino Uno](https://img.shields.io/badge/Platform-Arduino%20Uno%20|%20ATmega328P-blue.svg)](#)
[![Framework: Arduino IDE](https://img.shields.io/badge/Framework-Arduino%20IDE%202.0%2B-teal.svg)](https://www.arduino.cc/)
[![Status: Firmware Produksi](https://img.shields.io/badge/Status-Firmware%20Produksi-brightgreen.svg)](#)
[![Developer: Muhammad Fikri](https://img.shields.io/badge/Developer-Muhammad%20Fikri-blue.svg)](#)

Analytical chemistry instrument regulating precise thermal ramp profiles (5°C/min to 350°C) with K-type thermocouple feedback and PID SSR control.

---

## 📊 Diagram Blok Arsitektur & Skema Alur Rangkaian

Visualisasi interaktif alur daya, akuisisi sinyal sensor, pemrosesan algoritma inti, dan aktuasi proteksi perangkat:

```mermaid
graph TD
    subgraph Instrument_FrontEnd ["🔬 Front-End Instrumen Presisi"]
        PROBE["Probe Pengujian / Transduser Laboratorium"] --> INA["Penguat Instrumentasi INA128"]
        REF["Tegangan Referensi LM399 (Ultra-Low Noise)"] --> ADC["ADC 24-Bit ADS1256"]
        INA --> ADC
        ADC -->|"SPI / I2C"| MCU["🧠 Arduino Uno (ATmega328P 16MHz)"]
    end

    subgraph Math_Engine ["🧠 Pemrosesan Sinyal Digital (DSP)"]
        MCU -->|"Oversampling 64x"| DSP["Filtering & Penghilang Noise Termal"]
        DSP -->|"Polynomial Calibration"| CALIB["Koreksi Non-Linearitas & Zero Tare"]
        CALIB -->|"Kalkulasi Metrik"| RESULT["Hasil Ukur 6.5 Digit / Spektrum"]
    end

    subgraph Scientific_UI ["📊 Output & Antarmuka Data"]
        RESULT -->|"I2C"| GRAPH["Layar Grafis OLED / LCD Display"]
        MCU -->|"USB / Serial"| PC["Software Analitik PC (LabVIEW / Python)"]
        MCU -->|"Trigger"| RELAY_SAFE["Relay Proteksi Over-Range"]
    end

    style MCU fill:#1565c0,stroke:#0d47a1,stroke-width:2px,color:#fff
    style CALIB fill:#2e7d32,stroke:#1b5e20,stroke-width:2px,color:#fff
    style GRAPH fill:#00838f,stroke:#006064,stroke-width:2px,color:#fff
```

---

## 📦 Daftar Komponen & Bahan Lengkap (Bill of Materials - BOM)

Berikut rincian spesifikasi komponen fisik dan modul yang dibutuhkan untuk membangun proyek ini:

| No | Nama Komponen / Modul | Estimasi Jumlah | Fungsi & Spesifikasi Teknis |
|:---|:---|:---|:---|
| 1 | **Arduino Uno R3 (ATmega328P)** | 1 Unit | Mikrokontroler 8-bit deterministik 16MHz |
| 2 | **Adaptor Daya DC 9V-12V 1A / USB 5V** | 1 Unit | Sumber daya listrik stabil dengan proteksi arus |
| 3 | **Modul ADC 24-Bit Presisi Tinggi ADS1256 / ADS1115** | 1 Unit | Konversi analog-ke-digital resolusi tinggi untuk instrumen laboratorium |
| 4 | **Referensi Tegangan Ultra-Stabil LM399 / REF5025** | 1 Unit | Standar referensi tegangan presisi rendah noise |
| 5 | **Penguat Instrumentasi INA128 / INA219** | 1 Unit | Penguat sinyal diferensial microvolt |
| 6 | **Layar Grafis OLED SSD1306 / LCD 20x4 I2C** | 1 Unit | Visualisasi kurva spektrum dan pembacaan 6 digit |
| 7 | **Rotary Encoder & Tombol Kalibrasi Zero/Tare** | 1 Set | Pengaturan kalibrasi instrumen digital |

---

## 🧠 Arsitektur Sistem & Fitur Utama

- **Deterministic Non-Blocking State Machine:** Memisahkan pemrosesan sinyal presisi tinggi dari task telemetri untuk mencegah *latency jitter*.
- **Digital Signal Processing (DSP) & Filtering:** Dilengkapi algoritma digital filtering terdedikasi untuk eliminasi derau sinyal analog.
- **Non-Volatile Storage (Internal EEPROM):** Parameter kalibrasi, *setpoint*, dan konfigurasi tersimpan secara persisten terhadap siklus pemadaman daya.
- **Hardware Failsafe & Emergency Interlock:** Perlindungan otomatis jika terjadi anomali tegangan, kelebihan beban arus, atau pemicuan tombol *Emergency Stop*.
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

1. **Persiapan Catu Daya:** Hubungkan catu daya utama ke jalur daya mikrokontroler. Pasang kapasitor *decoupling* 100nF di dekat pin VCC untuk meredam ripple switching.
2. **Pemasangan Sensor & Modul:** Sambungkan jalur sinyal sensor ke pin mikrokontroler yang telah ditentukan. Gunakan resistor pull-up 4.7kΩ pada jalur SDA/SCL jika menggunakan modul I2C.
3. **Pemasangan Aktuator:** Hubungkan modul relay / gate driver MOSFET ke pin kontrol output. Pasang dioda *flyback* (1N4007) pada beban induktif untuk mengeliminasi lonjakan tegangan balik (*back-EMF*).
4. **Pemasangan Tombol Emergency Stop:** Sambungkan tombol darurat ke pin interupsi eksternal dengan konfigurasi *Active-LOW* menggunakan resistor *pull-up*.
5. **Verifikasi Koneksi:** Lakukan pengecekan jalur ground bersama (*Common Ground*) pada seluruh modul sebelum menyalakan daya.

---

## 🚀 Panduan Kompilasi & Upload (Arduino IDE)

1. Buka **Arduino IDE 2.0+**.
2. Masuk ke menu **Tools > Board**:
   * Pilih **`Arduino Uno`**.
3. Pastikan dependensi pustaka terpasang via Library Manager:
   * `ArduinoJson`
   * `Wire` & `SPI`
   * `EEPROM`
4. Buka berkas [`arduino-uno-gas-chromatograph-oven.ino`](./arduino-uno-gas-chromatograph-oven.ino).
5. Klik tombol **Verify** (✓) kemudian **Upload** (➔).
6. Buka **Serial Monitor** pada baudrate **`115200`** untuk melihat streaming telemetri dan status operasional.

---

## 📄 Lisensi
Didistribusikan di bawah lisensi open-source **MIT License**. Dikembangkan oleh **Muhammad Fikri**.
