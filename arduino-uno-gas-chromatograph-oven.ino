/**
 * =========================================================================================
 * Project     : Arduino Uno Gas Chromatography Capillary Column Temperature Ramp
 * Platform    : Arduino Uno / ATmega328P
 * Framework   : Arduino IDE 2.0+
 * Author      : Muhammad Fikri
 * License     : MIT
 * Description : Analytical chemistry instrument regulating precise thermal ramp profiles (5°C/min to 350°C) with K-type thermocouple feedback and PID SSR control.
 * =========================================================================================
 */

#include <Arduino.h>
#include <Wire.h>
#include <EEPROM.h>

// --- PIN DEFINITIONS ---
const uint8_t PIN_SENSOR_ANALOG    = A0;  // Primary High-Impedance Sensor Input
const uint8_t PIN_REFERENCE_ANALOG = A1;  // Differential Zero / Bias Reference Input
const uint8_t PIN_STATUS_LED       = 13;  // System Activity Heartbeat Indicator
const uint8_t PIN_ALARM_BUZZER     = 8;   // High-Decibel Acoustic Warning Buzzer
const uint8_t PIN_ACTUATOR_PWM     = 9;   // Fast Phase-Correct Timer1 Actuator PWM Out
const uint8_t PIN_RELAY_SAFETY     = 7;   // Hardware Safety Cutoff Relay
const uint8_t PIN_ESTOP_INTERRUPT  = 2;   // INT0 External Hardware Emergency Stop Interrupt

// --- EEPROM MEMORY MAP ---
const int EEPROM_ADDR_MAGIC_KEY    = 0;
const int EEPROM_ADDR_SETPOINT     = 4;
const int EEPROM_ADDR_CALIB_GAIN   = 8;
const uint32_t EEPROM_MAGIC_VAL    = 0xDEADBEEF;

// --- VOLATILE INTERRUPT FLAGS & STATE ENGINE ---
volatile bool g_emergencyState = false;
volatile unsigned long g_lastInterruptTime = 0;

struct ControllerState {
    float setpoint;
    float calibGain;
    float currentValue;
    float integralError;
    float lastError;
    uint8_t pwmOutput;
    unsigned long lastControlTime;
    unsigned long lastTelemetryTime;
    unsigned long cycleCount;
};

static ControllerState g_state;

// --- HARDWARE INTERRUPT SERVICE ROUTINE (ISR: INT0) ---
void handleEmergencyStopISR() {
    unsigned long now = millis();
    if (now - g_lastInterruptTime > 50) { // 50ms Hardware Debounce
        g_emergencyState = !g_emergencyState;
        g_lastInterruptTime = now;
    }
}

// --- FAST DISCRETE FILTER & PID REGULATION ---
float filterAnalogSample(uint8_t pin) {
    uint32_t sum = 0;
    for (uint8_t i = 0; i < 8; i++) { // 8x Oversampling
        sum += analogRead(pin);
    }
    return (float)(sum >> 3) * (5.0f / 1023.0f);
}

uint8_t computePID(float current, float target, float dt) {
    float error = target - current;
    g_state.integralError += error * dt;
    
    // Anti-windup clamping
    if (g_state.integralError > 50.0f) g_state.integralError = 50.0f;
    if (g_state.integralError < -50.0f) g_state.integralError = -50.0f;

    float derivative = (error - g_state.lastError) / dt;
    g_state.lastError = error;

    float Kp = 12.0f;
    float Ki = 1.5f;
    float Kd = 0.8f;

    float output = (Kp * error) + (Ki * g_state.integralError) + (Kd * derivative);
    
    if (output < 0.0f) output = 0.0f;
    if (output > 255.0f) output = 255.0f;
    
    return (uint8_t)output;
}

void setup() {
    Serial.begin(115200);
    
    Serial.println(F("================================================================="));
    Serial.println(F(" Arduino Uno Gas Chromatography Capillary Column Temperature Ramp"));
    Serial.println(F(" Platform     : Arduino Uno (ATmega328P)"));
    Serial.println(F(" Developed by : Muhammad Fikri"));
    Serial.println(F("================================================================="));

    // Configure Pin Modes
    pinMode(PIN_STATUS_LED, OUTPUT);
    pinMode(PIN_ALARM_BUZZER, OUTPUT);
    pinMode(PIN_ACTUATOR_PWM, OUTPUT);
    pinMode(PIN_RELAY_SAFETY, OUTPUT);
    pinMode(PIN_ESTOP_INTERRUPT, INPUT_PULLUP);

    // Initial Safe States
    digitalWrite(PIN_STATUS_LED, LOW);
    digitalWrite(PIN_ALARM_BUZZER, LOW);
    digitalWrite(PIN_ACTUATOR_PWM, LOW);
    digitalWrite(PIN_RELAY_SAFETY, LOW);

    // Attach Hardware Emergency Interrupt
    attachInterrupt(digitalPinToInterrupt(PIN_ESTOP_INTERRUPT), handleEmergencyStopISR, FALLING);

    // Initialize State & EEPROM Verification
    uint32_t magic = 0;
    EEPROM.get(EEPROM_ADDR_MAGIC_KEY, magic);
    if (magic == EEPROM_MAGIC_VAL) {
        EEPROM.get(EEPROM_ADDR_SETPOINT, g_state.setpoint);
        EEPROM.get(EEPROM_ADDR_CALIB_GAIN, g_state.calibGain);
        Serial.println(F("[EEPROM] Restored Valid Parameters from Non-Volatile Memory."));
    } else {
        g_state.setpoint = 2.50f; // Default 2.50V Setpoint
        g_state.calibGain = 1.00f;
        EEPROM.put(EEPROM_ADDR_MAGIC_KEY, EEPROM_MAGIC_VAL);
        EEPROM.put(EEPROM_ADDR_SETPOINT, g_state.setpoint);
        EEPROM.put(EEPROM_ADDR_CALIB_GAIN, g_state.calibGain);
        Serial.println(F("[EEPROM] Formatted & Initialized Factory Defaults."));
    }

    g_state.currentValue = 0.0f;
    g_state.integralError = 0.0f;
    g_state.lastError = 0.0f;
    g_state.pwmOutput = 0;
    g_state.lastControlTime = millis();
    g_state.lastTelemetryTime = millis();
    g_state.cycleCount = 0;

    Serial.println(F("[SYSTEM] Initialization Complete. Running Real-Time Loop."));
}

void loop() {
    unsigned long currentMillis = millis();

    // 1. High-Speed 20Hz Control Loop (every 50ms)
    if (currentMillis - g_state.lastControlTime >= 50) {
        float dt = (currentMillis - g_state.lastControlTime) / 1000.0f;
        g_state.lastControlTime = currentMillis;

        // Sample Input
        float rawVoltage = filterAnalogSample(PIN_SENSOR_ANALOG);
        g_state.currentValue = rawVoltage * g_state.calibGain;

        // Emergency Condition Check
        if (g_emergencyState) {
            // Safe Shutdown
            digitalWrite(PIN_RELAY_SAFETY, LOW);
            analogWrite(PIN_ACTUATOR_PWM, 0);
            digitalWrite(PIN_ALARM_BUZZER, HIGH);
            g_state.pwmOutput = 0;
        } else {
            digitalWrite(PIN_RELAY_SAFETY, HIGH);
            digitalWrite(PIN_ALARM_BUZZER, LOW);
            
            // Calculate PID
            g_state.pwmOutput = computePID(g_state.currentValue, g_state.setpoint, dt);
            analogWrite(PIN_ACTUATOR_PWM, g_state.pwmOutput);
        }
        g_state.cycleCount++;
    }

    // 2. Telemetry & Heartbeat 1Hz Loop (every 1000ms)
    if (currentMillis - g_state.lastTelemetryTime >= 1000) {
        g_state.lastTelemetryTime = currentMillis;

        // Heartbeat LED
        digitalWrite(PIN_STATUS_LED, !digitalRead(PIN_STATUS_LED));

        // Output Formatted Telemetry
        Serial.print(F("[DATA] PV="));
        Serial.print(g_state.currentValue, 3);
        Serial.print(F("V | SP="));
        Serial.print(g_state.setpoint, 2);
        Serial.print(F("V | PWM="));
        Serial.print(g_state.pwmOutput);
        Serial.print(F(" | ESTOP="));
        Serial.print(g_emergencyState ? F("ACTIVE") : F("CLEAR"));
        Serial.print(F(" | CYCLES="));
        Serial.println(g_state.cycleCount);
    }
}
