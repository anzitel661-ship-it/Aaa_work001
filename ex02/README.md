# ex02 — millis() 非阻塞 LED 闪烁

## 实验目的

对比 `delay()` 阻塞式 与 `millis()` 非阻塞式 两种 LED 控制方式。

---

## 频率说明

| 参数 | 值 |
|------|-----|
| 间隔 | 500ms 亮 / 500ms 灭 |
| 频率 | 1 Hz（每秒 1 个完整周期） |
| 占空比 | 50% |

---

## delay() vs millis() 对比

| 维度 | delay()（lab02） | millis()（ex02） |
|------|-----------------|-----------------|
| 工作原理 | 调用 `delay(1000)` 让 CPU 原地死等 1 秒 | 每次 `loop()` 检查时间戳差值是否达到 500ms |
| CPU 利用率 | 等待期间 CPU 空转，无法做任何事 | CPU 可同时处理其他任务 |
| 多任务 | 无法实现 | 支持多任务并发 |
| 响应速度 | 按键/传感器在 `delay()` 期间无响应 | 实时响应外部事件 |
| 代码复杂度 | 简单 | 稍复杂 |
| 适用场景 | 入门学习、极简 demo | 实际项目、多任务系统 |

### 代码对比

```cpp
// delay() 阻塞方式 (lab02)
void loop() {
  digitalWrite(ledPin, HIGH);
  delay(1000);                  // CPU 在此卡死 1 秒
  digitalWrite(ledPin, LOW);
  delay(1000);                  // CPU 再次卡死 1 秒
}

// millis() 非阻塞方式 (ex02)
void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= 500) {
    previousMillis = currentMillis;
    ledState = !ledState;
    digitalWrite(ledPin, ledState);
  }
  // 此处可继续执行其他任务，不会阻塞
}
```

### 时序对比

```
delay():
  LED     [====亮1s====][____灭1s____][====亮1s====]...
  CPU     卡死............卡死............卡死..........

millis():
  LED     [=亮0.5s=][_灭0.5s_][=亮0.5s=][_灭0.5s_]...
  CPU     执行.执行.执行.执行.执行.执行.执行.执行.执行.
```

---

## millis() 溢出安全性

`millis()` 返回 32 位无符号整数，约 49.7 天后溢出回绕到 0。

使用无符号减法判断间隔：

```cpp
if (currentMillis - previousMillis >= INTERVAL)
```

即使溢出，减法结果依然正确：

- `previousMillis` = 4294967295（接近溢出上限）
- `currentMillis` = 500（溢出后）
- `500 - 4294967295` = 501（32 位无符号运算，结果正确）

只要间隔小于 49.7 天，此写法永远安全。

---

## 运行方式

1. Arduino IDE 打开 `ex02.ino`
2. 选择开发板：ESP32 Dev Module
3. 烧录并打开串口监视器（115200 波特率）
4. 观察 LED 以 1Hz 频率闪烁，串口同步打印状态
