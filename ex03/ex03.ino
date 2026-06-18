// 使用 millis() 函数控制LED产生SOS求救信号
// 非阻塞方式 — ESP32 板载 LED (GPIO 2)

const int ledPin = 2;

// ---- SOS 时序步骤表 ----
// 每步定义 { LED状态, 持续时间(ms) }
struct Step {
  int state;            // HIGH 或 LOW
  unsigned long duration;
};

const Step SOS_SEQUENCE[] = {
  // --- S: 短闪 ×3 (每个 200ms亮 / 200ms灭) ---
  { HIGH, 200 }, { LOW, 200 },
  { HIGH, 200 }, { LOW, 200 },
  { HIGH, 200 }, { LOW, 200 },
  // 字母间隔补足 (已含上次200ms灭, 再补300ms → 总计500ms)
  { LOW,  300 },

  // --- O: 长闪 ×3 (每个 600ms亮 / 200ms灭) ---
  { HIGH, 600 }, { LOW, 200 },
  { HIGH, 600 }, { LOW, 200 },
  { HIGH, 600 }, { LOW, 200 },
  // 字母间隔补足
  { LOW,  300 },

  // --- S: 短闪 ×3 ---
  { HIGH, 200 }, { LOW, 200 },
  { HIGH, 200 }, { LOW, 200 },
  { HIGH, 200 }, { LOW, 200 },
  // 单词间隔补足 (已含上次200ms灭, 再补1800ms → 总计2000ms)
  { LOW, 1800 },
};

const int STEP_COUNT = sizeof(SOS_SEQUENCE) / sizeof(Step);

int currentStep = 0;                  // 当前步骤索引
unsigned long stepStartMillis = 0;    // 当前步骤开始时刻

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  Serial.println("=== millis() 非阻塞 SOS 求救信号 ===");
  Serial.print("总步骤数: ");
  Serial.println(STEP_COUNT);

  // 启动第一步
  digitalWrite(ledPin, SOS_SEQUENCE[0].state);
  stepStartMillis = millis();
  printStep(0);
}

void loop() {
  unsigned long now = millis();

  // 检查当前步骤是否已到时
  if (now - stepStartMillis >= SOS_SEQUENCE[currentStep].duration) {
    // 推进到下一步
    currentStep++;
    if (currentStep >= STEP_COUNT) {
      currentStep = 0;  // 循环播放
    }

    // 应用新步骤
    int state = SOS_SEQUENCE[currentStep].state;
    digitalWrite(ledPin, state);
    stepStartMillis = now;
    printStep(currentStep);
  }

  // 此处可执行其他任务, LED 闪烁不会被阻塞
}

// 串口输出当前步骤信息
void printStep(int step) {
  int state = SOS_SEQUENCE[step].state;
  unsigned long dur = SOS_SEQUENCE[step].duration;

  Serial.print("步骤 ");
  Serial.print(step);
  Serial.print("/");
  Serial.print(STEP_COUNT - 1);
  Serial.print("  LED ");
  Serial.print(state == HIGH ? "ON " : "OFF");
  Serial.print("  持续 ");
  Serial.print(dur);
  Serial.println(" ms");
}
