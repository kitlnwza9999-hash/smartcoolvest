# 🦺 SmartCool Vest (เสื้อกั๊กปรับอากาศอัจฉริยะ)

> **ระบบควบคุมเสื้อกั๊กระบายความร้อนอัจฉริยะด้วย ESP32-S3 ผ่าน Web Bluetooth (BLE) และแบบจำลอง 3D Digital Twin**  
> ออกแบบสำหรับผู้ปฏิบัติงานกลางแจ้งหรือในโรงงานอุตสาหกรรม โดยยึดหลักความปลอดภัยของแบตเตอรี่และการใช้งานที่ง่ายที่สุด (Glove-first & Sunlight-legible UX)

---

## 📌 จุดเด่นของระบบ (Key Features)

1. **การควบคุมพัดลมจริงด้วยคลื่นความถี่สูง (25 kHz Ultrasonic PWM):**
   - ความถี่ 25 kHz เหนือย่านความถี่เสียงมนุษย์ ตัดเสียงหวีดรบกวนของมอเตอร์ (No Coil Whine)
   - **ระบบ Kickstart (เร่งออกตัว):** จ่ายพัลส์ 75% นาน 180 ms เพื่อเอาชนะแรงเสียดทานเริ่มต้น ป้องกันพัดลมติดขัดตอนเปิดระดับต่ำ
   - **ระบบ Soft-Start (เร่งนุ่มนวล):** ควบคุม Slew Rate ทุก 20 ms ป้องกันไฟกระชากและป้องกันแรงดันไฟ 5V ตกวูบ
   - **Minimum Duty Clamping:** กำหนดค่า Duty ต่ำสุด (~15-18%) ไม่ให้พัดลมหยุดหมุนค้างขณะสั่งงาน
2. **สถาปัตยกรรมระบายความร้อนแยก 3 โซนอิสระ:**
   - **พัดลมตัว 70 mm (×2 ตัว - ช่อง A/B):** ระบายความร้อนแผ่นหลังและลำตัว (0–100%)
   - **พัดลมบ่า/ต้นคอ 40 mm (×2 ตัว):** เป่าลมเย็นมุม 45° สู่ต้นคอและใบหน้า (จำกัดซอฟต์แวร์ไม่เกิน 70% เพื่อความปลอดภัยและเสียง)
   - **พัดลมระบายแบตเตอรี่ BTMS 30 mm (×1 ตัว):** ระบบควบคุมอัตโนมัติเมื่อแบตเตอรี่ $\ge 40^\circ\text{C}$ (ผู้ใช้ปิดไม่ได้)
3. **ระบบความปลอดภัยแบตเตอรี่หลายชั้น (Multi-tier Safety Protection):**
   - **Hardware Fail-Safe:** ขา Gate มีตัวต้านทาน Pull-down $10\,\text{k}\Omega$ ดึงลงกราวด์ทันที พัดลมหยุดหมุนเมื่อบอร์ดรีเซ็ตหรือค้าง
   - **ระดับ 1 ($\ge 40^\circ\text{C}$):** พัดลมแบตเตอรี่เริ่มทำงานอัตโนมัติ (Hysteresis ปิดเมื่อ $< 37^\circ\text{C}$)
   - **ระดับ 2 ($\ge 50^\circ\text{C}$):** ปรับลดกำลังพัดลมตัวและบ่าเหลือ 60% อัตโนมัติ (Thermal Derating)
   - **ระดับ 3 ($\ge 60^\circ\text{C}$):** ตัดวงจรนิรภัยค้างทันที (Hardware Latch Cutoff) พัดลมดับสนิทจนกว่าจะกดรีเซ็ต
4. **หน้าจอควบคุม 3D Interactive Digital Twin & PWA (`index.html`):**
   - พัดลมในโมเดล 3D และละอองลมหมุนเร็วขึ้นตามค่า Telemetry จริงของบอร์ด
   - ระบบ **Auto Mode** ปรับระดับพัดลมอัตโนมัติตามดัชนีความร้อน **Heat Index (สูตร Rothfusz)** พร้อมระบบพรีเซ็ต
   - รองรับทั้งการเชื่อมต่อผ่าน **Web Bluetooth GATT**, **Wi-Fi AP (`http://192.168.4.1`)** และ **โหมด Demo**
   - ปุ่มคีย์แพดขนาดใหญ่ ($\ge 58\text{ px}$) สำหรับการสวมถุงมือ และปุ่ม **⛔ STOP ฉุกเฉินแบบ Sticky** ตรึงอยู่ด้านล่างตลอดเวลา

---

## 🔌 ผังการจัดสรรพิน (ESP32-S3 Hardware Pinout)

| ฟังก์ชัน | พิน ESP32-S3 | Peripheral / ชนิดพิน | รายละเอียด |
|---|:---:|---|---|
| **พัดลมตัว 70mm (×2)** | **GPIO 11** | LEDC PWM (CH0) | ความถี่ 25 kHz, ผ่าน R Gate $100\,\Omega$ + R Pull-down $10\,\text{k}\Omega$ ไป GND |
| **พัดลมบ่า 40mm (×2)** | **GPIO 12** | LEDC PWM (CH1) | ความถี่ 25 kHz, ผ่าน R Gate $100\,\Omega$ + R Pull-down $10\,\text{k}\Omega$ ไป GND (จำกัด $\le 70\%$) |
| **พัดลมแบตเตอรี่ 30mm** | **GPIO 13** | LEDC PWM (CH2) | ความถี่ 25 kHz, ผ่าน R Gate $100\,\Omega$ + R Pull-down $10\,\text{k}\Omega$ ไป GND (ระบบคุมอัตโนมัติ) |
| **ไฟสถานะระบบ RGB** | **GPIO 48** | Built-in WS2812 | สีเขียวจาง = รอเชื่อมต่อ, สีน้ำเงิน = เชื่อมต่อ BLE สำเร็จ |
| *(เฟส 2) วัดแรงดันแบต* | **GPIO 1** | ADC1_CH0 | ผ่านวงจรแบ่งแรงดัน (Voltage Divider) |
| *(เฟส 2) NTC แบตเตอรี่* | **GPIO 2** | ADC1_CH1 | อนุกรมกับตัวต้านทาน $10\,\text{k}\Omega$ 1% ไป 3.3V |
| *(เฟส 2) I2C SDA (SHT31)* | **GPIO 4** | I2C SDA | เซนเซอร์วัดอุณหภูมิและความชื้นอากาศ |
| *(เฟส 2) I2C SCL (SHT31)* | **GPIO 5** | I2C SCL | สัญญาณนาฬิกา I2C |
| *(เฟส 2) 1-Wire (DS18B20)* | **GPIO 6** | 1-Wire DQ | วัดอุณหภูมิผิวสัมผัสเสื้อกั๊ก (Pull-up $4.7\,\text{k}\Omega$) |

> [!CAUTION]
> **ข้อควรระวังของ ESP32-S3:** ห้ามใช้งานพิน GPIO 26 ถึง GPIO 37 เด็ดขาด เนื่องจากเชื่อมต่ออยู่กับ Octal Flash / Octal PSRAM ภายในชิป

---

## 📡 โปรโตคอลการสื่อสาร (BLE GATT & Wi-Fi)

* **ชื่ออุปกรณ์ BLE:** `SmartVest-Test`
* **Service UUID:** `7d1c0001-2f4b-4c1a-9a53-5b0e6f6c0a01`
* **Command Characteristic (WRITE / WRITE NR):** `7d1c0002-2f4b-4c1a-9a53-5b0e6f6c0a01`
  - รูปแบบอาร์เรย์ไบต์ `[cmd, val, a, b]`:
    - `1`: พัดลมตัว % (`val` = 0–100)
    - `2`: พัดลมบ่า % (`val` = 0–70)
    - `3`: เลือกระดับความแรง (`val` = 0, 1, 2, 3, 4, 5)
    - `4`: STOP หยุดฉุกเฉิน
    - `5`: รีเซ็ตระบบ
    - `6`: สลับจำลองแบตร้อน
    - `7`: รันโปรแกรมอัตโนมัติ (`val` = 0: Pre-cool, 1: Ramp-up, 2: Custom)
    - `8`: หยุดโปรแกรมอัตโนมัติ
    - `9`: บันทึก Custom 4 เฟสลง NVS `[9, phaseIndex 0–3, level 0–5, minutes 0–255]`
    - `12`: โหมดทดสอบเร็ว (`val` = 1: 1 นาที = 6 วินาที, 0: ปกติ)
* **Telemetry Characteristic (NOTIFY / READ ทุก 500 ms):** `7d1c0003-2f4b-4c1a-9a53-5b0e6f6c0a01`
  - ขนาด 12 ไบต์ (Little-Endian):
    `int16 Ta×10 | u8 Rh | int16 Tb×10 | u8 SOC | u8 FanBody% | u8 FanNeck% | u8 FanBat% | u8 Flags | u8 Phase | u8 Program`
* **Wi-Fi AP สำรอง:** SSID `SmartVest-Test` รหัสผ่าน `12345678` (IP: `http://192.168.4.1`)

---

## 🛠️ โครงสร้างโปรเจกต์ (Project Structure)

```text
smartcoolvest/
├── include/              # Header files เพิ่มเติม
├── lib/                  # ไลบรารีเฉพาะโปรเจกต์
├── src/
│   ├── main.cpp          # ซอร์สโค้ดหลัก ESP32-S3 (PWM, BLE, Wi-Fi, Safety Logic)
│   └── webpage.h         # หน้าเว็บ UI ที่ถูกแปลงเป็น PROGMEM ฝังในบอร์ด
├── index.html            # หน้าเว็บ UI เดี่ยวฉบับเต็ม (สำหรับ Deploy ขึ้น Vercel / PWA)
├── platformio.ini        # คอนฟิกูเรชันบอร์ด ESP32-S3 DevKitC-1
└── README.md             # เอกสารคู่มือโครงการ
```

---

## 🚀 วิธีการคอมไพล์และอัปโหลด (Build & Flash)

### ผ่าน PlatformIO Core (CLI):
```powershell
# คอมไพล์โปรเจกต์
pio run

# อัปโหลดลงบอร์ด ESP32-S3
pio run -t upload

# เปิด Serial Monitor (115200 baud)
pio device monitor
```

### การใช้งานหน้าเว็บ PWA:
- นำไฟล์ `index.html` ไปวางบน Vercel, Netlify หรือ GitHub Pages (เปิดใช้ HTTPS)
- เปิดผ่านเบราว์เซอร์ Chrome บนมือถือ Android แล้วเลือก **"เพิ่มลงในหน้าจอหลัก (Install PWA)"** เพื่อเชื่อมต่อ Web Bluetooth ได้อย่างราบรื่น
