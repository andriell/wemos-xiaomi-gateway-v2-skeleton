WiFiUDP timeUdp;

char *timeServerList[] = {
  "0.pool.ntp.org",
  "1.pool.ntp.org",
  "2.pool.ntp.org",
  "3.pool.ntp.org",
  "us.pool.ntp.org",
  "time-a.timefreq.bldrdoc.gov",
  "time-b.timefreq.bldrdoc.gov",
  "time-c.timefreq.bldrdoc.gov"
};
int timeServerListLenght = 8;


void timeSetup() {
  timeUdp.begin(8888);
  timeUpdate();
}

boolean timeUpdate() {
  for (int i = 0; i < timeServerListLenght; i++) {
    time_t t = timeNtpTime(timeServerList[i]);
    if (t > 0) {
      setTime(t);
      if (configDebugLvl > 0) {
        Serial.print("Time: ");
        Serial.println(timeString());
      }
      return true;
    }
    delay(100);
  }
  return false;
}

String timeString() {
  static char str[19];
  sprintf(str, "%02d:%02d:%02d %02d-%02d-%04d", hour(), minute(), second(), day(), month(), year());
  return String(str);
}


/*-------- NTP code ----------*/

const int TIME_NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
byte timePacketBuffer[TIME_NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets

time_t timeNtpTime(char* serverName) {
  IPAddress ntpServerIP; // NTP server's ip address

  while (timeUdp.parsePacket() > 0) ; // discard any previously received packets
  if (configDebugLvl > 0) {
    Serial.println("Transmit NTP Request");
  }
  // get a random server from the pool
  WiFi.hostByName(serverName, ntpServerIP);
  if (configDebugLvl > 0) {
    Serial.print(serverName);
    Serial.print(": ");
    Serial.println(ntpServerIP);
  }
  timeSendNTPpacket(ntpServerIP);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = timeUdp.parsePacket();
    if (size >= TIME_NTP_PACKET_SIZE) {
      if (configDebugLvl > 0) {
        Serial.println("Receive NTP Response");
      }
      timeUdp.read(timePacketBuffer, TIME_NTP_PACKET_SIZE);  // read packet into the buffer
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)timePacketBuffer[40] << 24;
      secsSince1900 |= (unsigned long)timePacketBuffer[41] << 16;
      secsSince1900 |= (unsigned long)timePacketBuffer[42] << 8;
      secsSince1900 |= (unsigned long)timePacketBuffer[43];
      return secsSince1900 - 2208988800UL + configTimeZone * SECS_PER_HOUR;
    }
  }
  if (configDebugLvl > 0) {
    Serial.println("No NTP Response :-(");
  }
  return 0; // return 0 if unable to get the time
}

// send an NTP request to the time server at the given address
void timeSendNTPpacket(IPAddress &address) {
  // set all bytes in the buffer to 0
  memset(timePacketBuffer, 0, TIME_NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  timePacketBuffer[0] = 0b11100011;   // LI, Version, Mode
  timePacketBuffer[1] = 0;     // Stratum, or type of clock
  timePacketBuffer[2] = 6;     // Polling Interval
  timePacketBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  timePacketBuffer[12] = 49;
  timePacketBuffer[13] = 0x4E;
  timePacketBuffer[14] = 49;
  timePacketBuffer[15] = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  timeUdp.beginPacket(address, 123); //NTP requests are to port 123
  timeUdp.write(timePacketBuffer, TIME_NTP_PACKET_SIZE);
  timeUdp.endPacket();
}
