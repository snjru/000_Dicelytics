# Dicelytics Estimated BOM Cost Table

| Component | Recommended Model | Estimated Cost (JPY) | Cost Optimization Key Points |
| :--- | :--- | :---: | :--- |
| **Main SBC** | Raspberry Pi Zero 2 W | ¥3,000 | Minimizes SBoM cost; optimized via C++ binary builds |
| **Camera** | OV5647-compatible Camera Module | ¥1,000 | Equivalent sensor to official v1; provides sufficient resolution |
| **Motor** | FA-130RA (130-size DC Motor) | ¥150 | Mass-produced standard component; compatible with Tamiya parts |
| **Motor Driver** | DRV8835 Module | ¥400 | Compact and low-cost 2-channel H-bridge driver |
| **Display** | SSD1306 0.96" OLED (I2C) | ¥400 | Optional (Cost drops to ¥0 if served via Web UI) |
| **Power & Wiring** | 5V USB Power Supply + Logic Isolation Circuit | ¥500 | Includes capacitors and diodes for motor noise suppression |
| **Enclosure** | 3D Printed Parts (PLA) + Sheet Metal / Acrylic | ¥500 | 3D print functional mechanisms only; use off-the-shelf outer panels |
| **Total** | **Total BOM Cost** | **¥5,950** | **Complete standalone kit for under ¥10,000** |
