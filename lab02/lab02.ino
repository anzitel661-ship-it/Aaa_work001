// lab02 — LED 基础闪烁 (delay 阻塞方式)
// 板载 LED 连接 GPIO 2，高电平点亮
const int ledPin = 2;

// 时间参数
const int ON_TIME  = 1000;  // 点亮持续 1000ms
const int OFF_TIME = 1000;  // 熄灭持续 1000ms

void setup() {
  // 串口初始化，波特率 115200
  Serial.begin(115200);
  // LED 引脚设为输出模式
  pinMode(ledPin, OUTPUT);
  Serial.println("=== lab02 LED 基础闪烁 ===");
}

void loop() {
  // LED 亮
  digitalWrite(ledPin, HIGH);
  Serial.println("LED ON");
  delay(ON_TIME);  // delay() 阻塞 CPU，期间无法执行其他任务

  // LED 灭
  digitalWrite(ledPin, LOW);
  Serial.println("LED OFF");
  delay(OFF_TIME);
}