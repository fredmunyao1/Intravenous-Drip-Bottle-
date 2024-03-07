#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SoftwareSerial.h>

#define OLED_RESET 6
Adafruit_SSD1306 display(OLED_RESET);

const int trigPin = 4;   // Trigger pin of Ultrasonic sensor
const int echoPin = 3;   // Echo pin of Ultrasonic sensor
const int relayPin = 7;  // Relay pin
long duration;
int distance;
int waterLevel;

// GSM setup
SoftwareSerial gsmSerial(10, 11); // RX, TX pins for GSM module
String phoneNumber = "+254707857644"; // Phone number to send SMS and make call
bool callMade = false;
bool messageSent = false;

void setup() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW); // Initially turn off the relay

  // GSM module initialization
  gsmSerial.begin(9600);
  delay(1000); // Wait for the module to initialize

  // OLED display initialization
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.println("IV Level   Monitor");
  display.display();
  delay(2000);
  display.clearDisplay();
}

void loop() {
  // Clear the previous distance value
  distance = 0;

  // Trigger the ultrasonic sensor
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Read the echo pin, and calculate the distance
  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.034 / 2;

  // Determine the water level based on distance
  // Modify these thresholds according to your setup
  if (distance >= 19) {
    waterLevel = 0; // Empty
    digitalWrite(relayPin, HIGH); // Turn on the relay when water level is empty
    if (!callMade) {
      makeCall();
      callMade = true;
    }
  } else if (distance >= 15 && distance < 19) {
    waterLevel = 1; // Low
    digitalWrite(relayPin, LOW);
    if (!messageSent) {
      sendSMS("2");
      messageSent = true;
    }
  } else if (distance >= 6 && distance < 15) {
    waterLevel = 2; // Medium
    digitalWrite(relayPin, LOW); // Turn off the relay when water level is medium
    if (!messageSent) {
      sendSMS("Water level is medium.");
      messageSent = true;
    }
  } else {
    waterLevel = 3; // High
    digitalWrite(relayPin, LOW); // Turn off the relay when water level is high
    callMade = false; // Reset call flag
    messageSent = false; // Reset message flag
  }

  // Display water level on OLED
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.print("Distance: ");
  display.println(distance);
  display.print("IV Level: ");
  switch (waterLevel) {
    case 0:
      display.println("Empty");
      break;
    case 1:
      display.println("Low");
      break;
    case 2:
      display.println("Medium");
      break;
    case 3:
      display.println("High");
      break;
  }
  display.setTextSize(1);
  display.setCursor(26,16);
  display.println("Dispensing:");
  display.setCursor(21,24);
  display.println("Normal Saline");
  
  display.display();

  delay(1000); // Delay for stability
}

void sendSMS(String message) {
  gsmSerial.println("AT+CMGF=1"); // Set the GSM module to text mode
  delay(1000);
  gsmSerial.print("AT+CMGS=\"");
  gsmSerial.print(phoneNumber);
  gsmSerial.println("\"");
  delay(1000);
  gsmSerial.println(message);
  gsmSerial.println((char)26); // ASCII code of CTRL+Z
  delay(1000);
}

void makeCall() {
  gsmSerial.println("ATD" + phoneNumber + ";"); // Command to make a call
  delay(1000);
}
