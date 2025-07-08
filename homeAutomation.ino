#include <WiFi.h>
#include <WebServer.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

// WiFi credentials
const char* ssid = "SSID";
const char* password = "PASSWORD";

// HiveMQ Cloud credentials
const char* mqtt_server = "MQTT_Broker";
const int mqtt_port = 8883;
const char* mqtt_user = "USER";
const char* mqtt_pass = "Password";

// MQTT topics
const char* cooler_topic = "home/cooler";
const char* fan_topic = "home/fan";
const char* light_topic = "home/light";
const char* heater_topic = "home/heater";

// GPIO pin definitions
const int coolerPin = 32;  // Relay 1
const int fanPin = 27;     // Relay 2
const int lightPin = 26;   // Relay 3
const int heaterPin = 25;  // Relay 4


// Appliance states
bool coolerState = false;
bool fanState = false;
bool lightState = false;
bool heaterState = false;


WiFiClientSecure espClient;
PubSubClient client(espClient);
WebServer server(80);

// Send current status as JSON
void sendStatus() {
  String json = "{";
  json += "\"cooler\":" + String(coolerState ? "true" : "false") + ",";
  json += "\"fan\":" + String(fanState ? "true" : "false") + ",";
  json += "\"light\":" + String(lightState ? "true" : "false") + ",";
  json += "\"heater\":" + String(heaterState ? "true" : "false");
  json += "}";
  server.send(200, "application/json", json);
}

// HTML interface
void handleRoot() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Smart Home Control Panel</title>
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <style>
    body {
      margin: 0;
      font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
      background-color: #f0f2f5;
      color: #333;
      text-align: center;
    }
    header {
      background-color: #0078D7;
      color: white;
      padding: 20px 0;
      font-size: 24px;
      font-weight: bold;
      box-shadow: 0 2px 5px rgba(0,0,0,0.2);
    }
    .container {
      display: flex;
      flex-wrap: wrap;
      justify-content: center;
      padding: 30px;
    }
    .device-card {
      background: white;
      border-radius: 12px;
      box-shadow: 0 4px 12px rgba(0,0,0,0.1);
      padding: 20px;
      margin: 15px;
      width: 200px;
      transition: transform 0.2s;
    }
    .device-card:hover {
      transform: translateY(-5px);
    }
    .device-name {
      font-size: 20px;
      margin-bottom: 10px;
    }
    .status {
      font-weight: bold;
      margin-bottom: 10px;
    }
    .on { color: green; }
    .off { color: red; }
    button {
      padding: 10px 20px;
      border: none;
      border-radius: 8px;
      background-color: #0078D7;
      color: white;
      font-size: 16px;
      cursor: pointer;
    }
    button:hover {
      background-color: #005ea6;
    }
  </style>
</head>
<body>
  <header>🏠 Smart Home Control Panel</header>
  <div class="container">
    <div class="device-card">
      <div class="device-name">Cooler</div>
      <div class="status" id="cooler_state">...</div>
      <button onclick="toggle('cooler')">Toggle</button>
    </div>
    <div class="device-card">
      <div class="device-name">Fan</div>
      <div class="status" id="fan_state">...</div>
      <button onclick="toggle('fan')">Toggle</button>
    </div>
    <div class="device-card">
      <div class="device-name">Light</div>
      <div class="status" id="light_state">...</div>
      <button onclick="toggle('light')">Toggle</button>
    </div>
    <div class="device-card">
      <div class="device-name">Heater</div>
      <div class="status" id="heater_state">...</div>
      <button onclick="toggle('heater')">Toggle</button>
    </div>
  </div>

<script>
function toggle(device) {
  fetch('/toggle/' + device).then(() => updateStates());
}

function updateStates() {
  fetch('/status')
    .then(response => response.json())
    .then(data => {
      document.getElementById('cooler_state').innerText = data.cooler ? 'ON' : 'OFF';
      document.getElementById('cooler_state').className = data.cooler ? 'status on' : 'status off';

      document.getElementById('fan_state').innerText = data.fan ? 'ON' : 'OFF';
      document.getElementById('fan_state').className = data.fan ? 'status on' : 'status off';

      document.getElementById('light_state').innerText = data.light ? 'ON' : 'OFF';
      document.getElementById('light_state').className = data.light ? 'status on' : 'status off';

      document.getElementById('heater_state').innerText = data.heater ? 'ON' : 'OFF';
      document.getElementById('heater_state').className = data.heater ? 'status on' : 'status off';
    });
}

setInterval(updateStates, 2000);
window.onload = updateStates;
</script>
</body>
</html>
  )rawliteral";
  server.send(200, "text/html", html);
}


// Toggle helper
void toggleAppliance(bool& state, int pin, const char* topic) {
  state = !state;
  digitalWrite(pin, state ? HIGH : LOW);
  client.publish(topic, state ? "ON" : "OFF", true);
  server.send(200, "text/plain", "OK");
}

// Connect to WiFi
void setupWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected! IP address: " + WiFi.localIP().toString());
}

// Handle MQTT messages
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String msg;
  for (unsigned int i = 0; i < length; i++) msg += (char)payload[i];

  if (String(topic) == cooler_topic) {
    coolerState = (msg == "ON");
    digitalWrite(coolerPin, coolerState ? LOW : HIGH);
  } else if (String(topic) == fan_topic) {
    fanState = (msg == "ON");
    digitalWrite(fanPin, fanState ? LOW : HIGH);
  } else if (String(topic) == light_topic) {
    lightState = (msg == "ON");
    digitalWrite(lightPin, lightState ? LOW : HIGH);
  } else if (String(topic) == heater_topic) {
    heaterState = (msg == "ON");
    digitalWrite(heaterPin, heaterState ? LOW : HIGH);
  }

  Serial.printf("MQTT: [%s] %s\n", topic, msg.c_str());
}

// Reconnect to MQTT broker
void reconnectMQTT() {
  while (!client.connected()) {
    Serial.print("Connecting to MQTT...");
    if (client.connect("ESP32_SmartHome", mqtt_user, mqtt_pass)) {
      Serial.println("connected.");
      client.subscribe(cooler_topic);
      client.subscribe(fan_topic);
      client.subscribe(light_topic);
      client.subscribe(heater_topic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" retrying in 5s");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(coolerPin, OUTPUT);
  pinMode(fanPin, OUTPUT);
  pinMode(lightPin, OUTPUT);
  pinMode(heaterPin, OUTPUT);

  // Set all relays to OFF initially (HIGH = OFF for active LOW relays)
  digitalWrite(coolerPin, HIGH);
  digitalWrite(fanPin, HIGH);
  digitalWrite(lightPin, HIGH);
  digitalWrite(heaterPin, HIGH);

  setupWiFi();
  configTime(0, 0, "pool.ntp.org");
  espClient.setInsecure();  // Don't verify server cert (for dev only)

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(mqttCallback);

  // Setup web server routes
  server.on("/", handleRoot);
  server.on("/status", sendStatus);
  server.on("/toggle/cooler", []() {
    toggleAppliance(coolerState, coolerPin, cooler_topic);
  });
  server.on("/toggle/fan", []() {
    toggleAppliance(fanState, fanPin, fan_topic);
  });
  server.on("/toggle/light", []() {
    toggleAppliance(lightState, lightPin, light_topic);
  });
  server.on("/toggle/heater", []() {
    toggleAppliance(heaterState, heaterPin, heater_topic);
  });

  server.begin();
  Serial.println("Web server started.");
}

void loop() {
  if (!client.connected()) reconnectMQTT();
  client.loop();
  server.handleClient();
}
