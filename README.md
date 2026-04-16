# Oscilloscope-Using-ESP32
This project is a low-cost digital oscilloscope built using the ESP32 microcontroller. It captures analog signals using ADC and displays the waveform in real-time on an OLED screen.

🚀 Features
Real-time waveform display
Voltage measurement (Vpp, Vrms)
Frequency measurement
Adjustable V/div and T/div
Run/Stop functionality
Compact and portable design


🎯 Objective
Design low-cost oscilloscope using ESP32
Display analog signals in real-time
Provide signal analysis (voltage & frequency)
Help students understand waveform behavior


🧠 Working Principle
Input signal is applied to ESP32 ADC
ADC converts analog signal → digital values
Data is processed (scaling + filtering)
Waveform displayed on OLED screen
Measurements like Vpp, Vrms, frequency are calculated
FIR filter (noise removal)
FFT (frequency analysis)


🧩 System Architecture
Main blocks:
Signal Input
ESP32 ADC
Signal Processing
FIR Filter
FFT Analysis
OLED Display


🛠️ Hardware Used
ESP32-WROOM-32
OLED Display (128×64)
Resistors (voltage divider)
Push Buttons (controls)
Capacitor (filter)
Power Supply (USB 5V)


💻 Software Used
Arduino IDE (C/C++)
ESP32 Board Package
Adafruit GFX Library
I2C Communication (Wire Library)


⚙️ Important Calculations

Voltage conversion:
V = (ADC / 4095) × 3.3
Peak-to-Peak Voltage:
Vpp = Vmax - Vmin
These are used for accurate signal measurement.

🧪 Testing

Performed tests:
ADC reading verification
Waveform display check
Button functionality test
Voltage & frequency accuracy

👉 Testing includes:

Unit testing
Integration testing
Functional testing


📊 Results
Successfully displayed sine, square, triangular waves
Accurate Vpp and Vrms values
Frequency measurement working
Smooth waveform scrolling
Vpp accuracy ≈ correct
Vrms error < 2%


⚖️ Comparison
Feature	ESP32 Scope	DSO	CRO
Cost	Very Low	High	Medium
Portability	High	Low	Low
Accuracy	Medium	High	Medium


⚡ Advantages
Low cost
Portable
Easy to use
Real-time display
Good for learning


⚠️ Limitations
Limited frequency range (few kHz)
Lower accuracy than professional scope
Limited ADC resolution
Single channel


🌍 Applications
Electronics labs
DIY projects
Signal testing
Educational tool
Circuit debugging


🔮 Future Scope
Multi-channel oscilloscope
Higher sampling rate
Mobile app integration
External ADC for better accuracy
