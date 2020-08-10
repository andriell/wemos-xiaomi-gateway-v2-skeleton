DynamicJsonDocument exampleDoc(1024);

#define EXAMPLE_PERIOD 60

boolean examplePlugIsOn = false;
int exampleLoopDiscoveryCount = EXAMPLE_PERIOD;
int exampleLoopListCount = EXAMPLE_PERIOD - 5;
int exampleLoopPlugCount = EXAMPLE_PERIOD - 10;

void exampleLoopSendCommand() {
  exampleLoopDiscoveryCount++;
  exampleLoopListCount++;
  exampleLoopPlugCount++;

  String cmd;

  if (exampleLoopDiscoveryCount > EXAMPLE_PERIOD) {
    exampleLoopDiscoveryCount = 0;

    cmd = "{\"cmd\":\"whois\"}";
    Serial.print(timeString());
    Serial.print(" Multicast discovery send: ");
    Serial.println(cmd);
    xg2DiscoveryRequest(cmd);
  }

  if (exampleLoopListCount > EXAMPLE_PERIOD) {
    exampleLoopListCount = 0;

    cmd = "{\"cmd\": \"get_id_list\"}";
    Serial.print(timeString());
    Serial.print(" Unicast send: ");
    Serial.println(cmd);
    xg2UnicastRequest(cmd);
  }

  if (exampleLoopPlugCount > EXAMPLE_PERIOD) {
    // Switch a plug
    exampleLoopPlugCount = 0;
    Serial.print(timeString());
    if (configExamplePlugSid.length() < 10) {
      Serial.println(" configExamplePlugSid is empty");
      return;
    }
    Serial.print(" Switches the plug using a token: ");
    Serial.println(xg2GatewayToken());

    examplePlugIsOn = !examplePlugIsOn;
    examplePlugIsOn ? ledOn() : ledOff();
    cmd = String("[{\\\"status\\\":\\\"") + (examplePlugIsOn ? "on" : "off") + String("\\\",\\\"key\\\":\\\"") + xg2Key() + String("\\\"}]");
    xg2Write("plug", configExamplePlugSid, cmd);
  }
}


void exampleUnicastLoop() {
  while (xg2NextUnicastResp()) {
    Serial.print(timeString());
    Serial.print(" Unicast resp. cmd: ");
    Serial.print(xg2UnicastCmd());
    Serial.print("; model: ");
    Serial.print(xg2UnicastModel());
    Serial.print("; sid: ");
    Serial.print(xg2UnicastSid());
    Serial.print("; short_id: ");
    Serial.print(xg2UnicastShortId());
    Serial.print("; data: ");
    Serial.println(xg2UnicastData());

    // {"cmd":"get_id_list_ack","sid":"12345678901234","token":"12345678901234","data":"[\"12345678901234\",\"12345678901234\"]"}
    if (xg2UnicastCmd().equals("get_id_list_ack")) {
      JsonArray array = xg2UnicastDataAsJsonArray();
      Serial.print("Devices found: ");
      Serial.println(array.size());

      int i = 0;
      for (JsonVariant v : array) {
        Serial.print(timeString());
        Serial.print(" Device ");
        Serial.print(++i);
        String cmd = String("{\"cmd\":\"read\",\"sid\":\"") + v.as<String>() + String("\"}");
        xg2UnicastRequest(cmd);

        int limit = 200;
        while (!xg2NextUnicastResp()) {
          if (--limit <= 0) {
            Serial.println("No answer");
            break;
          }
          delay(10);
        }
        Serial.print("; model: ");
        Serial.print(xg2UnicastModel());
        Serial.print("; sid: ");
        Serial.print(xg2UnicastSid());
        Serial.print("; short_id: ");
        Serial.print(xg2UnicastShortId());
        Serial.print("; data: ");
        Serial.println(xg2UnicastData());
      }
    }
  }
}

void exampleMulticastLoop() {
  while (xg2NextMulticastResp()) {
    Serial.print(timeString());
    Serial.print(" Multicast resp. cmd: ");
    Serial.print(xg2MulticastCmd());
    Serial.print("; model: ");
    Serial.print(xg2MulticastModel());
    Serial.print("; sid: ");
    Serial.print(xg2MulticastSid());
    Serial.print("; short_id: ");
    Serial.print(xg2MulticastShortId());
    Serial.print("; data: ");
    Serial.println(xg2MulticastData());
  }
}

void exampleDiscoveryLoop() {
  while (xg2NextDiscoveryResp()) {
    // Resp: {"cmd":"iam","port":"9898","sid":"12345678901234","model":"gateway","proto_version":"1.1.2","ip":"192.168.1.1"}
    JsonObject resp = xg2DiscoveryResp();
    Serial.print(timeString());
    Serial.print(" Discovery resp. ip: ");
    Serial.print(xg2DiscoveryIp());
    Serial.print("; port: ");
    Serial.print(xg2DiscoveryPort());
    Serial.print("; sid: ");
    Serial.print(xg2DiscoverySid());
    Serial.print("; proto_version: ");
    Serial.println(resp["proto_version"].as<String>());
  }
}
