/*
 * 作业9：实时传感器Web仪表盘
 * 功能：网页端实时显示触摸传感器的模拟量数值，手靠近数值变小，离开数值恢复。
 *       实现数据采集（上报）而非仅控制（下发），展示IoT双向通信。
 * 实现要点：
 *   1. ESP32通过 /data 接口返回JSON格式的触摸传感器数值
 *   2. 网页端使用 AJAX 定时轮询（setInterval + fetch），实时更新显示
 *   3. 仪表盘风格UI：大字数值 + 动态条形图 + 状态指示
 */

#include <WiFi.h>
#include <WebServer.h>

// ==================== WiFi 配置 ====================
const char* ssid = "Tel";
const char* password = "tj1376125";

// ==================== 引脚定义 ====================
#define TOUCH_PIN  4      // 触摸引脚 T0 (GPIO4)

// ==================== Web服务器 ====================
WebServer server(80);

// ==================== 网页HTML ====================
const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>作业9：传感器仪表盘</title>
  <style>
    * { margin: 0; padding: 0; box-sizing: border-box; }
    body {
      font-family: 'Segoe UI', 'Consolas', monospace;
      background: #0b0e14;
      color: #e0e0e0;
      min-height: 100vh;
      display: flex;
      justify-content: center;
      align-items: center;
    }
    .dashboard {
      background: #12161d;
      border: 1px solid #2a3040;
      border-radius: 20px;
      padding: 36px 30px;
      width: 92%;
      max-width: 440px;
      text-align: center;
    }
    h1 {
      font-size: 22px;
      font-weight: 600;
      letter-spacing: 2px;
      color: #8b949e;
      text-transform: uppercase;
      margin-bottom: 6px;
    }
    .subtitle {
      font-size: 12px;
      color: #484f58;
      margin-bottom: 28px;
    }

    /* ---- 主数值 ---- */
    .reading {
      font-size: 96px;
      font-weight: bold;
      color: #58a6ff;
      text-shadow: 0 0 40px rgba(88,166,255,0.4);
      line-height: 1;
      transition: color 0.3s;
    }
    .reading.near  { color: #f78166; text-shadow: 0 0 40px rgba(247,129,102,0.5); }
    .reading.touch { color: #da3633; text-shadow: 0 0 50px rgba(218,54,51,0.7); }
    .unit {
      font-size: 16px;
      color: #484f58;
      margin-bottom: 24px;
    }

    /* ---- 条形指示条 ---- */
    .bar-container {
      background: #1c2333;
      border-radius: 8px;
      height: 18px;
      overflow: hidden;
      margin-bottom: 12px;
      position: relative;
    }
    .bar-fill {
      height: 100%;
      border-radius: 8px;
      background: linear-gradient(90deg, #da3633, #d2991d, #238636);
      transition: width 0.25s ease-out;
    }

    /* ---- 刻度标签 ---- */
    .scale-labels {
      display: flex;
      justify-content: space-between;
      font-size: 11px;
      color: #484f58;
      margin-bottom: 24px;
    }

    /* ---- 状态标签 ---- */
    .status-badge {
      display: inline-block;
      padding: 6px 20px;
      border-radius: 20px;
      font-size: 14px;
      font-weight: bold;
      letter-spacing: 1px;
      background: #1c2333;
      color: #8b949e;
      transition: all 0.3s;
    }
    .status-badge.far    { background: #1c3a2a; color: #3fb950; }
    .status-badge.near   { background: #3a2e1c; color: #d2991d; }
    .status-badge.touch  { background: #3a1c1c; color: #da3633; }

    /* ---- 页脚 ---- */
    .footer {
      margin-top: 24px;
      font-size: 11px;
      color: #30363d;
    }
  </style>
</head>
<body>
  <div class="dashboard">
    <h1>Sensor Dashboard</h1>
    <p class="subtitle">触摸传感器实时监控 — T0 (GPIO4)</p>

    <div id="reading" class="reading">--</div>
    <div class="unit">touchRead() 原始值</div>

    <div class="bar-container">
      <div id="barFill" class="bar-fill" style="width:50%;"></div>
    </div>
    <div class="scale-labels">
      <span>触摸 (低)</span><span>空闲 (高)</span>
    </div>

    <div id="statusBadge" class="status-badge">等待数据...</div>

    <p class="footer">刷新间隔 200ms &nbsp;|&nbsp; 作业9</p>
  </div>

  <script>
    var READING = document.getElementById('reading');
    var BAR = document.getElementById('barFill');
    var BADGE = document.getElementById('statusBadge');

    // 假定传感器范围: 触摸≈0~20, 靠近≈20~35, 空闲≈35~80
    // 实际范围根据串口监视器调整
    var MAX_VAL = 80;   // 空闲上限
    var MIN_VAL = 0;    // 触摸下限
    var NEAR_THRESHOLD = 35;  // 靠近阈值

    function updateDashboard(val) {
      READING.textContent = val;

      // 条形图 (值越小越靠近左边 = 触摸)
      var pct = Math.max(0, Math.min(100, (val - MIN_VAL) / (MAX_VAL - MIN_VAL) * 100));
      BAR.style.width = pct.toFixed(1) + '%';

      // 状态分类
      READING.className = 'reading';
      BADGE.className = 'status-badge';
      if (val < 20) {
        READING.classList.add('touch');
        BADGE.classList.add('touch');
        BADGE.textContent = '已触摸';
      } else if (val < NEAR_THRESHOLD) {
        READING.classList.add('near');
        BADGE.classList.add('near');
        BADGE.textContent = '靠近中';
      } else {
        BADGE.classList.add('far');
        BADGE.textContent = '空闲';
      }
    }

    function fetchData() {
      fetch('/data')
        .then(function(r) { return r.json(); })
        .then(function(json) {
          updateDashboard(json.value);
        })
        .catch(function(e) {
          READING.textContent = '---';
          BADGE.textContent = '连接断开';
        });
    }

    // 立即拉取一次，之后每200ms轮询
    fetchData();
    setInterval(fetchData, 200);
  </script>
</body>
</html>
)rawliteral";

// ==================== Web路由处理 ====================

void handleRoot() {
  server.send(200, "text/html; charset=UTF-8", INDEX_HTML);
}

// 返回触摸传感器数值（JSON格式）
void handleData() {
  int touchValue = touchRead(TOUCH_PIN);
  String json = "{\"value\":" + String(touchValue) + "}";
  server.send(200, "application/json", json);
}

void handleNotFound() {
  server.send(404, "text/plain", "404 Not Found");
}

// ==================== 初始化 ====================
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("===== 作业9：实时传感器Web仪表盘 =====");

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
  server.on("/data", handleData);
  server.onNotFound(handleNotFound);
  server.begin();

  Serial.println("HTTP服务器已启动");
  Serial.println("在浏览器中打开上方地址查看实时仪表盘");
}

// ==================== 主循环 ====================
void loop() {
  server.handleClient();
}
