// lab03 — PWM LED 呼吸灯
// 板载 LED 连接 GPIO 2，通过 PWM 控制亮度

const int ledPin = 2;      // LED 引脚

// PWM 参数配置
const int freq = 5000;     // PWM 频率 5000Hz (高于人眼闪烁感知)
const int resolution = 8;  // 分辨率 8 位 → 占空比范围 0~255

// 呼吸节奏参数
const int STEP_DELAY = 10;   // 每步延时 10ms，控制呼吸速度
const int DUTY_MAX   = 255;  // 最大占空比 (2^8 - 1)

void setup() {
  Serial.begin(115200);
  // 将 LED 引脚绑定到 PWM 通道（自动分配），设置频率和分辨率
  ledcAttach(ledPin, freq, resolution);
  Serial.println("=== lab03 PWM 呼吸灯 ===");
}

void loop() {
  // 渐亮阶段：占空比从 0 递增到 255
  for (int dutyCycle = 0; dutyCycle <= DUTY_MAX; dutyCycle++) {
    ledcWrite(ledPin, dutyCycle);  // 更新 PWM 占空比
    delay(STEP_DELAY);
  }

  // 渐暗阶段：占空比从 255 递减到 0
  for (int dutyCycle = DUTY_MAX; dutyCycle >= 0; dutyCycle--) {
    ledcWrite(ledPin, dutyCycle);
    delay(STEP_DELAY);
  }

  Serial.println("Breathing cycle completed");  // 每个呼吸周期输出一次
}