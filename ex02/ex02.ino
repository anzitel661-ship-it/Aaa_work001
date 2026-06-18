// 使用 millis() 函数控制LED以1Hz频率稳定闪烁
// 非阻塞方式，避免使用 delay() — ESP32 板载 LED (GPIO 2)

const int ledPin = 2;               // ESP32板载LED引脚
const unsigned long INTERVAL = 500; // 闪烁间隔 500ms → 1Hz (1秒一个完整周期)

int ledState = LOW;                       // LED当前状态
unsigned long previousMillis = 0;         // 上一次切换LED的时间戳

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  Serial.println("=== millis() 非阻塞 1Hz LED 闪烁 ===");
  Serial.print("间隔: ");
  Serial.print(INTERVAL);
  Serial.println(" ms (频率: 1 Hz)");
}

void loop() {
  // 获取当前系统运行毫秒数
  unsigned long currentMillis = millis();

  // 判断是否达到切换间隔
  if (currentMillis - previousMillis >= INTERVAL) {
    previousMillis = currentMillis;  // 保存本次切换时间戳

    // 翻转LED状态
    ledState = (ledState == LOW) ? HIGH : LOW;
    digitalWrite(ledPin, ledState);

    // 串口输出当前状态
    if (ledState == HIGH) {
      Serial.println("LED ON  (500ms亮)");
    } else {
      Serial.println("LED OFF (500ms灭)");
    }
  }

  // 此处可以执行其他任务，不会被阻塞
  // 例如：读取传感器、处理通信等
}
