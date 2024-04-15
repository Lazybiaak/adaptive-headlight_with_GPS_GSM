#include <Wire.h>
#include <TinyGPS++.h>
#include <Servo.h>
#include <math.h>

HardwareSerial& GPS = Serial;  // assuming your board has Serial1
TinyGPSPlus gps;

#define CTRL_REG1 0x20
#define CTRL_REG2 0x21
#define CTRL_REG3 0x22
#define CTRL_REG4 0x23
#define CTRL_REG5 0x24

float x; // angular velocity around x-axis
float y; // angular velocity around y-axis
float z; // angular velocity around z-axis
float angleZ = 0; // angle of rotation around z-axis
unsigned long previousTime = 0;
int ldrpin=A0;
int ledpin=11;
int brightness;
float Latitude, Longitude;
// L3G4200D gyro address
int L3G4200D_Address = 105;
Servo myservo;

String message;// = "Hey there. My live location is: Location not available";
void setup() {

  Serial.begin(9600);
  GPS.begin(9600);
  Wire.begin();
  pinMode(ldrpin, INPUT);
  pinMode(ledpin,OUTPUT);
  digitalWrite(ledpin,LOW);
  myservo.attach(10);  // attaches the servo on pin 9 to the servo object
  delay(100);
  myservo.write(90);
  Serial.println("starting up L3G4200D");
  setupL3G4200D(2000); // Configure L3G4200  - 250, 500 or 2000 deg/sec
  delay(1500);
  Serial.println("Suru vo");
}

void loop() {
  int red=analogRead(ldrpin);
  if(red>800){
    analogWrite(ledpin,10);}
  else{
  brightness= map(red, 0, 800, 255, 0);

  analogWrite(ledpin,brightness);
  }
  // Read GPS data
  

  unsigned long currentTime = millis();
  float elapsedTime = (currentTime - previousTime) / 1000.0; // Convert to seconds

  getGyroValues();  // This will update x, y, and z with new values

  // Integrate angular velocities to get angles of rotation
  if(z>=-10 && z<=10)z=0;
  angleZ += (z * elapsedTime);
 float degree = map(angleZ, 1500,-1500,180,0);
  if(degree>180)degree=180;
  if(degree<0)degree=0;
  Serial.print("   Angle : ");
  Serial.println(degree);
  myservo.write(degree);
  previousTime = currentTime;

  while (GPS.available() > 0) {

    gps.encode(GPS.read());
    if (gps.location.isUpdated()) {
      Serial.println("GPS available");
      Latitude = gps.location.lat();
      Longitude = gps.location.lng();
      Serial.print("Latitude= ");
      Serial.print(Latitude, 6);
      Serial.print(" Longitude= ");
      Serial.println(Longitude, 6);
      if (gps.date.isValid() && gps.time.isValid()) {
        int hour;
        int year = gps.date.year();
        int month = gps.date.month();
        int day = gps.date.day();
        int minute;
        if (gps.time.minute() + 45 > 60) {
          hour = (gps.time.hour() + 6) % 24;
          minute = (gps.time.minute() + 45) % 60;
        } else {
          hour = gps.time.hour() + 5;
          minute = (gps.time.minute() + 45);
        }
        int second = gps.time.second();

        Serial.print("Date: ");
        Serial.print(year);
        Serial.print("-");
        Serial.print(month);
        Serial.print("-");
        Serial.print(day);
        Serial.print(" Time: ");
        Serial.print(hour);
        Serial.print(":");
        Serial.print(minute);
        Serial.print(":");
        Serial.println(second);

        message += String(Latitude, 6);
        message += ",";
        message += String(Longitude, 6);
        message += ",";
        message += String(month);
        message += "-";
        message += String(day);
        message += ", ";
        message += String(hour);
        message += ":";
        message += String(minute);
      }
      // Convert the String to a char array
      char charArray[message.length() + 1];
      message.toCharArray(charArray, message.length() + 1);

      Wire.beginTransmission(9);                           // transmit to device #9
      Wire.write(message.c_str());  
      Serial.println("message to gsm sent");
      Wire.endTransmission();  // stop transmitting
    }
  }
}

void getGyroValues() {
  byte zMSB = readRegister(L3G4200D_Address, 0x2D);
  byte zLSB = readRegister(L3G4200D_Address, 0x2C);
  z = ((zMSB << 8) | zLSB);
}

int setupL3G4200D(int scale) {
  //From  Jim Lindblom of Sparkfun's code

  // Enable x, y, z and turn off power down:
  writeRegister(L3G4200D_Address, CTRL_REG1, 0b00001111);

  // If you'd like to adjust/use the HPF, you can edit the line below to configure CTRL_REG2:
  writeRegister(L3G4200D_Address, CTRL_REG2, 0b00000000);

  // Configure CTRL_REG3 to generate data ready interrupt on INT2
  // No interrupts used on INT1, if you'd like to configure INT1
  // or INT2 otherwise, consult the datasheet:
  writeRegister(L3G4200D_Address, CTRL_REG3, 0b00001000);

  // CTRL_REG4 controls the full-scale range, among other things:

  if (scale == 250) {
    writeRegister(L3G4200D_Address, CTRL_REG4, 0b00000000);
  } else if (scale == 500) {
    writeRegister(L3G4200D_Address, CTRL_REG4, 0b00010000);
  } else {
    writeRegister(L3G4200D_Address, CTRL_REG4, 0b00110000);
  }

  // CTRL_REG5 controls high-pass filtering of outputs, use it
  // if you'd like:
  writeRegister(L3G4200D_Address, CTRL_REG5, 0b00000000);
}

void writeRegister(int deviceAddress, byte address, byte val) {
  Wire.beginTransmission(deviceAddress); // start transmission to device 
  Wire.write(address);       // send register address
  Wire.write(val);         // send value to write
  Wire.endTransmission();     // end transmission
}

int readRegister(int deviceAddress, byte address) {

  int v;
  Wire.beginTransmission(deviceAddress);
  Wire.write(address); // register to read
  Wire.endTransmission();

  Wire.requestFrom(deviceAddress, 1); // read a byte

  while (!Wire.available()) {
    // waiting
  }

  v = Wire.read();
  return v;
}
