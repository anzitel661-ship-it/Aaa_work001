# ex02 — millis() 非阻塞 LED 闪烁

## 实验目的

对比 `delay()` **阻塞式** 与 `millis()` **非阻塞式** 两种 LED 控制方式。

---

## 频率说明

| 参数 | 值 |
|------|-----|
| 间隔 | 500ms 亮 / 500ms 灭 |
| 频率 | **1 Hz**（每秒 1 个完整周期） |
| 占空比 | 50% |

---

## delay() vs millis() 对比

| 维度 | `delay()`（lab02） | `millis()`（ex02） |
|------|-------------------|-------------------|
| **工作原理** | 调用 `delay(1000)` 让 CPU 原地死等 1 秒 | 每次 `loop()` 检查时间戳差值是否达到 500ms |
| **CPU 利用率** | ❌ 等待期间 CPU 100% 空转，无法做任何事 | ✅ CPU 可同时处理其他任务（读传感器、通信等） |
| **多任务** | ❌ 无法实现 | ✅ 支持多任务并发 |
| **响应速度** | ❌ 按键/传感器在 `delay()` 期间无响应 | ✅ 实时响应外部事件 |
| **代码复杂度** | ★☆☆☆☆ 简单 | ★★☆☆☆ 稍复杂 |
| **适用场景** | 入门学习、极简 demo | **实际项目、多任务系统** |

### 代码差异

```cpp
// ========== delay() 阻塞方式 (lab02) ==========
void loop() {
  digitalWrite(ledPin, HIGH);   // 亮
  delay(1000);                  // ← CPU 卡死 1 秒，什么也干不了
  digitalWrite(ledPin, LOW);    // 灭
  delay(1000);                  // ← CPU 又卡死 1 秒
}

// ========== millis() 非阻塞方式 (ex02) ==========
void loop() {
  unsigned long currentMillis = millis();         // 获取当前时间

  if (currentMillis - previousMillis >= 500) {    // 到了 500ms 吗？
    previousMillis = currentMillis;               // 更新基准时间
    ledState = !ledState;                         // 翻转 LED
    digitalWrite(ledPin, ledState);
  }
  // ↓ 程序继续执行，可以在此添加其他任务
  // readSensor();
  // checkButton();
  // sendData();
}
```

### 时序图

```
delay() 方式：
  LED ████████________████████________  （1s 亮 / 1s 灭 = 0.5Hz）
  任务  [卡死1s][卡死1s][卡死1s]...       ← 无法做其他事

millis() 方式：
  LED ████____████____████____        （500ms 亮 / 500ms 灭 = 1Hz）
  任务  [执行][执行][执行][执行]...       ← 始终可响应其他事件
```

---

## millis() 溢出安全性

`millis()` 返回 `unsigned long` 类型（32 位），最大值约 49.7 天。到达最大值后会回绕到 0（溢出）。由于使用了 **无符号整数减法**：

```cpp
if (currentMillis - previousMillis >= INTERVAL)
```

即使发生溢出，减法结果依然正确。例如：
- `previousMillis = 4294967295`（接近溢出）
- `currentMillis = 500`（溢出后）
- `500 - 4294967295` 在 32 位无符号运算下 = `501` ✅ 正确

**结论：只要间隔 < 49.7 天，此写法永远正确。**

---

## 运行方式

1. Arduino IDE 打开 `ex02.ino`
2. 选择开发板：ESP32 Dev Module
3. 烧录并打开串口监视器（115200 波特率）
4. 观察 LED 以 1Hz 频率闪烁，串口同步打印状态
