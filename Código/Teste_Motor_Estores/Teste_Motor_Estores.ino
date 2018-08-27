void setup() {
  // put your setup code here, to run once:
pinMode(4,OUTPUT);
digitalWrite(4,HIGH);
pinMode(5,OUTPUT);
digitalWrite(5,HIGH);
}

void loop() {
 // put your main code here, to run repeatedly:
delay(10000);


digitalWrite(5,HIGH);
delay(200);
digitalWrite(4,LOW);

delay(10000);


digitalWrite(4,HIGH);
delay(200);
digitalWrite(5,LOW);



delay(10000);
digitalWrite(4,HIGH);
digitalWrite(5,HIGH);

}
