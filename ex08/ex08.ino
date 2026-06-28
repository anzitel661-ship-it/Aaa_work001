/*
 * 作业8：物联网安防报警器模拟实验
 * 功能：ESP32作为安防主机，网页端布防/撤防。布防状态下触摸引脚触发LED高频闪烁
 *       报警，报警自锁（松手不停），仅网页撤防可解除。
 * 实现要点：
 *   1. 三状态状态机：DISARMED / ARMED / ALARM
 *   2. 报警自锁：一旦触发ALARM，即使松开触摸也保持闪烁
 *   3. 网页端按钮控制布防/撤防
 *   4. 非阻塞 millis() 驱动LED高频闪烁
 */

#include <WiFi.h>
#include <WebServer.h>

// ==================== WiFi 配置 ====================
const char* ssid = "Tel";
const char* password = "tj1376125";

// ==================== 引脚定义 ====================
#define TOUCH_PIN  4      // 触摸引脚 T0 (GPIO4)
#define LED_PIN    2      // LED引脚 (ESP32 DevKit板载)

// ==================== 触摸阈值 ====================
#define THRESHOLD  400    // 触摸阈值（需根据串口读数调整）

// ==================== 报警闪烁参数 ====================
#define ALARM_BLINK_MS  120   // 报警闪烁半周期（毫秒），越小闪得越快

// ==================== 系统状态枚举 ====================
enum AlarmState {
  DISARMED = 0,   // 撤防：触摸无效
  ARMED    = 1,   // 布防：等待触发
  ALARM    = 2    // 报警：LED高频闪烁，自锁
};

AlarmState state = DISARMED;

// ==================== 报警闪烁变量（非阻塞） ====================
unsigned long lastBlinkTime = 0;
bool blinkOn = false;

// ==================== Web服务器 ====================
WebServer server(80);

// ==================== 网页HTML ====================
const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>作业8：安防报警器</title>
  <style>
    * { margin: 0; padding: 0; box-sizing: border-box; }
    body {
      font-family: 'Segoe UI', Arial, sans-serif;
      background: #0d1117;
      color: #c9d1d9;
      min-height: 100vh;
      display: flex;
      justify-content: center;
      align-items: center;
    }
    .panel {
      background: #161b22;
      border: 1px solid #30363d;
      border-radius: 16px;
      padding: 36px 28px;
      width: 90%;
      max-width: 400px;
      text-align: center;
    }
    h1 { font-size: 24px; margin-bottom: 4px; }
    .subtitle { color: #8b949e; font-size: 13px; margin-bottom: 24px; }

    /* ---- 状态指示灯 ---- */
    .status-dot {
      width: 60px; height: 60px;
      border-radius: 50%;
      margin: 0 auto 20px;
      background: #30363d;
      box-shadow: 0 0 12px rgba(0,0,0,0.5);
      transition: background 0.3s, box-shadow 0.3s;
    }
    .status-dot.disarmed { background: #238636; box-shadow: 0 0 20px rgba(35,134,54,0.6); }
    .status-dot.armed    { background: #d2991d; box-shadow: 0 0 20px rgba(210,153,29,0.6); }
    .status-dot.alarm    { background: #da3633; box-shadow: 0 0 30px rgba(218,54,51,0.8); animation: pulse 0.3s infinite; }

    @keyframes pulse {
      0%,100% { box-shadow: 0 0 20px rgba(218,54,51,0.5); }
      50%     { box-shadow: 0 0 50px rgba(218,54,51,1.0); }
    }

    .status-text {
      font-size: 20px;
      font-weight: bold;
      margin-bottom: 28px;
    }

    /* ---- 按钮 ---- */
    .btn-row { display: flex; gap: 16px; justify-content: center; flex-wrap: wrap; }
    button {
      padding: 14px 32px;
      font-size: 16px;
      font-weight: bold;
      border: none;
      border-radius: 10px;
      cursor: pointer;
      transition: transform 0.1s, opacity 0.15s;
    }
    button:active { transform: scale(0.95); }
    button:disabled { opacity: 0.35; cursor: not-allowed; }

    .btn-arm {
      background: #d2991d;
      color: #0d1117;
    }
    .btn-disarm {
      background: #238636;
      color: #fff;
    }

    /* ---- 提示 ---- */
    .hint {
      margin-top: 24px;
      font-size: 12px;
      color: #484f58;
    }
  </style>
</head>
<body>
  <div class="panel">
    <h1>安防报警器</h1>
    <p class="subtitle">IoT Security Alarm</p>

    <div id="statusDot" class="status-dot disarmed"></div>
    <div id="statusText" class="status-text">已撤防</div>

    <div class="btn-row">
      <button class="btn-arm"   onclick="sendCmd('arm')">布防</button>
      <button class="btn-disarm" onclick="sendCmd('disarm')">撤防</button>
    </div>

    <p class="hint">布防后触摸开发板引脚触发报警，撤防解除</p>
  </div>

  <script>
    function updateUI(status) {
      var dot = document.getElementById('statusDot');
      var txt = document.getElementById('statusText');
      dot.className = 'status-dot ' + status;
      var labels = { disarmed: '已撤防', armed: '已布防', alarm: '⚠ 报警中' };
      txt.textContent = labels[status] || status;
    }

    function sendCmd(cmd) {
      fetch('/' + cmd).then(function(r) { return r.text(); }).then(function(status) {
        updateUI(status);
      });
    }

    // 定期轮询当前状态（报警触发后页面自动刷新）
    function pollStatus() {
      fetch('/status').then(function(r) { return r.text(); }).then(function(status) {
        updateUI(status);
      });
    }
    setInterval(pollStatus, 1000);
  </script>
</body>
</html>
)rawliteral";

// ==================== Web路由处理 ====================

void handleRoot() {
  server.send(200, "text/html; charset=UTF-8", INDEX_HTML);
}

void handleArm() {
  if (state == DISARMED) {
    state = ARMED;
    Serial.println(">>> 已布防 (ARMED)");
  }
  server.send(200, "text/plain", stateToString());
}

void handleDisarm() {
  state = DISARMED;
  digitalWrite(LED_PIN, LOW);
  blinkOn = false;
  Serial.println(">>> 已撤防 (DISARMED)");
  server.send(200, "text/plain", stateToString());
}

void handleStatus() {
  server.send(200, "text/plain", stateToString());
}

void handleNotFound() {
  server.send(404, "text/plain", "404 Not Found");
}

// ==================== 辅助函数 ====================

const char* stateToString() {
  switch (state) {
    case DISARMED: return "disarmed";
    case ARMED:    return "armed";
    case ALARM:    return "alarm";
    default:       return "unknown";
  }
}

// ==================== 初始化 ====================
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("===== 作业8：物联网安防报警器 =====");

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // 连接WiFi
  WiFi.begin(ssid, password);
  Serial.print("连接WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n连接成功");
  Serial.print("访问地址: http://");
  Serial.println(WiFi.localIP());

  // 注册路由
  server.on("/", handleRoot);
  server.on("/arm", handleArm);
  server.on("/disarm", handleDisarm);
  server.on("/status", handleStatus);
  server.onNotFound(handleNotFound);
  server.begin();

  Serial.println("HTTP服务器已启动");
  Serial.println("初始状态: 已撤防 (DISARMED)");
}

// ==================== 主循环 ====================
void loop() {
  server.handleClient();

  // ---- 触摸检测 ----
  int touchValue = touchRead(TOUCH_PIN);
  bool touched = (touchValue < THRESHOLD);

  // 仅在布防状态下，触摸触发报警（报警自锁，松手不恢复）
  if (state == ARMED && touched) {
    state = ALARM;
    Serial.println("!!! 报警触发 (ALARM) !!!");
  }

  // ---- 报警闪烁（非阻塞） ----
  if (state == ALARM) {
    if (millis() - lastBlinkTime >= ALARM_BLINK_MS) {
      lastBlinkTime = millis();
      blinkOn = !blinkOn;
      digitalWrite(LED_PIN, blinkOn ? HIGH : LOW);
    }
  }

  // ---- 调试输出（每500ms打印一次状态和触摸值） ----
  static unsigned long lastDebugTime = 0;
  if (millis() - lastDebugTime >= 500) {
    lastDebugTime = millis();
    Serial.print("状态: ");
    Serial.print(stateToString());
    Serial.print("\t触摸值: ");
    Serial.print(touchValue);
    Serial.print(touched ? " (触摸中)" : "");
    Serial.println();
  }
}
