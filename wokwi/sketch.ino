// ==============================================================================
// SmartCool Vest - Wokwi Circuit & Power Simulation Sketch (ESP32-S3)
// วงจรจำลองระบบไฟ เซนเซอร์ และพัดลมเสมือนจริงบน Wokwi
// ==============================================================================

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DHTesp.h>

// ------------------------------------------------------------------------------
// 1. ผังการจัดสรรพินฮาร์ดแวร์ (Hardware Pinout)
// ------------------------------------------------------------------------------
#define PIN_FAN_BODY     11    // GPIO 11: พัดลมตัว 70 mm (Blue LED)
#define PIN_FAN_NECK     12    // GPIO 12: พัดลมบ่า 40 mm (Cyan LED, Max 70%)
#define PIN_FAN_BAT      13    // GPIO 13: พัดลมระบายบอร์ด/แบตเตอรี่ 30 mm (Orange LED)

#define PIN_BAT_VOLT     1     // GPIO 1: ADC1_CH0 จำลองแรงดันแบตเตอรี่ (Potentiometer)
#define PIN_NTC_BAT      2     // GPIO 2: ADC1_CH1 วัดอุณหภูมิแบตเตอรี่จริง (NTC Sensor)
#define PIN_DHT_ENV      4     // GPIO 4: เซนเซอร์วัดอุณหภูมิ/ความชื้นรอบตัว (DHT22)
#define PIN_BTN_STOP     10    // GPIO 10: สวิตช์ปุ่มกดหยุดฉุกเฉินภายนอก (Pushbutton Active LOW)

// ------------------------------------------------------------------------------
// 2. คอนฟิกูเรชัน PWM พัดลม (25 kHz Ultrasonic PWM & Soft-Start)
// ------------------------------------------------------------------------------
#define PWM_FREQ         25000 // 25,000 Hz Ultrasonic PWM
#define PWM_RES          8     // ความละเอียด 8-bit (0 - 255)
#define MIN_DUTY_BODY    38    // Duty ขั้นต่ำ ~15%
#define MIN_DUTY_NECK    38    // Duty ขั้นต่ำ ~15%
#define MIN_DUTY_BAT     45    // Duty ขั้นต่ำ ~18%
#define SLEW_STEP        8     // Soft-Start Slew Rate ต่อรอบ 20ms

// ------------------------------------------------------------------------------
// 3. ตัวแปรสถานะระบบ (System State)
// ------------------------------------------------------------------------------
DHTesp dht;
WebServer server(80);

float ta = 30.0f;       // อุณหภูมิอากาศ (°C)
int   rh = 55;          // ความชื้นสัมพัทธ์ (%)
float tb = 28.0f;       // อุณหภูมิแบตเตอรี่ NTC (°C)
float soc = 85.0f;      // แบตเตอรี่คงเหลือ (%)
float batVolt = 3.9f;   // แรงดันแบตเตอรี่ (V)

int setB = 0;           // คำสั่งพัดลมตัว (0 - 100%)
int setN = 0;           // คำสั่งพัดลมบ่า (0 - 70%)
int effB = 0;           // กำลังพัดลมตัวหลังผ่าน Safety Check
int effN = 0;           // กำลังพัดลมบ่าหลังผ่าน Safety Check
int batFan = 0;         // กำลังพัดลมแบตเตอรี่ (0 - 100%)

bool stopped       = false; // ธงสั่งหยุดฉุกเฉิน (STOP)
bool cut           = false; // ธงตัดวงจรนิรภัยค้าง (Latch Cutoff เมื่อ Tb >= 60°C)
bool batFanEnabled = true;  // สวิตช์พัดลมบอร์ด/แบตเตอรี่ทำงานตลอด

// ตัวขับเคลื่อน Soft-Start
struct FanState {
  uint8_t targetDuty = 0;
  uint8_t currentDuty = 0;
};
FanState fanBody, fanNeck, fanBat;

unsigned long lastSensorRead = 0;
unsigned long lastPwmUpdate  = 0;
unsigned long lastDashPrint  = 0;

const int LV[6] = {0, 20, 40, 60, 80, 100};

// ------------------------------------------------------------------------------
// 4. ฟังก์ชันควบคุม PWM & Soft-Start
// ------------------------------------------------------------------------------
uint8_t percentToDuty(int pct, uint8_t minDuty) {
  if (pct <= 0) return 0;
  pct = constrain(pct, 0, 100);
  uint8_t duty = (uint8_t)map(pct, 0, 100, 0, 255);
  return max(duty, minDuty);
}

void updateDriver(FanState &f, uint8_t pin, uint8_t targetDuty) {
  f.targetDuty = targetDuty;
  if (f.currentDuty < f.targetDuty) {
    f.currentDuty = (uint8_t)min((int)f.targetDuty, (int)f.currentDuty + SLEW_STEP);
  } else if (f.currentDuty > f.targetDuty) {
    f.currentDuty = (uint8_t)max((int)f.targetDuty, (int)f.currentDuty - SLEW_STEP);
  }
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
  ledcWrite(pin, f.currentDuty);
#else
  // Fallback for older core mapping
  ledcWrite(pin == PIN_FAN_BODY ? 0 : (pin == PIN_FAN_NECK ? 1 : 2), f.currentDuty);
#endif
}

// ------------------------------------------------------------------------------
// 5. การอ่านค่าเซนเซอร์จริงใน Wokwi
// ------------------------------------------------------------------------------
void readSensors() {
  // 1. อ่าน DHT22 (อากาศรอบตัว)
  TempAndHumidity dhtData = dht.getTempAndHumidity();
  if (!isnan(dhtData.temperature)) {
    ta = dhtData.temperature;
    rh = (int)dhtData.humidity;
  }

  // 2. อ่าน NTC Sensor บน GPIO 2 (Wokwi NTC Module มี DAC/ADC Converter ในตัว)
  int rawNTC = analogRead(PIN_NTC_BAT);
  // คำนวณอุณหภูมิจาก NTC 10k (B=3950)
  if (rawNTC > 20 && rawNTC < 4070) {
    float v = (rawNTC / 4095.0f) * 3.3f;
    float r = 10000.0f * (3.3f - v) / v;
    if (r > 0) {
      float steinhart = r / 10000.0f;
      steinhart = log(steinhart);
      steinhart /= 3950.0f;
      steinhart += 1.0f / (25.0f + 273.15f);
      steinhart = 1.0f / steinhart;
      tb = constrain(steinhart - 273.15f, -20.0f, 100.0f);
    }
  }

  // 3. อ่าน Potentiometer บน GPIO 1 (จำลองระดับแบตเตอรี่)
  int rawPot = analogRead(PIN_BAT_VOLT);
  batVolt = (rawPot / 4095.0f) * 4.2f; // จำลอง 0 - 4.2V
  soc = constrain((rawPot / 4095.0f) * 100.0f, 0.0f, 100.0f);

  // 4. ตรวจจับปุ่มกด Emergency STOP (GPIO 10)
  if (digitalRead(PIN_BTN_STOP) == LOW) {
    if (!stopped) {
      stopped = true;
      Serial.println("\n[EMERGENCY] ⛔ กดปุ่มฉุกเฉินฮาร์ดแวร์ (GPIO 10)! ดับพัดลมทันที");
    }
  }

  // ----------------------------------------------------------------------------
  // ระบบความปลอดภัยหลายชั้น (Multi-tier Safety Rules)
  // ----------------------------------------------------------------------------
  effB = setB;
  effN = setN;

  // กฎระดับ 1: Tb >= 50°C -> ลดโหลดพัดลมเหลือ 60%
  if (tb >= 50.0f) {
    effB = (effB * 6) / 10;
    effN = (effN * 6) / 10;
  }

  // กฎระดับ 2: Tb >= 60°C -> ตัดวงจรนิรภัยค้างทันที (Hardware Latch Cutoff)
  if (tb >= 60.0f) {
    if (!cut) {
      cut = true;
      Serial.println("\n[CRITICAL SAFETY] 🚨 อุณหภูมิแบตเตอรี่ >= 60°C! ตัดวงจรนิรภัยค้าง (LATCH CUTOFF)");
    }
  }

  if (cut || stopped) {
    effB = 0;
    effN = 0;
  }

  // กฎระดับ 3: การทำงานของพัดลมแบตเตอรี่ (30mm)
  if (cut) {
    batFan = 0;
  } else if (tb >= 50.0f) {
    batFan = 100; // เร่งเต็มที่เมื่อแบตร้อนจัด
  } else if (tb >= 40.0f) {
    batFan = 60;  // เร่งปานกลางเพื่อระบายความร้อน
  } else if (batFanEnabled) {
    batFan = 40;  // ทำงานระดับสแตนด์บาย 40% ตลอดเวลา
  } else {
    batFan = 0;
  }
}

// ------------------------------------------------------------------------------
// 6. การจัดการคำสั่ง (Command Handler)
// ------------------------------------------------------------------------------
void executeCommand(String cmd) {
  cmd.trim();
  cmd.toLowerCase();

  if (cmd == "stop") {
    stopped = true;
    Serial.println(">> คำสั่ง: ⛔ STOP หยุดฉุกเฉิน");
  } else if (cmd == "reset") {
    stopped = false;
    cut = false;
    Serial.println(">> คำสั่ง: 🔄 รีเซ็ตระบบ (RESET) ปลดล็อกการตัดวงจร");
  } else if (cmd == "0") {
    setB = 0; setN = 0; stopped = false;
    Serial.println(">> ปิดพัดลม (ระดับ 0 / OFF)");
  } else if (cmd >= "1" && cmd <= "5") {
    int lvl = cmd.toInt();
    setB = LV[lvl];
    setN = min(LV[lvl], 70);
    stopped = false;
    Serial.printf(">> ตั้งระดับแรงลมเป็น: ระดับ %d (ตัว=%d%%, บ่า=%d%%)\n", lvl, setB, setN);
  } else if (cmd == "bat") {
    batFanEnabled = !batFanEnabled;
    Serial.printf(">> สลับพัดลมบอร์ด/แบตเตอรี่: %s\n", batFanEnabled ? "เปิด (ทำงานตลอด)" : "ปิด");
  } else if (cmd == "help") {
    Serial.println("\n--- รายการคำสั่งที่พิมพ์สั่งได้ใน Serial Monitor ---");
    Serial.println("  0 - 5 : ปรับระดับพัดลม (0 = OFF, 1=20%, 2=40%, 3=60%, 4=80%, 5=100%)");
    Serial.println("  stop  : สั่งหยุดฉุกเฉิน (STOP)");
    Serial.println("  reset : รีเซ็ตระบบปลดล็อก Latch Cutoff");
    Serial.println("  bat   : เปิด/ปิด พัดลมระบายบอร์ด 30mm");
    Serial.println("--------------------------------------------------\n");
  } else {
    Serial.println(">> ไม่รู้จักคำสั่ง พิมพ์ 'help' เพื่อดูคำสั่งทั้งหมด");
  }
}

// ------------------------------------------------------------------------------
// 7. แดชบอร์ดมอนิเตอร์บน Serial
// ------------------------------------------------------------------------------
void printDashboard() {
  Serial.print("\033[H\033[J"); // Clear ANSI terminal
  Serial.println("=================================================================");
  Serial.println("       🦺 SMARTCOOL VEST - REAL-TIME WOKWI SIMULATION DASHBOARD  ");
  Serial.println("=================================================================");
  
  // สถานะความปลอดภัย
  Serial.print(" [SYSTEM STATUS] : ");
  if (cut) {
    Serial.println("🚨 [LATCH CUTOFF] แบตร้อน >= 60°C วงจรตัดค้าง! พิมพ์ 'reset' เพื่อเริ่มใหม่");
  } else if (stopped) {
    Serial.println("⛔ [EMERGENCY STOPPED] พัดลมหยุดฉุกเฉิน พิมพ์ 'reset' หรือเลือกระดับ");
  } else if (tb >= 50.0f) {
    Serial.println("⚠️ [THERMAL DERATING] แบต >= 50°C ลดกำลังลมเหลือ 60%");
  } else {
    Serial.println("🟢 [NORMAL] ระบบทำงานปกติ");
  }

  // เซนเซอร์
  Serial.println("-----------------------------------------------------------------");
  Serial.printf("  🌡️ อุณหภูมิอากาศ (Ta) : %5.1f °C    💧 ความชื้น (Rh)   : %3d %%\n", ta, rh);
  Serial.printf("  🔋 อุณหภูมิแบต (Tb)  : %5.1f °C    ⚡ ความจุแบต (SOC) : %3.0f %% (%.2fV)\n", tb, soc, batVolt);

  // เอาต์พุต PWM & หลอดไฟ LED
  Serial.println("-----------------------------------------------------------------");
  Serial.printf("  [LED BLUE]   พัดลมตัว 70mm (GPIO 11) : %3d %% (PWM Duty: %3d/255)\n", effB, fanBody.currentDuty);
  Serial.printf("  [LED CYAN]   พัดลมบ่า 40mm (GPIO 12) : %3d %% (PWM Duty: %3d/255) [Max 70%%]\n", effN, fanNeck.currentDuty);
  Serial.printf("  [LED ORANGE] พัดลมแบต 30mm (GPIO 13) : %3d %% (PWM Duty: %3d/255) [%s]\n", 
    batFan, fanBat.currentDuty, batFanEnabled ? "Always ON" : "Off");
  Serial.println("=================================================================");
  Serial.println("💡 ลองเลื่อนตัวเลื่อนที่ NTC / Potentiometer หรือพิมพ์ '1'-'5' ในช่องด้านล่าง");
}

// ------------------------------------------------------------------------------
// 8. SETUP
// ------------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);

  // ตั้งค่าปุ่มกดหยุดฉุกเฉิน
  pinMode(PIN_BTN_STOP, INPUT_PULLUP);

  // กำหนด PWM 25 kHz สำหรับพัดลมทั้ง 3 ตัว
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
  ledcAttach(PIN_FAN_BODY, PWM_FREQ, PWM_RES);
  ledcAttach(PIN_FAN_NECK, PWM_FREQ, PWM_RES);
  ledcAttach(PIN_FAN_BAT,  PWM_FREQ, PWM_RES);
#else
  ledcSetup(0, PWM_FREQ, PWM_RES);
  ledcAttachPin(PIN_FAN_BODY, 0);
  ledcSetup(1, PWM_FREQ, PWM_RES);
  ledcAttachPin(PIN_FAN_NECK, 1);
  ledcSetup(2, PWM_FREQ, PWM_RES);
  ledcAttachPin(PIN_FAN_BAT, 2);
#endif

  // เริ่มต้น DHT22
  dht.setup(PIN_DHT_ENV, DHTesp::DHT22);

  // ตั้งค่า ADC
  analogReadResolution(12);

  // เชื่อมต่อ Wi-Fi ใน Wokwi (Wokwi-GUEST ไม่ต้องมีรหัสผ่าน)
  WiFi.begin("Wokwi-GUEST", "", 6);

  // Web API endpoint
  server.on("/api/state", []() {
    char json[220];
    snprintf(json, sizeof(json),
      "{\"ta\":%.1f,\"rh\":%d,\"tb\":%.1f,\"soc\":%d,\"b\":%d,\"n\":%d,\"bat\":%d,\"f\":%d}",
      ta, rh, tb, (int)soc, effB, effN, batFan, ((cut || stopped) ? 1 : 0) | (tb >= 50.0f ? 4 : 0)
    );
    server.send(200, "application/json", json);
  });

  server.on("/api/cmd", []() {
    int c = server.arg("c").toInt();
    int v = server.arg("v").toInt();
    if (c == 3) executeCommand(String(v));
    else if (c == 4) executeCommand("stop");
    else if (c == 5) executeCommand("reset");
    server.send(200, "text/plain", "ok");
  });

  server.begin();

  delay(500);
  Serial.println("\n\n[WOKWI READY] ระบบ SmartCool Vest เริ่มต้นพร้อมใช้งาน!");
  Serial.println("พิมพ์ 'help' ในช่องส่งข้อความด้านล่าง เพื่อดูคำสั่งทั้งหมด\n");
}

// ------------------------------------------------------------------------------
// 9. LOOP
// ------------------------------------------------------------------------------
void loop() {
  server.handleClient();

  // รับคำสั่งจาก Serial Monitor
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    executeCommand(input);
  }

  unsigned long now = millis();

  // อัปเดต PWM ทุก 20 ms ด้วย Soft-Start
  if (now - lastPwmUpdate >= 20) {
    lastPwmUpdate = now;
    uint8_t targetB = percentToDuty(effB, MIN_DUTY_BODY);
    uint8_t targetN = percentToDuty(effN, MIN_DUTY_NECK);
    uint8_t targetBat = percentToDuty(batFan, MIN_DUTY_BAT);

    updateDriver(fanBody, PIN_FAN_BODY, targetB);
    updateDriver(fanNeck, PIN_FAN_NECK, targetN);
    updateDriver(fanBat,  PIN_FAN_BAT,  targetBat);
  }

  // อ่านเซนเซอร์ทุก 200 ms
  if (now - lastSensorRead >= 200) {
    lastSensorRead = now;
    readSensors();
  }

  // อัปเดตแดชบอร์ดบน Serial ทุก 1000 ms
  if (now - lastDashPrint >= 1000) {
    lastDashPrint = now;
    printDashboard();
  }
}
