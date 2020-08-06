# Wemos Xiaomi Gateway V2 Skeleton


This program works with Xiaomi Gateway V2 in developer mode. 
This is the basis for working with a smart home using Wemos or other ESP8266 microcontrollers.
With this program you can control all devices working via the ZigBee protocol. 
Examples are in the examples.ino file.

## How to flash
After downloading this source, rename config.ino.example to config.ino and write your options there.

## Methods

* **boolean xg2NextUnicastResp()** - Reads the next unicast answer. Doesn't block the loop. Returns true if the answer has been read
    
Response example:

    {
        "cmd":"read_ack",
        "model":"sensor_magnet.aq2",
        "sid":"xxxxxxxx",
        "short_id":12345,
        "data":"{\"voltage\":3025,\"status\":\"open\"}"
    }

* **boolean xg2NextMulticastResp()** - Reads the next multicast answer. Doesn't block the loop. Returns true if the answer has been read. If the response was heartbeat, automatically update token for xg2Key() and xg2GatewayToken() methods.  

Response example:

    {
        "cmd":"heartbeat",
        "model":"gateway",
        "sid":"xxxxxxxx",
        "short_id":"0",
        "token":"xxxxxxxx",
        "data":"{\"ip\":\"192.168.1.1\"}"
    }

    
* **boolean xg2NextDiscoveryResp()** - Reads the next discovery answer. Doesn't block the loop. Returns true if the answer has been read
    
Response example:

    {
        "cmd":"iam",
        "port":"9898",
        "sid":"xxxxxxxx",
        "model":"gateway",
        "proto_version":"1.1.2",
        "ip":"192.168.1.1"
    }

* **int xg2Write(String model, String sid, String data)** - Changes the state of the device. For example, it turns on a wall switch or air conditioner.

Request example:

    {
        "cmd":"write",
        "model":"ctrl_neutral1",
        "sid":"xxxxxxxx",
        "data":"[{\"channel_0\":\"off\","key":"3EB43E37C20AFF4C5872CC0D04D81314",}]"
    }
    
* **int xg2UnicastRequest(String request)** - Unicast request. Returns more than one on success.
* **int xg2MulticastRequest(String request)** - Multicast request. Returns more than one on success.
* **int xg2DiscoveryRequest(String request)** - Discovery request. Returns more than one on success.

* **String xg2Key()** - Generates a key for xg2Write using the token from last heartbeat multicast response and the password from config.
* **String xg2Encrypt(String key, String token)** - Generates a key for xg2Write using key and token parameters.

### Getters

* **String xg2UnicastCmd()**   - A cmd parameter from last unicast response.
* **String xg2UnicastSid()**   - A sid parameter from last unicast response.
* **String xg2UnicastModel()** - A model parameter from last unicast response.
* **String xg2UnicastData()**  - A data parameter from last unicast response.
* **long xg2UnicastShortId()** - A short_id parameter from last unicast response.

* **String xg2MulticastCmd()**   - A cmd parameter from last multicast response.
* **String xg2MulticastSid()**   - A sid parameter from last multicast response.
* **String xg2MulticastModel()** - A model parameter from last multicast response.
* **String xg2MulticastData()**  - A data parameter from last multicast response.
* **long xg2MulticastShortId()** - A short_id parameter from last multicast response.
* **String xg2GatewayToken()**   - A cmd parameter from last unicast response.

* **String xg2DiscoveryIp()**  - A ip parameter from last discovery response.
* **int xg2DiscoveryPort()**   - A port parameter from last discovery response.
* **String xg2DiscoverySid()** - A sid parameter from last discovery response.

* **JsonObject xg2UnicastResp()**   - Last unicast response as JsonObject.
* **JsonObject xg2MulticastResp()** - Last multicast response as JsonObject.
* **JsonObject xg2DiscoveryResp()** - Last discovery response as JsonObject.

* **JsonObject xg2MulticastDataAsJsonObject()** - Last data from multicast response as JsonObject.
* **JsonArray xg2MulticastDataAsJsonArray()** - Last data from multicast response as JsonArray.
