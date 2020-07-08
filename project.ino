
/* 
  A fire emergency response systems that monitors levels of smoke, frame and temperature.
  Incase their is extream values from these sensors then means their might be a fire incidence.
  The system 'requests' to put up an alarm. The system informs the user about this events via use of Text messages.
  Users can cancel the incoming alarm by texting "OFF" the sim card in the system.
  This gives the user time to verify the incident as their can be a system failure too. If the request is not cancled then an alarm is put up.
  the alarm event includes: opening the door, Texting the user (again)/relevant bodies about the incidence, 
  giving them crucial info about the incident eg street name, location of incidence, house number.etc
  The system as other functions also.


  By: Martin Muthomi and Anne Mburu
  School Project
  www.martinmuthomi.co.ke
  Email: info@martinmuthomi.co.ke
    
     
   Digital Pin 3: DC gear reduction motor attached to the lock. (Or a motor controller or a solenoid or other unlocking mechanisim.)
   Digital Pin 4: Red LED. 
   Digital Pin 5: Green LED.
   Digital Pin 6: yellow LED.
   Digital Pin 7: GSM (7).
   Digital Pin 8: GSM (8).
   Digital Pin 10: piezo buzzer
   Digital Pin 11: Button.
   Digital Pin 13: flame sensor.
   Analog pin A1: temp sensor
   Analog pin A2: smoke sensor
   
    

 */


#include <SoftwareSerial.h>
#include <Servo.h>           //library for servo for unlocking door

//Create software serial object to communicate with SIM900
SoftwareSerial SIM900(7, 8); //SIM900 Tx & Rx is connected to Arduino #7 & #8
Servo lockMotor;

int buttonState = 0; 
String button;
int buttonv = 1;
String userMessage = "There is a fire incident in your home";
String userNumber = "+254792020946";
String marshalMessage = "Fire incident at House number 6,| location -> googlemaps.com";
String marshalNumber = "+254770728779";
String alarmStatus;
int now;
int initialTime;
int delayTime;
String textMessage;
int temp;
int flame_pin = HIGH ; // state of sensor

int smokeA2 = A2;
int pinTemp = A1;   //This is where our Output data goes

const int lockpin = 3;   // Configure servo pin
const int redLED = 4;              // Status red LED
const int greenLED = 5;            // Status green LED
const int yellowLED = 6;
const int piezoPin = 10;  //PIEZO PIN
const int buttonPin = 11; 
const int flame_sensor_pin = 13 ;// initializing pin 12 as the sensor output pin

void setup()
{
  lockMotor.attach(lockpin);           // Gear motor used to turn the lock.
  pinMode(smokeA2, INPUT);
  pinMode(pinTemp, INPUT); 
  pinMode(redLED, OUTPUT);
  pinMode(greenLED, OUTPUT);
  pinMode(yellowLED, OUTPUT);
  pinMode(piezoPin, OUTPUT);
  pinMode(buttonPin, INPUT);
  pinMode ( flame_sensor_pin , INPUT );  
  
  lockMotor.write(60);
  digitalWrite(greenLED, HIGH);        // Green LED on, everything is go.
  delay(1000);
  digitalWrite(greenLED, LOW);
  digitalWrite(yellowLED, HIGH);        // Green LED on, everything is go.
  delay(1000);
  digitalWrite(yellowLED, LOW);
  digitalWrite(redLED, HIGH);        // Yellow LED on, everything is go. //change the format they are lighting
  delay(1000);
  digitalWrite(redLED, LOW);

  
  alarmStatus = "off";
  
  //Begin serial communication with Arduino and Arduino IDE (Serial Monitor)
  Serial.begin(9600);
  
  //Begin serial communication with Arduino and SIM900
  SIM900.begin(9600);

  Serial.println("Initializing..."); 
  delay(1000);

  SIM900.println("AT"); //Handshaking with SIM900
  updateSerial();
  
  SIM900.println("AT+CMGF=1"); // Configuring TEXT mode
  updateSerial();
  SIM900.println("AT+CNMI=1,2,0,0,0"); // Decides how newly arrived SMS messages should be handled
  updateSerial();
  delay(1000);
}

void loop()
{
  
  
  Serial.print("alarm status is: ");
  Serial.println(alarmStatus);
  Serial.print(analogRead(smokeA2));
  Serial.println(temp);

  // read the state of the pushbutton value:
  buttonState = digitalRead(buttonPin);


   if (buttonState == HIGH ) {  
  alarmOff();
  }
    
 if(SIM900.available()>0){
  receiveSMS();  
}

if (alarmStatus == "set" ){
  digitalWrite(yellowLED, HIGH);        // Yellow LED on, everything is go.
  delay(200);
  digitalWrite(yellowLED, LOW);
   Serial.println("alarm status");
  alarmOn();
}
  else if (alarmStatus == "on"){

  Serial.println("alarm is on");
  tone(piezoPin, 1000, 200);
   digitalWrite(redLED, HIGH);        // Green LED on, everything is go.
  delay(200);
  digitalWrite(redLED, LOW);
  delay(800);
  noTone(piezoPin);
  
  } else {
  
  digitalWrite(greenLED, HIGH);        // Green LED on, everything is go.
  delay(100);
  digitalWrite(greenLED, LOW);
 if(analogRead(smokeA2)> 250){
  smokefunction();
 }
 
//check temp reading and compare with its threshold
 temp = analogRead(pinTemp);    //Read the analog pin
 temp = temp * 0.48828125;   // convert output (mv) to readable celcius
//float voltage = temp * 3.3;
//voltage /= 1024.0;
//temp = (voltage - 0.5) * 100;
if (temp >36) {
  tempfunction(temp);
}


flame_pin = digitalRead ( flame_sensor_pin ) ; // reading from the sensor
if (flame_pin == LOW ) // applying condition
{
Serial.println ( " FLAME , FLAME , FLAME " ) ;
flameSensor();
}


}//end of alarm status check 

} // end of loop 









//function called when smoke value exceed threshold
 void smokefunction(){
  //something on smoke
  int smokevalue = analogRead(smokeA2);
  Serial.print("The smoke value is: ");
  Serial.println(smokevalue);
//   Serial.println(smokevalue);
   
    //tone( pin number, frequency in hertz, duration in milliseconds);
    String message = "There is an incoming fire alarm. Kindly act accordingly. You can cancel within 30sec by replying with OFF.";
    sendSMS(message, userNumber);
    initialTime = millis(); //take time when alarm request started
    alarmOn();
   //  tone(piezoPin, 1000, 200);
    // triggerDoorUnlock();
 
   delay(2000);
 }










//temp function. called when the temp threshold is exceded 
void tempfunction(int temperature){
  
  // request for an alarm alert
  String message = "There is an incoming fire alarm. Kindly act accordingly. You can cancel within 30sec by replying with 'OFF'.";
  Serial.print("Temperature value is: ");
  Serial.println(temperature);
 // Serial.println("C");  //print the temperature status
 //  String Message = "Their is an incoming fire alarm. Kindly act accordingly. You can cancel within 30sec.";
   sendSMS(message, userNumber);
  initialTime = millis(); //take time when alarm request started
  alarmOn();
//  tone(piezoPin, 1000, 200);
//  delay(1000); 
//  noTone(piezoPin);
  
}








void flameSensor(){
    String message = "There is an incoming fire alarm. Kindly act accordingly. You can cancel within 30sec by replying with 'OFF'.";
    sendSMS(message, userNumber);
    initialTime = millis(); //take time when alarm request started
    alarmOn();
}







void alarmOn(){
  int cutoffTime = 30000; // 30 seconds
  Serial.print("initial time is: ");
  Serial.println(initialTime);
  alarmStatus = "set";
  now = millis();
  Serial.print("now is: ");
  Serial.println(now);
  delayTime = now - initialTime;   //in seconds
  //send a message to the user
  while (alarmStatus == "set"){
    buttonState = digitalRead(buttonPin);
//  Serial.print("button state is: ");
//  Serial.println(buttonState);
   if (buttonState == HIGH ) {  //not working
  alarmOff();
  }
  if(SIM900.available()>0){
  receiveSMS();  
  }
  digitalWrite(yellowLED, HIGH);        // Yellow LED on, everything is go.
  delay(100);
  digitalWrite(yellowLED, LOW);
  now = millis();
  delayTime = now - initialTime;   //in seconds
  Serial.print("delay time is: ");
  Serial.println(delayTime);
  if (delayTime > cutoffTime ){
    
    
    //triggerDoorUnlock();  //open door
    //tone(piezoPin, 1000, 200);
    sendSMS(userMessage, userNumber);

    sendSMS(marshalMessage, marshalNumber);

    triggerDoorUnlock();  //open door
    //send message to the owners ->sendSMS("message here",recpeits);
    //send message to the firefighter ->sendSMS("map link",recpeits);
    alarmStatus = "on";
  //  delay(2000);
    delayTime = 0;
    //check if i can add continue/break so that code dont proceed to temp
    break;
  }
  delay(2000);
  }
}









void alarmOff(){

  //close door
  //put off alarm
  alarmStatus = "off";
  triggerDoorlock(); 
  noTone(piezoPin);
  
}










// Runs the motor (or whatever) to unlock the door.
void triggerDoorUnlock(){
  Serial.println("Door unlocked!");

  lockMotor.write(180);
  digitalWrite(greenLED, HIGH);            // And the green LED too.
  delay(500);
  digitalWrite(greenLED, LOW);
  delay(500);
                  
}









// Runs the motor (or whatever) to unlock the door.
void triggerDoorlock(){
  Serial.println("Door locked!");

  
  lockMotor.write(60);             //Turn back the motor
  digitalWrite(greenLED, HIGH);            // And the green LED too.
  delay(500);
  digitalWrite(greenLED, LOW);

   
}











 void receiveSMS(){
      
    textMessage = SIM900.readString();
    Serial.print(textMessage);    
    delay(10);   
    if(textMessage.indexOf("OFF")>=0){
   // triggerDoorUnlock();        //open door by calling doorunlock function
    alarmOff();
    textMessage = "";  
     
  }
    if(textMessage.indexOf("ON")>=0){
   // triggerDoorlock();        //open door by calling doorunlock function
   //    initialTime = millis();
   //    alarmOn();
   if(alarmStatus == "off"){   //if alarm is on dont interfere with it
   initialTime = millis(); //take time when alarm request started
   alarmStatus = "set"; //change coz door wouldnt open
   }
    textMessage = "";   
  }

  if(textMessage.indexOf("STATE")>=0){
    String message = "Temp, smoke and flames levels are: //give values ";
   // sendSMS(message);
    Serial.println("Door state resquest");
    textMessage = "";
  }
  // add firestation
  // add number
  
    }








//// Function that sends SMS
//void sendSMS(String text, String Number){
//  
//  SIM900.println("AT"); //Handshaking with SIM900
//  updateSerial();
//  delay(1000);
//  SIM900.println("AT+CMGF=1"); // Configuring TEXT mode
//  updateSerial();
//  delay(1000);
//  SIM900.println("AT+CMGS=\""+Number+"\"");//change ZZ with country code and xxxxxxxxxxx with phone number to sms
//  updateSerial();
//  delay(1000);
//  SIM900.print(text); //text content
//  updateSerial();
//  delay(1000);
//  SIM900.write(26);
//  delay(1000);
//}







// Function that sends SMS
void sendSMS(String text, String Number){
  
  SIM900.println("AT"); //Handshaking with SIM900
  updateSerial();

  SIM900.println("AT+CMGF=1"); // Configuring TEXT mode
  updateSerial();
  SIM900.println("AT+CMGS=\""+Number+"\"");//change ZZ with country code and xxxxxxxxxxx with phone number to sms
  updateSerial();
  SIM900.print(text); //text content
  updateSerial();
  SIM900.write(26);
  delay(4000);
}











void updateSerial()
{
  delay(500);
  while (Serial.available()) 
  {
    SIM900.write(Serial.read());//Forward what Serial received to Software Serial Port
  }
  while(SIM900.available()) 
  {
    Serial.write(SIM900.read());//Forward what Software Serial received to Serial Port
  }
}
