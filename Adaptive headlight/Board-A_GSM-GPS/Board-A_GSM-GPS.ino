#include <SoftwareSerial.h>
#include <Wire.h>

#define GSM_TX_PIN 9
#define GSM_RX_PIN 8

SoftwareSerial gsmSerial(GSM_TX_PIN, GSM_RX_PIN);  // GSM connection
String message = "Location not available";
String phoneNumber[] = { "+9779867993602", "+9779840853915", "+9779841751148", "9840853915" };
float Latitude = 27.707706, Longitude = 85.325290;

void setup() {
  Serial.begin(9600);
  Wire.begin(9);
  gsmSerial.begin(9600);

  delay(2000);

  gsmSerial.println("AT");
  delay(1000);
  gsmSerial.println("AT+CMGF=1");  // Set SMS mode to text
  Serial.println("Initiated");\
  Wire.onReceive(receiveEvent); 
}
void receiveEvent(int howMany) {
  message = "Hey there. My live location is: http://www.biwek.com.np/maps/?q=";
  while (1 < Wire.available()) { // loop through all but the last
    char c = Wire.read(); // receive byte as a character
    message +=c;         // print the character
  }
}
void loop() {
  while (Wire.available()) {
    char c = Wire.read();  // read one character from the I2C
    message += c;          // append the character to the message
  }
  Serial.println("Message received: " + message);

  String incomingPhoneNumber = getIncomingCallPhoneNumber();

  if (incomingPhoneNumber != "") {
    Serial.print("Incoming call detected! Phone number: ");
    Serial.println(incomingPhoneNumber);
    if (isPhoneNumberValid(incomingPhoneNumber.c_str(), phoneNumber, 4)) {
      delay(30000);
      sendLocation(message, incomingPhoneNumber);
      delay(3000);
      Serial.println("Calling: ");
      gsmSerial.print("ATD" + incomingPhoneNumber + ";\r");
      delay(20000);
      gsmSerial.println("ATH");  // Hang up the call
      Serial.println("Call Ended: ");
    }
  }
}
bool isPhoneNumberValid(const char* incomingPhoneNumber, const String phoneNumber[], int size) {
  for (int i = 0; i < size; ++i) {
    if (strcmp(incomingPhoneNumber, phoneNumber[i].c_str()) == 0) {
      return true;  // Match found
    }
  }
  return false;  // No match found
}

void sendLocation(String message, String phNumber) {

  // Send SMS
  gsmSerial.println("AT+CMGS=\"" + phNumber + "\"");
  delay(1000);
  gsmSerial.println(message);
  delay(100);
  gsmSerial.write(0x1A);  // End of message
  delay(1000);

  // Check for "OK" response
  if (gsmSerial.find("OK")) {
    Serial.println("Message sent successfully.");
  } else {
    Serial.println("Failed to send message.");
    Serial.println("Check GSM module and network.");
  }
}
String getIncomingCallPhoneNumber() {
  gsmSerial.println("AT+CLCC");
  delay(100);

  if (gsmSerial.find("+CLCC: 1,1")) {
    gsmSerial.readStringUntil(',');
    gsmSerial.readStringUntil('"');                       // Skip the opening quote
    String phoneNumber = gsmSerial.readStringUntil('"');  // Read the phone number
    return phoneNumber;
  }

  return "";
}
