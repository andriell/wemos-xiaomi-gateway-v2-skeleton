DynamicJsonDocument exampleDoc(1024);

boolean exampleIsOn = false;

// Discover the gateway device
void exampleLoop1() {
  String cmd = "{\"cmd\":\"whois\"}";
  Serial.print("Multicast discovery send: ");
  Serial.println(cmd);
  xg2DiscoveryRequest(cmd);

  Serial.print("Waiting for an answer");
  while (!xg2NextDiscoveryResp()) {
    Serial.println(".");
    delay(100);
  }
  Serial.println();

  // Resp: {"cmd":"iam","port":"9898","sid":"12345678901234","model":"gateway","proto_version":"1.1.2","ip":"192.168.1.1"}
  JsonObject resp = xg2DiscoveryResp();
  Serial.print("Gateway. ip: ");
  Serial.print(xg2DiscoveryIp());
  Serial.print("; port: ");
  Serial.print(xg2DiscoveryPort());
  Serial.print("; sid: ");
  Serial.print(xg2DiscoverySid());
  Serial.print("; proto_version: ");
  Serial.println(resp["proto_version"].as<String>());
}

// Get of sub device SID and device info
void exampleLoop2() {
  String cmd = "{\"cmd\": \"get_id_list\"}";

  Serial.print("Multicast send: ");
  Serial.println(cmd);
  xg2MulticastRequest(cmd);

  Serial.print("Waiting for an answer");
  while (!xg2NextMulticastResp()) {
    Serial.println(".");
    delay(100);
  }
  Serial.println();

  // {"cmd":"get_id_list_ack","sid":"12345678901234","token":"12345678901234","data":"[\"12345678901234\",\"12345678901234\",\"12345678901234\"]"}
  JsonObject resp = xg2MulticastResp();
  Serial.print("Device SIDs: ");
  Serial.print(xg2MulticastData());


  JsonArray array = xg2MulticastDataAsJsonArray();
  for (JsonVariant v : array) {
    cmd = String("{\"cmd\":\"read\",\"sid\":\"") + v.as<String>() + String("\"}");
    xg2UnicastRequest(cmd);
    while (!xg2NextUnicastResp()) {
      delay(100);
    }
    Serial.print("Device. model: ");
    Serial.print(xg2UnicastModel());
    Serial.print("; sid: ");
    Serial.print(xg2UnicastSid());
    Serial.print("; short_id: ");
    Serial.print(xg2UnicastShortId());
    Serial.print("; data: ");
    Serial.println(xg2UnicastData());
  }
}

// Query sub-device list
void exampleLoop3() {
  if (const_str_.equals(configExamplePlugSid)) {
    return;
  }
  // We are waiting for the gateway to report the heartbeat
  // This should be done only if the program code begins with xg2Write
  // Heartbeat is updated automatically when called xg2NextMulticastResp
  while (const_str_.equals(xg2GatewayToken())) {
    xg2NextMulticastResp();
  }
  while (xg2NextMulticastResp()) {}

  String data = String("[{\\\"status\\\":\\\"") + (exampleIsOn ? "on" : "off") + String("\\\",\\\"key\\\":\\\"") + xg2Key() + String("\\\"}]");
  xg2Write("plug", configExamplePlugSid, data);
  exampleIsOn = !exampleIsOn;
}

// Listening MulticastResp
void exampleLoop4() {
  while (xg2NextMulticastResp()) {
    Serial.print("Multicast resp. cmd: ");
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
