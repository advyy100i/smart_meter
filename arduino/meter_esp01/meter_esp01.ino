#include <ESP8266WiFi.h>
#include <SerialTransfer.h>
#include <WiFiUdp.h>
#include <NTPClient.h>

#define ESP01_PACKET_MAGIC 0x2A4D
#define ESP01_BAUD_RATE 115200

#define SSID "twin"
#define PASSWORD "gogowifi"

const uint32_t MeterID = 0x12345678;

SerialTransfer esp01_transfer;
WiFiUDP wifiUdp;
NTPClient ntpClient(wifiUdp);

typedef struct __attribute__((packed)) esp01_packet
{
  uint16_t magic;
  uint16_t voltage;
  uint16_t current;
  uint32_t power;
  uint32_t energy;
} esp01_packet_t;

typedef struct tcp_packet
{
  uint32_t timestamp;
  uint32_t meter_id;
  uint16_t voltage;
  uint16_t current;
  uint32_t power;
  uint32_t energy;
} tcp_packet_t;

WiFiClient client;

/** Try to reconnect to the server. */
void reconnect_to_server()
{
  // keep on retrying until we are connected
  while (!client.connect("192.168.165.1", 5000))
  {
    delay(100);
  }
}

void setup()
{
  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(100);
  }

  // connect to our data aggregation server
  client.connect("192.168.165.1", 5000);

  // begin serial connection to our main board
  Serial.begin(ESP01_BAUD_RATE);
  esp01_transfer.begin(Serial);
  ntpClient.begin();
}

void loop()
{
  if (esp01_transfer.available())
  {
    esp01_packet_t packet;
    esp01_transfer.rxObj(packet, 0, sizeof(esp01_packet_t));

    if (packet.magic == ESP01_PACKET_MAGIC)
    {
      ntpClient.update();
      uint32_t timestamp = ntpClient.getEpochTime();
      tcp_packet_t tcp_packet;
      tcp_packet.meter_id = MeterID;
      tcp_packet.timestamp = timestamp;
      tcp_packet.voltage = packet.voltage;
      tcp_packet.current = packet.current;
      tcp_packet.power = packet.power;
      tcp_packet.energy = packet.energy;

      // send the packet to the server
      if (client.connected())
      {
        client.write((uint8_t *)&tcp_packet, sizeof(tcp_packet_t));
      }
      else
      {
        reconnect_to_server();
      }
    }
  }
}
