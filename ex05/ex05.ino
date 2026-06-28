/*
 * 作业5：多档位触摸调速呼吸灯
 * 功能：LED持续呈呼吸灯效果。每触摸一次引脚，"呼吸"节奏在三个档位间循环切换。
 * 实现要点：
 *   1. PWM呼吸灯（ledcAttach / ledcWrite，新版API）
 *   2. 边缘检测 + 软件防抖（非阻塞，不影响呼吸节奏）
 *   3. 档位循环切换（1→2→3→1），每档呼吸速度明显不同
 *   4. 非阻塞 millis() 驱动，触摸响应即时，呼吸平滑
 */

// ==================== 引脚与硬件配置 ====================
#define TOUCH_PIN   4      // 触摸引脚 T0 (GPIO4)
#define LED_PIN     2      // LED引脚 (ESP32 DevKit板载)

// ==================== PWM 属性 ====================
#define PWM_FREQ        5000   // PWM频率 5000Hz
#define PWM_RESOLUTION  8      // 分辨率 8位 (0~255)

// ==================== 阈值配置 ====================
#define THRESHOLD  400         // 触摸阈值（需根据串口读数调整）

// ==================== 防抖配置 ====================
#define DEBOUNCE_COOLDOWN_MS  400   // 档位切换后的冷却时间（防手抖）

// ==================== 档位对应的呼吸速度 ====================
// 每个档位由 (delayMs, stepSize) 决定呼吸节奏
// delayMs: 每步间隔时间（越小越快）
// stepSize: 每步占空比变化量（越大越"跳"，视觉上更急促）
struct SpeedGear {
  int delayMs;    // 每步延迟（毫秒）
  int stepSize;   // 占空比步长
};

const SpeedGear GEARS[] = {
  {  0, 0    },  // 占位，从索引1开始
  { 10, 1    },  // 档位1：缓慢呼吸（10ms/步，细腻平滑）
  {  5, 2    },  // 档位2：中速呼吸（5ms/步，适中节奏）
  {  2, 4    },  // 档位3：急促呼吸（2ms/步，快速起伏）
};

#define GEAR_COUNT  3   // 档位总数

// ==================== 全局状态变量 ====================
int speedGear = 1;              // 当前档位 (1/2/3)
bool lastTouchState = false;    // 上一次触摸状态（边缘检测用）
unsigned long lastGearChange = 0;  // 上次换档时刻（防抖冷却用）

// PWM 呼吸状态（非阻塞）
unsigned long lastPwmTime = 0;  // 上次更新PWM的时刻
int dutyCycle = 0;              // 当前占空比 (0~255)
bool rising = true;             // 呼吸方向（true=变亮，false=变暗）

// ==================== 初始化 ====================
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("===== 作业5：多档位触摸调速呼吸灯 =====");
  Serial.println("档位1 → 缓慢呼吸");
  Serial.println("档位2 → 中速呼吸");
  Serial.println("档位3 → 急促呼吸");
  Serial.println("触摸T0引脚切换档位");
  Serial.println();

  // 新版API：直接绑定引脚、频率和分辨率
  ledcAttach(LED_PIN, PWM_FREQ, PWM_RESOLUTION);
  ledcWrite(LED_PIN, 0);  // 初始熄灭

  printGearInfo();
}

// ==================== 主循环 ====================
void loop() {
  // ---------- 触摸检测 + 档位切换 ----------
  int touchValue = touchRead(TOUCH_PIN);
  bool currentTouch = (touchValue < THRESHOLD);

  // 边缘检测：上次未触摸 → 当前触摸（下降沿）
  // 同时检查冷却时间，防止手抖误触发连续换档
  if (!lastTouchState && currentTouch && (millis() - lastGearChange > DEBOUNCE_COOLDOWN_MS)) {
    // 有效触摸 → 档位循环切换 (1→2→3→1)
    speedGear = (speedGear % GEAR_COUNT) + 1;
    lastGearChange = millis();

    printGearInfo();
  }

  lastTouchState = currentTouch;

  // ---------- PWM 呼吸灯（非阻塞） ----------
  SpeedGear gear = GEARS[speedGear];

  if (millis() - lastPwmTime >= (unsigned long)gear.delayMs) {
    lastPwmTime = millis();

    // 写入当前占空比
    ledcWrite(LED_PIN, dutyCycle);

    // 根据档位步长更新占空比
    if (rising) {
      dutyCycle += gear.stepSize;
      if (dutyCycle >= 255) {
        dutyCycle = 255;
        rising = false;
        Serial.print(".");
      }
    } else {
      dutyCycle -= gear.stepSize;
      if (dutyCycle <= 0) {
        dutyCycle = 0;
        rising = true;
        Serial.print(".");
      }
    }
  }
}

// ==================== 辅助函数 ====================
void printGearInfo() {
  SpeedGear gear = GEARS[speedGear];
  const char* labels[] = {"", "缓慢", "中速", "急促"};

  Serial.print(">>> 切换到档位 ");
  Serial.print(speedGear);
  Serial.print(" (");
  Serial.print(labels[speedGear]);
  Serial.print("): delay=");
  Serial.print(gear.delayMs);
  Serial.print("ms, step=");
  Serial.print(gear.stepSize);
  Serial.println();
}
