// recieeves joystick data and forwards to arduino via serial
#include <WiFi.h>
#include <WiFiUdp.h>

const char* ssid = "RobotCar";
const char* password = "12345678";
const int port = 4210;

WiFiUDP udp;
char buf[32];

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  WiFi.softAP(ssid, password);
  udp.begin(port);
}

void loop() {
  // put your main code here, to run repeatedly:
  int len=udp.parsePacket();
  if(len>0){ 
    udp.read(buf, sizeof(buf));
    buf[len] = '\0';
    Serial.println(buf);
  }
}
