// SOS求救信号模式 — 基于 lab02 LED 闪烁
// 摩尔斯电码：S = ··· (三个短闪)，O = −−− (三个长闪)
// 使用 ESP32 板载 LED (GPIO 2)

const int ledPin = 2;  // ESP32板载LED引脚

// 时间参数 (毫秒)
const int SHORT_ON  = 200;   // 短闪点亮时长 (S)
const int LONG_ON   = 600;   // 长闪点亮时长 (O)
const int GAP_OFF   = 200;   // 每次熄灭间隔
const int LETTER_GAP = 500;  // 字母间额外间隔
const int WORD_GAP   = 2000; // 单词间额外间隔 (SOS 信号重复间隔)

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  Serial.println("=== SOS 求救信号模式启动 ===");
}

// ---- 闪烁辅助函数 ----
// times: 闪烁次数
// onTime: 点亮时长(ms)
void flash(int times, int onTime, const char* label) {
  for (int i = 0; i < times; i++) {
    digitalWrite(ledPin, HIGH);
    Serial.printf("  [%s] 第%d闪 ON  (%dms)\n", label, i + 1, onTime);
    delay(onTime);

    digitalWrite(ledPin, LOW);
    Serial.printf("  [%s] 第%d闪 OFF (%dms)\n", label, i + 1, GAP_OFF);
    delay(GAP_OFF);
  }
}

void loop() {
  // ===== S: 三个短闪 =====
  Serial.println(">> S (短闪 ×3)");
  flash(3, SHORT_ON, "S");
  delay(LETTER_GAP - GAP_OFF);  // 补足字母间隔 (已含最后一次 GAP_OFF)

  // ===== O: 三个长闪 =====
  Serial.println(">> O (长闪 ×3)");
  flash(3, LONG_ON, "O");
  delay(LETTER_GAP - GAP_OFF);

  // ===== S: 三个短闪 =====
  Serial.println(">> S (短闪 ×3)");
  flash(3, SHORT_ON, "S");
  delay(LETTER_GAP - GAP_OFF);

  // ===== 单词结束，长间隔后重复 =====
  Serial.println("=== SOS 信号发送完毕，等待重复... ===\n");
  delay(WORD_GAP - GAP_OFF);
}
