/*
 * 作业7：Web网页端无极调光器
 * 功能：手机/电脑浏览器打开ESP32网页，拖动滑动条实时调节LED亮度。
 * 实现要点：
 *   1. WiFi + WebServer 提供HTTP服务
 *   2. HTML滑动条 <input type="range" min="0" max="255">
 *   3. JavaScript fetch() 实时将滑动条数值发送给ESP32
 *   4. ESP32解析URL参数并写入PWM占空比（ledcWrite）
 */

#include <WiFi.h>
#include <WebServer.h>

// ==================== WiFi 配置 ====================
const char* ap_ssid = "esp32ex204";

// ==================== 引脚与PWM配置 ====================
#define LED_PIN     2      // LED引脚 (ESP32 DevKit板载)
#define PWM_FREQ        5000
#define PWM_RESOLUTION  8      // 8位分辨率 (0~255)

// ==================== Web服务器 ====================
WebServer server(80);

// ==================== 网页HTML ====================
// 使用 rawliteral 避免字符串转义问题
const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>作业7：无极调光器</title>
  <style>
    * { margin: 0; padding: 0; box-sizing: border-box; }
    body {
      font-family: 'Segoe UI', Arial, sans-serif;
      background: linear-gradient(135deg, #1a1a2e 0%, #16213e 100%);
      color: #eee;
      min-height: 100vh;
      display: flex;
      justify-content: center;
      align-items: center;
    }
    .container {
      background: rgba(255,255,255,0.05);
      border-radius: 20px;
      padding: 40px 30px;
      width: 90%;
      max-width: 420px;
      box-shadow: 0 8px 32px rgba(0,0,0,0.4);
      backdrop-filter: blur(10px);
    }
    h1 {
      font-size: 28px;
      margin-bottom: 6px;
    }
    .subtitle {
      color: #888;
      font-size: 14px;
      margin-bottom: 30px;
    }
    /* ---- 亮度图标 ---- */
    .icon-row {
      display: flex;
      justify-content: space-between;
      align-items: center;
      margin-bottom: 10px;
      font-size: 14px;
      color: #aaa;
    }
    .icon-row span:first-child { font-size: 20px; }
    .icon-row span:last-child  { font-size: 26px; }
    /* ---- 滑动条 ---- */
    input[type=range] {
      -webkit-appearance: none;
      width: 100%;
      height: 12px;
      border-radius: 6px;
      background: linear-gradient(to right, #0f0, #ff0, #f00);
      outline: none;
      cursor: pointer;
    }
    input[type=range]::-webkit-slider-thumb {
      -webkit-appearance: none;
      width: 36px;
      height: 36px;
      border-radius: 50%;
      background: #fff;
      border: 3px solid #ff9800;
      box-shadow: 0 0 18px rgba(255,152,0,0.6);
      cursor: pointer;
      transition: box-shadow 0.15s;
    }
    input[type=range]::-webkit-slider-thumb:active {
      box-shadow: 0 0 30px rgba(255,152,0,0.9);
    }
    /* ---- 数值显示 ---- */
    .value-display {
      text-align: center;
      margin-top: 24px;
      font-size: 48px;
      font-weight: bold;
      color: #ff9800;
      text-shadow: 0 0 20px rgba(255,152,0,0.5);
    }
    .value-label {
      text-align: center;
      font-size: 13px;
      color: #666;
    }
  </style>
</head>
<body>
  <div class="container">
    <h1>无极调光器</h1>
    <p class="subtitle">拖动滑动条调节LED亮度</p>

    <div class="icon-row">
      <span>&#9788;</span>
      <span>&#9728;</span>
    </div>
    <input type="range" min="0" max="255" value="0" id="slider"
           oninput="updateSlider(this.value)">

    <div class="value-display"><span id="val">0</span></div>
    <div class="value-label">PWM 占空比 / 255</div>
  </div>

  <script>
    function updateSlider(val) {
      document.getElementById('val').textContent = val;
      fetch('/set?value=' + val).catch(function(e) {
        console.log('发送失败: ' + e);
      });
    }
  </script>
</body>
</html>
)rawliteral";

// ==================== 处理函数 ====================

// 首页 —— 返回HTML页面
void handleRoot() {
  server.send(200, "text/html; charset=UTF-8", INDEX_HTML);
}

// 设置亮度 —— 解析URL参数并写入PWM
void handleSet() {
  if (server.hasArg("value")) {
    int duty = server.arg("value").toInt();
    duty = constrain(duty, 0, 255);           // 限幅保护
    ledcWrite(LED_PIN, duty);
    server.send(200, "text/plain", "OK");
    Serial.print("滑动条数值: ");
    Serial.println(duty);
  } else {
    server.send(400, "text/plain", "Missing 'value' parameter");
  }
}

// 404 处理
void handleNotFound() {
  server.send(404, "text/plain", "404 Not Found");
}

// ==================== 初始化 ====================
void setup() {
  Serial.begin(115200);
  delay(1000);

  // 初始化PWM
  ledcAttach(LED_PIN, PWM_FREQ, PWM_RESOLUTION);
  ledcWrite(LED_PIN, 0);

  // ESP32 自建开放 WiFi AP
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ap_ssid);

  Serial.println("AP 已启动");
  Serial.print("AP 名称: ");
  Serial.println(ap_ssid);
  Serial.print("访问地址: http://");
  Serial.println(WiFi.softAPIP());

  // 注册路由
  server.on("/", handleRoot);
  server.on("/set", handleSet);
  server.onNotFound(handleNotFound);
  server.begin();

  Serial.println("HTTP服务器已启动");
  Serial.println("在浏览器中打开上方地址，拖动滑动条即可调光");
}

// ==================== 主循环 ====================
void loop() {
  server.handleClient();
}
