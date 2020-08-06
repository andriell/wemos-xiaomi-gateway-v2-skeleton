// Дожидается подключения к wi-fi
// Выводит лог на экран
void wifiConnection() {
  if (configDebugLvl > 0) {
    Serial.println(String("Connecting to "));
    Serial.println(String(configWifiSsid));
  }

  while (WiFi.status() != WL_CONNECTED) {
    ledBlink(1, 250);
    if (configDebugLvl > 0) {
      Serial.println(String("Connecting to Wi-Fi"));
    }
  }
  if (configDebugLvl > 0) {
    Serial.println(String("Wi-Fi connected"));
    Serial.println(String("IP address: "));

    Serial.println(wifiGetIpString());
  }
  ledBlink(3, 500);
}

void wifiSetup() {
  WiFi.hostname("Temperature monitor");
  WiFi.mode(WIFI_STA);
  WiFi.begin(configWifiSsid, configWifiPassword);

  wifiConnection();

  WiFi.setAutoReconnect(true);
}

String wifiGetIpString()
{
  const IPAddress ipAddress = WiFi.localIP();
  return String(ipAddress[0]) + String(".") + \
         String(ipAddress[1]) + String(".") + \
         String(ipAddress[2]) + String(".") + \
         String(ipAddress[3]);
}


void wifiLoop()
{
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.reconnect();
    wifiConnection();
  }
}
