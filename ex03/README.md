# ex03 — millis() 非阻塞 SOS 求救信号

## 实验目的

在非阻塞框架下实现 SOS 摩尔斯电码闪烁，与 lab02_sos 的 `delay()` 实现形成对比。

---

## SOS 信号规范

| 字母 | 摩尔斯码 | LED 表现 |
|------|---------|---------|
| S | ··· | 短闪 3 次 (200ms 亮 / 200ms 灭) |
| O | --- | 长闪 3 次 (600ms 亮 / 200ms 灭) |
| S | ··· | 短闪 3 次 (200ms 亮 / 200ms 灭) |

字母间隔 500ms，单词间隔 2000ms。

---

## 设计方案：步骤表 + millis()

与 lab02_sos 每个字母写一个 for 循环不同，ex03 将整个 SOS 序列展开为一张**步骤表**：

```cpp
struct Step {
  int state;               // HIGH 或 LOW
  unsigned long duration;  // 该状态持续毫秒数
};

const Step SOS_SEQUENCE[] = {
  // S: 短闪 x3
  { HIGH, 200 }, { LOW, 200 },
  { HIGH, 200 }, { LOW, 200 },
  { HIGH, 200 }, { LOW, 200 },
  { LOW,  300 },              // 字母间隔补足 (200+300=500ms)

  // O: 长闪 x3
  { HIGH, 600 }, { LOW, 200 },
  { HIGH, 600 }, { LOW, 200 },
  { HIGH, 600 }, { LOW, 200 },
  { LOW,  300 },              // 字母间隔补足

  // S: 短闪 x3
  { HIGH, 200 }, { LOW, 200 },
  { HIGH, 200 }, { LOW, 200 },
  { HIGH, 200 }, { LOW, 200 },
  { LOW, 1800 },              // 单词间隔补足 (200+1800=2000ms)
};
```

每次 `loop()` 用 `millis()` 检查当前步骤是否到时，到时则推进到下一步：

```cpp
if (now - stepStartMillis >= SOS_SEQUENCE[currentStep].duration) {
  currentStep++;                                      // 下一步
  digitalWrite(ledPin, SOS_SEQUENCE[currentStep].state);
  stepStartMillis = now;
}
```

---

## 间隔补足说明

每个字母末尾已有一个 200ms 的 `LOW`（熄灭），所以后续间隔只需补差额：

```
S 最后 200ms 灭 + 字母间隔补 300ms = 共 500ms 字母间隔
O 最后 200ms 灭 + 字母间隔补 300ms = 共 500ms 字母间隔
S 最后 200ms 灭 + 单词间隔补1800ms = 共2000ms 单词间隔
```

---

## delay() vs millis() 实现 SOS 对比

| | lab02_sos (delay) | ex03 (millis) |
|------|------------------|---------------|
| 实现方式 | `flash()` 函数内 for 循环 + `delay()` | 步骤表数组 + `millis()` 时间戳 |
| 阻塞 | 每条 SOS 全程阻塞约 6 秒 | 全程非阻塞 |
| 播放期间能否响应按键 | 不能 | 能 |
| 可扩展性 | 改时序需改代码逻辑 | 改时序只改步骤表数据 |
| 适用场景 | 入门理解 SOS 概念 | 实际项目中嵌入 SOS 功能 |

---

## 运行方式

1. Arduino IDE 打开 `ex03.ino`
2. 选择开发板：ESP32 Dev Module
3. 烧录并打开串口监视器（115200 波特率）
4. 观察 LED 循环播放 SOS，串口同步打印步骤号、LED 状态和持续时间
