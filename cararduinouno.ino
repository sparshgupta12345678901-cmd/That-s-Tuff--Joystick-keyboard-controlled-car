#define ENA 5
#define ENB 6
#define IN1 7
#define IN2 8
#define IN3 9
#define IN4 11

void setup() {
  Serial.begin(9600);
  pinMode(IN1,OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3,OUTPUT); pinMode(IN4,OUTPUT);
  pinMode(ENA,OUTPUT); pinMode(ENB,OUTPUT);  
}

void loop() {
  if(Serial.available()){
    String data=Serial.readStringUntil("\n");
    int comma = data.indexOf(",");
    if(comma>0){
      int x=data.substring(0,comma).toInt();
      int y=data.substring(comma+1).toInt();
      int forward = map(y,0,4095,255,-255);
      int turn = map(x,0,4095,-255,255);
      if(abs(forward)<15) forward = 0;
      if(abs(turn)<15) turn=0;
      int L=constrain(forward+turn,-255,255);
      int R=constrain(forward-turn,-255,255);
      setMotor(L,R);
    }
  }
}

void setMotor(int left, int right){
  digitalWrite(IN1, left>0);
  digitalWrite(IN2, left<=0);
  analogWrite(ENA,abs(left));
  digitalWrite(IN3, right>0);
  digitalWrite(IN4, right<=0);
  analogWrite(ENB, abs(right));
}