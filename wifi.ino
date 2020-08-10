// Дожидается подключения к wi-fi
// Выводит лог на экран
void wifiConnection() {
  dbg(1, "Connecting to ");
  dbgLn(1, configWifiSsid);

  while (WiFi.status() != WL_CONNECTED) {
    ledBlink(1, 250);
    dbgLn(1, "Connecting to Wi-Fi");
  }
  dbgLn(1, "Wi-Fi connected");
  dbg(1, "IP address: ");
  dbgLn(1, wifiGetIpString());
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
