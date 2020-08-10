// Xiaomi Gateway v2

#define XG2_GATEWAY_DISCOVERY_PORT 4321
#define XG2_GATEWAY_MULTICAST_PORT 9898
const IPAddress xg2MulticastIp(224, 0, 0, 50);

AES xg2Aes;

char xg2Buffer[UDP_TX_PACKET_MAX_SIZE + 1];

WiFiUDP xg2UdpUnicast;
WiFiUDP xg2UdpMulticast;
WiFiUDP xg2UdpDiscovery;

DynamicJsonDocument xg2DocUnicastResp(2048);
DynamicJsonDocument xg2DocUnicastRespData(2048);
DynamicJsonDocument xg2DocMulticastResp(2048);
DynamicJsonDocument xg2DocMulticastRespData(2048);
DynamicJsonDocument xg2DocDiscoveryResp(2048);
JsonObject xg2LastUnicastResp;
JsonObject xg2LastMulticastResp;
JsonObject xg2LastDiscoveryResp;

const JsonObject xg2EmptyJsonObject;
const JsonArray xg2EmptyJsonArray;

String xg2LastGatewayToken;

void xg2Setup() {
  if (configDebugLvl > 0) {
    String key = "0987654321qwerty";
    String token = "1234567890abcdef";
    String resp = xg2Encrypt(key, token);
    String result = String("3EB43E37C20AFF4C5872CC0D04D81314");
    boolean encryptionTest = result.equals(resp);

    Serial.print(encryptionTest ? "Encryption test success " : "!!!Encryption test error!!! ");
    Serial.print(result);
    Serial.print(encryptionTest ? "==" : "<>");
    Serial.println(resp);
    Serial.print("UDP_TX_PACKET_MAX_SIZE = ");
    Serial.println(UDP_TX_PACKET_MAX_SIZE);
  }

  xg2UdpUnicast.begin(9891);
  xg2UdpMulticast.beginMulticast(WiFi.localIP(), xg2MulticastIp, XG2_GATEWAY_MULTICAST_PORT);
  xg2UdpDiscovery.beginMulticast(WiFi.localIP(), xg2MulticastIp, 9893);
}

/**
   Reads the next unicast answer. Doesn't block the loop. Returns true if the answer has been read
*/
boolean xg2NextUnicastResp() {
  xg2LastUnicastResp = xg2EmptyJsonObject;
  int packetSize = xg2UdpUnicast.parsePacket();
  if (!packetSize) {
    if (configDebugLvl > 1) {
      Serial.println("Unicast response is empty");
    }
    return false;
  }
  if (configDebugLvl > 1) {
    Serial.print("Unicast packet. Size: ");
    Serial.print(packetSize);
    Serial.print("; Remote IP: ");
    Serial.print(xg2UdpUnicast.remoteIP().toString().c_str());
    Serial.print("; Remote Port: ");
    Serial.print(xg2UdpUnicast.remotePort());
    Serial.print("; Destination IP: ");
    Serial.print(xg2UdpDiscovery.destinationIP());
    Serial.print("; Local Port: ");
    Serial.println(xg2UdpUnicast.localPort());

  }

  int len = xg2UdpUnicast.read(xg2Buffer, UDP_TX_PACKET_MAX_SIZE);
  if (len > 0) {
    xg2Buffer[len] = 0;
  }
  if (configDebugLvl > 0) {
    Serial.print("Unicast resp: ");
    Serial.println(xg2Buffer);
  }

  deserializeJson(xg2DocUnicastResp, xg2Buffer);

  xg2LastUnicastResp = xg2DocUnicastResp.as<JsonObject>();

  return true;
}

/**
   Reads the next multicast answer. Doesn't block the loop. Returns true if the answer has been read
*/
boolean xg2NextMulticastResp() {
  xg2LastMulticastResp = xg2EmptyJsonObject;

  int packetSize = xg2UdpMulticast.parsePacket();
  if (!packetSize) {
    if (configDebugLvl > 1) {
      Serial.print("Multicast response is empty ");
      Serial.println(packetSize);
    }
    return false;
  }
  if (configDebugLvl > 1) {
    Serial.print("Multicast packet. Size: ");
    Serial.print(packetSize);
    Serial.print("; Remote IP: ");
    Serial.print(xg2UdpMulticast.remoteIP().toString().c_str());
    Serial.print("; Remote Port: ");
    Serial.print(xg2UdpMulticast.remotePort());
    Serial.print("; Destination IP: ");
    Serial.print(xg2UdpDiscovery.destinationIP());
    Serial.print("; Local Port: ");
    Serial.println(xg2UdpUnicast.localPort());
  }

  int len = xg2UdpMulticast.read(xg2Buffer, UDP_TX_PACKET_MAX_SIZE);
  if (len > 0) {
    xg2Buffer[len] = 0;
  }
  if (configDebugLvl > 0) {
    Serial.print("Multicast resp: ");
    Serial.println(xg2Buffer);
  }

  deserializeJson(xg2DocMulticastResp, xg2Buffer);

  xg2LastMulticastResp = xg2DocMulticastResp.as<JsonObject>();

  // Is this a gateway heartbeat ?
  if (
    xg2LastMulticastResp.containsKey(const_str_cmd)
    && xg2LastMulticastResp.containsKey(const_str_model)
    && xg2LastMulticastResp.containsKey(const_str_token)
    && const_str_heartbeat.equals(xg2LastMulticastResp[const_str_cmd].as<String>())
    && const_str_gateway.equals(xg2LastMulticastResp[const_str_model].as<String>())
  ) {
    // This is a gateway heartbeat
    xg2LastGatewayToken = xg2LastMulticastResp[const_str_token].as<String>();
    if (configDebugLvl > 1) {
      Serial.print("New token: ");
      Serial.println(xg2LastGatewayToken);
    }
  }

  return true;
}

/**
   Reads the next discovery answer. Doesn't block the loop. Returns true if the answer has been read
*/
boolean xg2NextDiscoveryResp() {
  xg2LastDiscoveryResp = xg2EmptyJsonObject;

  int packetSize = xg2UdpDiscovery.parsePacket();
  if (!packetSize) {
    if (configDebugLvl > 1) {
      Serial.println("Multicast discovery response is empty");
    }
    return false;
  }
  if (configDebugLvl > 1) {
    Serial.print("Multicast discovery packet. Size: ");
    Serial.print(packetSize);
    Serial.print("; Remote IP: ");
    Serial.print(xg2UdpDiscovery.remoteIP().toString().c_str());
    Serial.print("; Remote Port: ");
    Serial.print(xg2UdpDiscovery.remotePort());
    Serial.print("; Destination IP: ");
    Serial.print(xg2UdpDiscovery.destinationIP());
    Serial.print("; Local Port: ");
    Serial.println(xg2UdpUnicast.localPort());
  }

  int len = xg2UdpDiscovery.read(xg2Buffer, UDP_TX_PACKET_MAX_SIZE);
  if (len > 0) {
    xg2Buffer[len] = 0;
  }
  if (configDebugLvl > 0) {
    Serial.print("Multicast discovery resp: ");
    Serial.println(xg2Buffer);
  }

  deserializeJson(xg2DocDiscoveryResp, xg2Buffer);

  xg2LastDiscoveryResp = xg2DocDiscoveryResp.as<JsonObject>();

  return true;
}


/**
   Changes the state of the device. For example, includes a wall switch or air conditioner.
   {
     "cmd":"write",
     "model":"ctrl_neutral1",
     "sid":"xxxxxxxx",
     "data":"[{\"channel_0\":\"off\",\"key\":\"3EB43E37C20AFF4C5872CC0D04D81314\"}]"
   }
*/
int xg2Write(String model, String sid, String data) {
  String req =
    String("{\"cmd\":\"write\",\"model\":\"") + model +
    String("\",\"sid\":\"") + sid +
    String("\",\"data\":\"") + data + String("\"}");
  return xg2UnicastRequest(req);
}

/**
   Unicast request. Returns more than one on success.
*/
int xg2UnicastRequest(String request) {
  if (configDebugLvl > 0) {
    Serial.print("Unicast send: ");
    Serial.println(request);
  }

  xg2UdpUnicast.beginPacket(configGatewayIp, XG2_GATEWAY_MULTICAST_PORT);
  xg2UdpUnicast.write((char*) request.c_str());
  int isEnd = xg2UdpUnicast.endPacket();

  if (configDebugLvl > 1) {
    Serial.print("End unicast packet: ");
    Serial.println(isEnd);
  }
  return isEnd;
}

/**
   Multicast request. Returns more than one on success.
*/
int xg2MulticastRequest(String request) {
  if (configDebugLvl > 0) {
    Serial.print("Multicast send: ");
    Serial.println(request);
  }
  xg2UdpMulticast.beginPacketMulticast(xg2MulticastIp, XG2_GATEWAY_MULTICAST_PORT, WiFi.localIP());
  xg2UdpMulticast.write((char*) request.c_str());
  int isEnd = xg2UdpMulticast.endPacket();

  if (configDebugLvl > 1) {
    Serial.print("End multicast packet: ");
    Serial.println(isEnd);
  }
  return isEnd;
}

/**
   Discovery request. Returns more than one on success.
*/
int xg2DiscoveryRequest(String request) {
  if (configDebugLvl > 0) {
    Serial.print("Discovery send: ");
    Serial.println(request);
  }
  xg2UdpDiscovery.beginPacketMulticast(xg2MulticastIp, XG2_GATEWAY_DISCOVERY_PORT, WiFi.localIP());
  xg2UdpDiscovery.write((char*) request.c_str());
  int isEnd = xg2UdpDiscovery.endPacket();

  if (configDebugLvl > 1) {
    Serial.print("End discovery packet: ");
    Serial.println(isEnd);
  }
  return isEnd;
}

/**
   A cmd parameter from last unicast response.
*/
String xg2UnicastCmd() {
  return xg2LastUnicastResp.containsKey(const_str_cmd) ? xg2LastUnicastResp[const_str_cmd].as<String>() : const_str_empty;
}

/**
   A sid parameter from last unicast response.
*/
String xg2UnicastSid() {
  return xg2LastUnicastResp.containsKey(const_str_sid) ? xg2LastUnicastResp[const_str_sid].as<String>() : const_str_empty;
}

/**
   A model parameter from last unicast response.
*/
String xg2UnicastModel() {
  return xg2LastUnicastResp.containsKey(const_str_model) ? xg2LastUnicastResp[const_str_model].as<String>() : const_str_empty;
}

/**
   A data parameter from last unicast response.
*/
String xg2UnicastData() {
  return xg2LastUnicastResp.containsKey(const_str_data) ? xg2LastUnicastResp[const_str_data].as<String>() : const_str_empty;
}

/**
   A short_id parameter from last unicast response.
*/
long xg2UnicastShortId() {
  return xg2LastUnicastResp.containsKey(const_str_short_id) ? xg2LastUnicastResp[const_str_short_id].as<long>() : 0L;
}

/**
   A cmd parameter from last multicast response.
*/
String xg2MulticastCmd() {
  return xg2LastMulticastResp.containsKey(const_str_cmd) ? xg2LastMulticastResp[const_str_cmd].as<String>() : const_str_empty;
}

/**
   A sid parameter from last multicast response.
*/
String xg2MulticastSid() {
  return xg2LastMulticastResp.containsKey(const_str_sid) ? xg2LastMulticastResp[const_str_sid].as<String>() : const_str_empty;
}

/**
   A model parameter from last multicast response.
*/
String xg2MulticastModel() {
  return xg2LastMulticastResp.containsKey(const_str_model) ? xg2LastMulticastResp[const_str_model].as<String>() : const_str_empty;
}

/**
   A data parameter from last multicast response.
*/
String xg2MulticastData() {
  return xg2LastMulticastResp.containsKey(const_str_data) ? xg2LastMulticastResp[const_str_data].as<String>() : const_str_empty;
}

/**
   A short_id parameter from last multicast response.
*/
long xg2MulticastShortId() {
  return xg2LastMulticastResp.containsKey(const_str_short_id) ? xg2LastMulticastResp[const_str_short_id].as<long>() : 0L;
}

/**
   A ip parameter from last discovery response.
*/
String xg2GatewayToken() {
  return xg2LastGatewayToken;
}

/**
   A port parameter from last discovery response.
*/
String xg2DiscoveryIp() {
  return xg2LastDiscoveryResp.containsKey(const_str_ip) ? xg2LastDiscoveryResp[const_str_ip].as<String>() : const_str_empty;
}

/**
   A sid parameter from last discovery response.
*/
int xg2DiscoveryPort() {
  return xg2LastDiscoveryResp.containsKey(const_str_port) ? xg2LastDiscoveryResp[const_str_port].as<int>() : 0;
}

/**
   A cmd parameter from last unicast response.
*/
String xg2DiscoverySid() {
  return xg2LastDiscoveryResp.containsKey(const_str_sid) ? xg2LastDiscoveryResp[const_str_sid].as<String>() : const_str_empty;
}

/**
   Last unicast response as JsonObject.
*/
JsonObject xg2UnicastResp() {
  return xg2LastUnicastResp;
}

/**
   Last data from unicast response as JsonObject.
*/
JsonObject xg2UnicastDataAsJsonObject() {
  String data = xg2UnicastData();
  if (const_str_empty.equals(data)) {
    return xg2EmptyJsonObject;
  }
  deserializeJson(xg2DocUnicastRespData, data);
  return xg2DocUnicastRespData.as<JsonObject>();
}


/**
   Last data from unicast response as JsonArray.
*/
JsonArray xg2UnicastDataAsJsonArray() {
  String data = xg2UnicastData();
  if (const_str_empty.equals(data)) {
    return xg2EmptyJsonArray;
  }
  deserializeJson(xg2DocUnicastRespData, data);
  return xg2DocUnicastRespData.as<JsonArray>();
}

/**
   Last multicast response as JsonObject.
*/
JsonObject xg2MulticastResp() {
  return xg2LastMulticastResp;
}


/**
   Last data from multicast response as JsonObject.
*/
JsonObject xg2MulticastDataAsJsonObject() {
  String data = xg2MulticastData();
  if (const_str_empty.equals(data)) {
    return xg2EmptyJsonObject;
  }
  deserializeJson(xg2DocMulticastRespData, data);
  return xg2DocMulticastRespData.as<JsonObject>();
}

/**
   Last data from multicast response as JsonArray.
*/
JsonArray xg2MulticastDataAsJsonArray() {
  String data = xg2MulticastData();
  if (const_str_empty.equals(data)) {
    return xg2EmptyJsonArray;
  }
  deserializeJson(xg2DocMulticastRespData, data);
  return xg2DocMulticastRespData.as<JsonArray>();
}

/**
   Last discovery response as JsonObject.
*/
JsonObject xg2DiscoveryResp() {
  return xg2LastDiscoveryResp;
}

/**
   Generates a key for xg2Write using the token from last heartbeat multicast response and the password from config.
*/
String xg2Key() {
  return xg2Encrypt(configGatewayPassword, xg2LastGatewayToken);
}

/**
   Generates a key for xg2Write using  key and token parameters.
*/
String xg2Encrypt(String key, String token) {
  if (key.length() != 16 || token.length() != 16) {
    return const_str_empty;
  }

  byte iv[16] = {0x17, 0x99, 0x6d, 0x09, 0x3d, 0x28, 0xdd, 0xb3, 0xba, 0x69, 0x5a, 0x2e, 0x6f, 0x58, 0x56, 0x2e};
  byte keyBytes[17];
  byte tokenBytes[17];
  byte respBytes[16];

  key.getBytes(keyBytes, 17);
  token.getBytes(tokenBytes, 17);

  xg2Aes.do_aes_encrypt(tokenBytes, 16, respBytes, keyBytes, 16, iv);

  unsigned int len = xg2Aes.get_size();
  if (len != 16) {
    return const_str_empty;
  }

  return xg2ArrayToString(respBytes, len);
}

String xg2ArrayToString(byte in[], unsigned int len) {
  char out[len * 2 + 1];
  for (unsigned int i = 0; i < len; i++) {
    byte nib1 = (in[i] >> 4) & 0x0F;
    byte nib2 = (in[i] >> 0) & 0x0F;
    out[i * 2 + 0] = nib1  < 0xA ? '0' + nib1  : 'A' + nib1  - 0xA;
    out[i * 2 + 1] = nib2  < 0xA ? '0' + nib2  : 'A' + nib2  - 0xA;
  }
  out[len * 2] = '\0';
  return String(out);
}
