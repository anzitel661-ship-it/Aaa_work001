/*
 * 作业6：警车双闪灯效（双通道PWM）
 * 功能：两个LED呈现平滑交替的渐变闪烁效果——灯A变亮时灯B变暗，反之亦然。
 *       模拟警车双闪的柔和过渡版，两个灯光此消彼长，过渡非常柔和。
 * 实现要点：
 *   1. 两个独立PWM通道（ledcAttach / ledcWrite 新版API）
 *   2. "反相"关系：灯A占空比 = duty，灯B占空比 = 255 - duty
 *   3. 平滑渐变，delay 控制过渡速度
 */

// ==================== 引脚定义 ====================
// 使用两个独立GPIO，面包板上各接一个LED（串联220Ω限流电阻）
#define LED_A_PIN  16     // 灯A (红灯)
#define LED_B_PIN  17     // 灯B (蓝灯)

// ==================== PWM 属性 ====================
#define PWM_FREQ        5000   // PWM频率 5000Hz
#define PWM_RESOLUTION  8      // 分辨率 8位 (0~255)

// ==================== 过渡速度 ====================
#define FADE_DELAY_MS  10      // 每步延迟（毫秒），越小越平滑

// ==================== 初始化 ====================
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("===== 作业6：警车双闪灯效 =====");
  Serial.println("灯A (GPIO16) 与 灯B (GPIO17) 反相渐变闪烁");
  Serial.println();

  // 初始化两个独立PWM通道
  ledcAttach(LED_A_PIN, PWM_FREQ, PWM_RESOLUTION);
  ledcAttach(LED_B_PIN, PWM_FREQ, PWM_RESOLUTION);

  // 初始状态：灯A熄灭，灯B全亮
  ledcWrite(LED_A_PIN, 0);
  ledcWrite(LED_B_PIN, 255);
}

// ==================== 主循环 ====================
void loop() {
  // 阶段1：灯A逐渐变亮 (0 → 255)，灯B逐渐变暗 (255 → 0)
  for (int duty = 0; duty <= 255; duty++) {
    ledcWrite(LED_A_PIN, duty);         // 灯A 变亮
    ledcWrite(LED_B_PIN, 255 - duty);   // 灯B 变暗（反相）
    delay(FADE_DELAY_MS);
  }

  // 阶段2：灯A逐渐变暗 (255 → 0)，灯B逐渐变亮 (0 → 255)
  for (int duty = 255; duty >= 0; duty--) {
    ledcWrite(LED_A_PIN, duty);         // 灯A 变暗
    ledcWrite(LED_B_PIN, 255 - duty);   // 灯B 变亮（反相）
    delay(FADE_DELAY_MS);
  }

  Serial.println("双闪周期完成");
}
