// ==============================================================================
// SmartCool Vest - Main Firmware (ESP32-S3)
// เฟิร์มแวร์ควบคุมเสื้อกั๊กปรับอากาศอัจฉริยะ พร้อมระบบความปลอดภัยและการขับ PWM พัดลมจริง
// Phase 1 Desk Test (5V Power Supply): PWM 25 kHz, Soft-Start, Kickstart, Hardware Failsafe
// ==============================================================================

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <Preferences.h>
#include "webpage.h"

// ------------------------------------------------------------------------------
// การจัดสรรพินฮาร์ดแวร์ ESP32-S3 (Safe Pinout Mapping)
// ------------------------------------------------------------------------------
#define PIN_FAN_BODY     11    // GPIO 11: พัดลมตัว 70 mm (×2) ผ่าน Low-Side MOSFET / Driver
#define PIN_FAN_NECK     12    // GPIO 12: พัดลมบ่า 40 mm (×2) จำกัดกำลังลม ≤ 70%
#define PIN_FAN_BAT      13    // GPIO 13: พัดลมระบายความร้อนแบตเตอรี่ 30 mm (ระบบควบคุมอัตโนมัติ)

// ------------------------------------------------------------------------------
// คอนฟิกูเรชัน PWM พัดลม (25 kHz Ultrasonic PWM & Slew Limiter)
// ------------------------------------------------------------------------------
#define PWM_FREQ         25000 // 25,000 Hz: ความถี่มาตรฐานพัดลมคอมพิวเตอร์ ไร้เสียงหวีดรบกวนหูมนุษย์
#define PWM_RES          8     // ความละเอียด 8-bit (ค่า Duty: 0 - 255)
#define MIN_DUTY_BODY    38    // Duty ขั้นต่ำ ~15% (38/255) ป้องกันพัดลมติดขัด/มอเตอร์ไม่หมุน
#define MIN_DUTY_NECK    38    // Duty ขั้นต่ำ ~15% (38/255)
#define MIN_DUTY_BAT     45    // Duty ขั้นต่ำ ~18% (45/255) สำหรับพัดลมจิ๋ว 30mm
#define KICKSTART_DUTY   190   // Duty เร่งออกตัวชั่วขณะ ~75% (190/255) เอาชนะแรงเสียดทานเริ่มต้น
#define KICKSTART_MS     180   // ระยะเวลาเร่งออกตัว 180 ms
#define SLEW_STEP        8     // อัตราเร่ง Duty สูงสุดต่อรอบ Loop (Soft-Start ป้องกันไฟกระชาก)

// ------------------------------------------------------------------------------
// BLE GATT UUIDs & Wi-Fi Hotspot Config
// ------------------------------------------------------------------------------
#define SVC_UUID         "7d1c0001-2f4b-4c1a-9a53-5b0e6f6c0a01"
#define CMD_UUID         "7d1c0002-2f4b-4c1a-9a53-5b0e6f6c0a01"
#define TEL_UUID         "7d1c0003-2f4b-4c1a-9a53-5b0e6f6c0a01"

const char* AP_SSID = "SmartVest-Test";
const char* AP_PASS = "12345678";

// ------------------------------------------------------------------------------
// สถานะการทำงานและตัวแปรระบบ (System State)
// ------------------------------------------------------------------------------
float ta = 34.0f;       // อุณหภูมิอากาศ (°C) - จำลอง
int   rh = 60;          // ความชื้นสัมพัทธ์ (%) - จำลอง
float tb = 30.0f;       // อุณหภูมิแบตเตอรี่ NTC (°C) - จำลอง
float soc = 85.0f;      // แบตเตอรี่คงเหลือ (%) - จำลอง

int setB = 0;           // คำสั่งพัดลมตัว (0 - 100%)
int setN = 0;           // คำสั่งพัดลมบ่า (0 - 70%)
int effB = 0;           // กำลังพัดลมตัวที่ระบบจ่ายจริงหลังผ่าน Safety Scaling
int effN = 0;           // กำลังพัดลมบ่าที่ระบบจ่ายจริงหลังผ่าน Safety Scaling
int batFan = 0;         // กำลังพัดลมแบตเตอรี่ (0 - 100% ระบบคุมอัตโนมัติ)

bool stopped = false;   // ธงสั่งหยุดพัดลม (STOP)
bool hotSim  = false;   // ธงจำลองแบตร้อนเพื่อทดสอบระบบความปลอดภัย
bool cut     = false;   // ธงตัดวงจรนิรภัยค้าง (Latch Cutoff เมื่อ Tb >= 60°C)
bool bleOn   = false;   // สถานะการเชื่อมต่อ Bluetooth BLE

// สถานะการขับ Duty จริงของแต่ละช่อง (สำหรับการทำ Soft-Start และ Kickstart)
struct FanDriverState {
  uint8_t targetDuty = 0;
  uint8_t currentDuty = 0;
  bool isKicking = false;
  unsigned long kickStartTime = 0;
};

FanDriverState fanBody;
FanDriverState fanNeck;
FanDriverState fanBat;

// ------------------------------------------------------------------------------
// โปรแกรมหลายเฟส (Multi-phase Programs: Pre-cool, Ramp-up, Custom บน NVS)
// ------------------------------------------------------------------------------
const int LV[6] = {0, 20, 40, 60, 80, 100};
uint8_t PRE[2][8] = {
  {5, 3, 3, 10, 2, 0, 255, 0},   // 0 = Pre-cool (ระดับ 5×3น. -> 3×10น. -> 2 ต่อเนื่อง)
  {1, 5, 2, 5,  3, 0, 255, 0}    // 1 = Ramp-up (ระดับ 1×5น. -> 2×5น. -> 3 ต่อเนื่อง)
};
uint8_t cust[8] = {5, 2, 3, 5, 1, 0, 255, 0}; // 2 = Custom 4 เฟส (เก็บถาวรใน NVS)

Preferences pf;
bool runningProg = false;
bool fastTest = false;
int pg = 0;                     // รหัสโปรแกรม (0, 1, 2)
int ph = 0;                     // ลำดับเฟสปัจจุบัน (0 = ไม่รัน, 1-4 = เฟส 1-4)
unsigned long phStart = 0;

WebServer web(80);
BLECharacteristic* telChar;
unsigned long lastTick500ms = 0;
unsigned long lastPwmTick20ms = 0;

// ------------------------------------------------------------------------------
// ไฟ RGB แสดงสถานะระบบ
// ------------------------------------------------------------------------------
void setSystemLed(bool connected) {
#ifdef RGB_BUILTIN
  // เขียวจางขณะรอเชื่อมต่อ, น้ำเงินสว่างเมื่อเชื่อมต่อ BLE สำเร็จ
  #if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
    rgbLedWrite(RGB_BUILTIN, 0, connected ? 0 : 8, connected ? 45 : 0);
  #else
    neopixelWrite(RGB_BUILTIN, 0, connected ? 0 : 8, connected ? 45 : 0);
  #endif
#else
  #ifdef LED_BUILTIN
    digitalWrite(LED_BUILTIN, connected ? HIGH : LOW);
  #endif
#endif
}

// ------------------------------------------------------------------------------
// คำนวณรหัสสถานะ Flags (1 ไบต์) ตามโปรโตคอล Telemetry
// bit0: ตัดวงจร / สั่งหยุด
// bit1: จำลองแบตร้อน
// bit2: ลดโหลดพัดลมเนื่องจากความร้อน (Tb >= 50°C)
// ------------------------------------------------------------------------------
uint8_t getTelemetryFlags() {
  return ((cut || stopped) ? 1 : 0) | (hotSim ? 2 : 0) | (tb >= 50.0f ? 4 : 0);
}

// ------------------------------------------------------------------------------
// ควบคุมฮาร์ดแวร์ LEDC PWM แบบข้ามเวอร์ชัน Core ESP32 (v2.x / v3.x)
// ------------------------------------------------------------------------------
void writePwmDuty(uint8_t pin, uint8_t channel, uint8_t duty) {
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
  ledcWrite(pin, duty);
#else
  ledcWrite(channel, duty);
#endif
}

// คำนวณค่า Duty PWM จากค่าร้อยละ (0 - 100%) พร้อม Minimum Duty Clamping
uint8_t percentToDuty(int pct, uint8_t minDuty) {
  if (pct <= 0) return 0;
  pct = constrain(pct, 0, 100);
  uint8_t rawDuty = (uint8_t)map(pct, 0, 100, 0, 255);
  return max(rawDuty, minDuty);
}

// อัปเดต PWM ของพัดลมแต่ละช่องพร้อมอัลกอริทึม Kickstart & Soft-Start
void updateFanDriver(FanDriverState &fan, uint8_t pin, uint8_t channel, uint8_t targetDuty) {
  unsigned long now = millis();

  // ตรวจจับเงื่อนไข Kickstart: เริ่มต้นหมุนจากหยุดนิ่ง (Duty = 0 -> > 0)
  if (targetDuty > 0 && fan.targetDuty == 0 && fan.currentDuty == 0) {
    fan.isKicking = true;
    fan.kickStartTime = now;
    fan.currentDuty = KICKSTART_DUTY;
    writePwmDuty(pin, channel, fan.currentDuty);
    fan.targetDuty = targetDuty;
    return;
  }

  fan.targetDuty = targetDuty;

  // จัดการช่วงเวลา Kickstart
  if (fan.isKicking) {
    if (now - fan.kickStartTime < KICKSTART_MS) {
      return; // รักษาแรงดันเร่งออกตัว
    } else {
      fan.isKicking = false;
    }
  }

  // Soft-Start Slew Rate Limiter (เร่ง/ผ่อนความเร็วอย่างนุ่มนวล)
  if (fan.currentDuty < fan.targetDuty) {
    fan.currentDuty = (uint8_t)min((int)fan.targetDuty, (int)fan.currentDuty + SLEW_STEP);
  } else if (fan.currentDuty > fan.targetDuty) {
    fan.currentDuty = (uint8_t)max((int)fan.targetDuty, (int)fan.currentDuty - SLEW_STEP);
  }

  writePwmDuty(pin, channel, fan.currentDuty);
}

// ------------------------------------------------------------------------------
// ตรรกะโปรแกรมหลายเฟส (Program Sequence Runner)
// ------------------------------------------------------------------------------
uint8_t* getProgData(int id) { return id < 2 ? PRE[id] : cust; }

void applyLevel(int lvl) {
  lvl = constrain(lvl, 0, 5);
  setB = LV[lvl];
  setN = min(LV[lvl], 70); // ลมบ่าจำกัดสูงสุดไม่เกิน 70% เสมอ
  stopped = false;
}

void startPhase(int phaseIndex) {
  ph = phaseIndex;
  phStart = millis();
  uint8_t* p = getProgData(pg);
  applyLevel(p[2 * phaseIndex]);
}

void runProgram(int progId) {
  if (progId < 0 || progId > 2) return;
  uint8_t* p = getProgData(progId);
  if (p[0] > 5) return;
  pg = progId;
  runningProg = true;
  startPhase(0);
}

void tickProgram() {
  if (!runningProg) return;
  if (cut || stopped) {
    runningProg = false;
    return;
  }

  uint8_t* p = getProgData(pg);
  unsigned long durationMs = (unsigned long)p[2 * ph + 1] * (fastTest ? 6000UL : 60000UL);

  if (durationMs > 0 && (millis() - phStart >= durationMs)) {
    int nextPhase = ph + 1;
    if (nextPhase > 3 || p[2 * nextPhase] > 5) {
      runningProg = false; // จบโปรแกรม
    } else {
      startPhase(nextPhase);
    }
  }
}

// ------------------------------------------------------------------------------
// คำสั่งควบคุมระบบ (Command Dispatcher: [cmd, val, a, b])
// ------------------------------------------------------------------------------
void handleCommand(int c, int v, int a = 0, int b = 0) {
  // หากระบบตัดวงจรค้าง (Cutoff) จะรับเฉพาะคำสั่งรีเซ็ต (c = 5) เท่านั้น
  if (cut && ((c >= 1 && c <= 3) || c == 7)) return;

  // หากผู้ใช้สั่งระดับหรือหยุด ให้ยกเลิกโปรแกรมอัตโนมัติ
  if (c >= 1 && c <= 5) runningProg = false;

  switch (c) {
    case 1: // ปรับพัดลมตัว % (0 - 100)
      setB = constrain(v, 0, 100);
      stopped = false;
      break;

    case 2: // ปรับพัดลมบ่า % (จำกัดไม่เกิน 70%)
      setN = constrain(v, 0, 70);
      stopped = false;
      break;

    case 3: // เลือกระดับความแรง 0 - 5
      if (v >= 0 && v <= 5) applyLevel(v);
      break;

    case 4: // สั่งหยุดฉุกเฉิน (STOP)
      stopped = true;
      setB = 0;
      setN = 0;
      break;

    case 5: // รีเซ็ตระบบ (RESET)
      stopped = false;
      hotSim  = false;
      cut     = false;
      setB    = 0;
      setN    = 0;
      break;

    case 6: // สลับโหมดจำลองแบตร้อน (Hot Battery Test)
      hotSim = !hotSim;
      break;

    case 7: // รันโปรแกรมอัตโนมัติ (0 = Pre-cool, 1 = Ramp-up, 2 = Custom)
      runProgram(v);
      break;

    case 8: // หยุดโปรแกรมอัตโนมัติ
      runningProg = false;
      break;

    case 9: // ตั้งค่าและบันทึก Custom ลง NVS [9, phaseIndex (0-3), level, minutes]
      if (v >= 0 && v < 4) {
        cust[2 * v]     = (a > 5) ? 255 : a;
        cust[2 * v + 1] = constrain(b, 0, 255);
        pf.putBytes("cp", cust, 8);
      }
      break;

    case 12: // โหมดทดสอบเร็ว (Fast Test: 1 นาที = 6 วินาที)
      fastTest = (v != 0);
      break;
  }
}

// ------------------------------------------------------------------------------
// แบบจำลองอุณหภูมิและความปลอดภัยแบตเตอรี่ (Thermal & Safety Simulation)
// ------------------------------------------------------------------------------
void stepThermalSimulation() {
  // สุ่มความผันผวนของสภาพแวดล้อมจำลอง
  ta = constrain(ta + random(-10, 11) * 0.02f, 20.0f, 45.0f);
  rh = constrain(rh + (int)random(-1, 2), 30, 95);

  effB = setB;
  effN = setN;

  // กฎความปลอดภัย 1: แบตเตอรี่ >= 50°C ลดกำลังพัดลมเหลือ 60%
  if (tb >= 50.0f) {
    effB = (effB * 6) / 10;
    effN = (effN * 6) / 10;
  }

  // กฎความปลอดภัย 2: แบตเตอรี่ >= 60°C ตัดวงจรนิรภัยค้าง (Hardware Latch Cutoff)
  if (tb >= 60.0f) {
    cut = true;
  }

  if (cut || stopped) {
    effB = 0;
    effN = 0;
  }

  // คำนวณแนวโน้มความร้อนสะสมของแบตเตอรี่
  float targetTb = 28.0f + 0.4f * (ta - 25.0f) + 0.06f * (effB + effN) + (hotSim ? 45.0f : 0.0f) - 0.06f * batFan;
  tb += (targetTb - tb) * 0.1f;

  // กฎความปลอดภัย 3: พัดลมแบตเตอรี่ควบคุมอัตโนมัติ (เปิดเมื่อ Tb >= 40°C, Hysteresis ปิดเมื่อ < 37°C)
  if (cut) {
    batFan = 0;
  } else if (tb >= 40.0f || (batFan > 0 && tb >= 37.0f)) {
    batFan = constrain(30 + (int)((tb - 40.0f) * 5), 15, 100);
  } else {
    batFan = 0;
  }

  // จำลองการบริโภคพลังงานแบตเตอรี่ (SOC)
  soc = max(0.0f, soc - 0.002f * (1.0f + (effB + effN) / 100.0f));
}

// ------------------------------------------------------------------------------
// แพ็กเกจข้อมูล Telemetry 12 ไบต์ (Little-Endian)
// ------------------------------------------------------------------------------
void packTelemetryPacket(uint8_t* buf) {
  int16_t tempAir = (int16_t)(ta * 10.0f);
  int16_t tempBat = (int16_t)(tb * 10.0f);

  memcpy(buf, &tempAir, 2);
  buf[2] = (uint8_t)rh;
  memcpy(buf + 3, &tempBat, 2);
  buf[5] = (uint8_t)soc;
  buf[6] = (uint8_t)effB;
  buf[7] = (uint8_t)effN;
  buf[8] = (uint8_t)batFan;
  buf[9] = getTelemetryFlags();
  buf[10] = runningProg ? (uint8_t)(ph + 1) : 0;
  buf[11] = (uint8_t)pg;
}

// ------------------------------------------------------------------------------
// BLE Callbacks
// ------------------------------------------------------------------------------
class VestServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer*) { bleOn = true; }
  void onDisconnect(BLEServer*) {
    bleOn = false;
    BLEDevice::startAdvertising();
  }
};

class VestCommandCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* c) {
    size_t len = c->getLength();
    if (len >= 2) {
      uint8_t* d = c->getData();
      handleCommand(d[0], d[1], len > 2 ? d[2] : 0, len > 3 ? d[3] : 0);
    }
  }
};

// ------------------------------------------------------------------------------
// SETUP
// ------------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);

  // 1. ความปลอดภัยฮาร์ดแวร์: ดึงขา PWM ลงกราวด์ทันที ป้องกันพัดลมกระตุกตอนบูต
  pinMode(PIN_FAN_BODY, OUTPUT);
  digitalWrite(PIN_FAN_BODY, LOW);
  pinMode(PIN_FAN_NECK, OUTPUT);
  digitalWrite(PIN_FAN_NECK, LOW);
  pinMode(PIN_FAN_BAT, OUTPUT);
  digitalWrite(PIN_FAN_BAT, LOW);

  // 2. กำหนดค่า LEDC PWM 25 kHz
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

  // ปิด PWM ทุกช่อง
  writePwmDuty(PIN_FAN_BODY, 0, 0);
  writePwmDuty(PIN_FAN_NECK, 1, 0);
  writePwmDuty(PIN_FAN_BAT,  2, 0);

  // 3. เริ่มต้นไฟสถานะและตัวสร้างสุ่ม
  setSystemLed(false);
  randomSeed(esp_random());

  // 4. โหลดโปรแกรม Custom จาก NVS
  pf.begin("vest", false);
  if (pf.getBytesLength("cp") == 8) {
    pf.getBytes("cp", cust, 8);
  }

  // 5. เริ่มต้นระบบบลูทูธพลังงานต่ำ (BLE GATT)
  BLEDevice::init("SmartVest-Test");
  BLEServer* server = BLEDevice::createServer();
  server->setCallbacks(new VestServerCallbacks());

  BLEService* service = server->createService(SVC_UUID);

  BLECharacteristic* cmdChar = service->createCharacteristic(
    CMD_UUID,
    BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_WRITE_NR
  );
  cmdChar->setCallbacks(new VestCommandCallbacks());

  telChar = service->createCharacteristic(
    TEL_UUID,
    BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_READ
  );
  telChar->addDescriptor(new BLE2902());

  service->start();

  BLEAdvertising* adv = BLEDevice::getAdvertising();
  adv->addServiceUUID(SVC_UUID);
  adv->setScanResponse(true);
  BLEDevice::startAdvertising();

  // 6. Wi-Fi Access Point สำรอง & เว็บควบคุมฝังในบอร์ด
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASS);

  web.on("/", []() {
    web.send_P(200, "text/html; charset=utf-8", INDEX_HTML);
  });

  web.on("/api/state", []() {
    char json[220];
    snprintf(json, sizeof(json),
      "{\"ta\":%.1f,\"rh\":%d,\"tb\":%.1f,\"soc\":%d,\"b\":%d,\"n\":%d,\"bat\":%d,\"f\":%d,\"ph\":%d,\"pg\":%d}",
      ta, rh, tb, (int)soc, effB, effN, batFan, getTelemetryFlags(), runningProg ? (ph + 1) : 0, pg
    );
    web.send(200, "application/json", json);
  });

  web.on("/api/cmd", []() {
    handleCommand(
      web.arg("c").toInt(),
      web.arg("v").toInt(),
      web.arg("a").toInt(),
      web.arg("b").toInt()
    );
    web.send(200, "text/plain", "ok");
  });

  web.begin();

  Serial.println("\n=============================================");
  Serial.println("  SmartCool Vest ESP32-S3 Firmware v2.0 Ready");
  Serial.printf ("  PWM Frequency: %d Hz (25 kHz)\n", PWM_FREQ);
  Serial.printf ("  Fan Pins: Body=%d, Neck=%d, Bat=%d\n", PIN_FAN_BODY, PIN_FAN_NECK, PIN_FAN_BAT);
  Serial.print  ("  Wi-Fi AP IP: http://"); Serial.println(WiFi.softAPIP());
  Serial.println("  BLE Device: SmartVest-Test Advertised");
  Serial.println("=============================================\n");
}

// ------------------------------------------------------------------------------
// LOOP
// ------------------------------------------------------------------------------
void loop() {
  web.handleClient();

  // ลูปขับเคลื่อน PWM พัดลมด้วย Soft-Start & Kickstart (ทุก 20 ms)
  unsigned long now = millis();
  if (now - lastPwmTick20ms >= 20) {
    lastPwmTick20ms = now;

    // คำนวณ Target Duty จากกำลังจริงที่ผ่าน Safety Check
    uint8_t targetBody = percentToDuty(effB, MIN_DUTY_BODY);
    uint8_t targetNeck = percentToDuty(effN, MIN_DUTY_NECK);
    uint8_t targetBat  = percentToDuty(batFan, MIN_DUTY_BAT);

    updateFanDriver(fanBody, PIN_FAN_BODY, 0, targetBody);
    updateFanDriver(fanNeck, PIN_FAN_NECK, 1, targetNeck);
    updateFanDriver(fanBat,  PIN_FAN_BAT,  2, targetBat);
  }

  // ลูป Telemetry และตรรกะระบบ (ทุก 500 ms)
  if (now - lastTick500ms >= 500) {
    lastTick500ms = now;

    tickProgram();
    stepThermalSimulation();

    if (bleOn && telChar) {
      uint8_t packet[12];
      packTelemetryPacket(packet);
      telChar->setValue(packet, 12);
      telChar->notify();
    }

    setSystemLed(bleOn);
  }
}
