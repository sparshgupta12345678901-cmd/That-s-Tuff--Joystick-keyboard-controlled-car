// controller esp32 sends joystick data stuff over wifi
#include <Wifi.h>
#include <WifiUdp.h>

const char* ssid = "RobotCar";
const char* password = "123345678";
const char* carIP = "192.168.4.1";
const int port = 4210;

WiFiUDP udp;

void setup() {
  // put your setup code here, to run once:
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(300);
  udp.begin(4211);
}

void loop() {
  // put your main code here, to run repeatedly:
  int x=analogRead(1);
  int y=analogRead(2);

  char msg[32];
  sprintf(msg, "%d,%d", x,y);
  udp.beginPacket(carIP, port);
  udp.print(msg);
  udp.endPacket();
  delay(30);
}
