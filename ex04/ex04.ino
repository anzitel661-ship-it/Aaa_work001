/*
 * 作业4：基于触摸传感器的"自锁"开关
 * 功能：摸一下触摸引脚，LED亮起并保持长亮；再摸一下，LED熄灭。
 * 实现要点：
 *   1. 布尔型状态变量 ledState 记录LED当前状态
 *   2. 边缘检测：检测"上次未触摸 → 本次触摸"的下降沿瞬间
 *   3. 软件防抖：连续多次确认触摸状态，过滤手抖误触发
 *   4. 仅在有效触摸边沿翻转LED状态
 */

// ==================== 引脚定义 ====================
#define TOUCH_PIN  4    // 触摸引脚 T0 (GPIO4)
#define LED_PIN    2    // LED引脚 (ESP32 DevKit板载LED)

// ==================== 阈值配置 ====================
// 实际使用时请通过串口监视器观察 touchValue 并调整此阈值
// 未触摸时读数通常 > 40，触摸时读数下降至 20 以下
#define THRESHOLD  400

// ==================== 防抖配置 ====================
#define DEBOUNCE_DELAY_MS  50   // 防抖间隔（毫秒）
#define DEBOUNCE_COUNT      3   // 连续确认次数（须连续3次检测到触摸才算有效）

// ==================== 全局状态变量 ====================
bool ledState = false;           // LED当前状态（true=亮，false=灭）
bool lastTouchState = false;     // 上一次读取的触摸状态（true=未触摸，false=被触摸）
                                 // 注意：touchRead()返回值越小表示触摸越强

void setup() {
  Serial.begin(115200);
  delay(1000); // 等待串口监控器稳定连接
  Serial.println("===== 作业4：触摸自锁开关 =====");
  Serial.println("摸一下 → LED长亮；再摸一下 → LED熄灭");
  Serial.println("请通过串口监视器观察触摸值，调整THRESHOLD阈值");
  Serial.println();

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, ledState ? HIGH : LOW);

  Serial.print("当前阈值: ");
  Serial.println(THRESHOLD);
}

void loop() {
  // 读取触摸值（值越小 → 触摸越强）
  int touchValue = touchRead(TOUCH_PIN);

  // 根据阈值判断当前是否被触摸
  bool currentTouchState = (touchValue < THRESHOLD);

  // 打印触摸值，方便调试和调整阈值
  Serial.print("Touch Value: ");
  Serial.print(touchValue);
  Serial.print("\t| State: ");
  Serial.print(currentTouchState ? "TOUCHED" : "released");
  Serial.print("\t| LED: ");
  Serial.println(ledState ? "ON" : "OFF");

  // ========== 边缘检测 + 防抖逻辑 ==========
  // 检测下降沿：上一次未触摸(false) → 当前被触摸(true)
  if (!lastTouchState && currentTouchState) {
    // 检测到触摸边沿，进入软件防抖确认
    if (debounceConfirm()) {
      // 防抖通过 → 翻转LED状态
      ledState = !ledState;
      digitalWrite(LED_PIN, ledState ? HIGH : LOW);

      Serial.print(">>> 有效触摸！LED 已");
      Serial.println(ledState ? "点亮" : "熄灭");
    }
  }

  // 更新上一次触摸状态，供下一轮边缘检测使用
  lastTouchState = currentTouchState;

  delay(30); // 主循环间隔约30ms，兼顾响应速度与CPU占用
}

/*
 * 软件防抖函数
 * 连续多次采样确认触摸状态，防止手抖或瞬时干扰导致误触发
 * 返回 true 表示确认为有效触摸，false 表示可能是抖动
 */
bool debounceConfirm() {
  int confirmCount = 0;

  for (int i = 0; i < DEBOUNCE_COUNT; i++) {
    delay(DEBOUNCE_DELAY_MS); // 等待一段时间再采样
    int val = touchRead(TOUCH_PIN);
    if (val < THRESHOLD) {
      confirmCount++; // 本次采样确认触摸
    }
  }

  // 所有采样点都确认触摸，才算有效
  bool confirmed = (confirmCount >= DEBOUNCE_COUNT);

  if (!confirmed) {
    Serial.println("    [防抖] 触摸未通过确认，已过滤");
  }

  return confirmed;
}
