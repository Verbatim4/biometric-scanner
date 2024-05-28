#include <Adafruit_Fingerprint.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>

#if (defined(__AVR__) || defined(ESP8266)) && !defined(__AVR_ATmega2560__)
SoftwareSerial mySerial(14, 12);
#else
#define mySerial Serial1
#endif

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

// WiFi credentials
const char* ssid = "Pixel 4a";
const char* password = "12345678";

// Google Sheets script URL
String serverName = "https://script.google.com/macros/s/AKfycbxGfQf9NoNtuf6KNynVebPVXOeW1SbLi8F4qtREIjpbAIdIbiglGCbM6RqmrlR6YeCo/exec";

// Define fingerprint ID to name mappings
// Add additional IDs as needed
#define SHREEKAR 999
#define HEMANTH 1
#define NAVEEN 12

// Timing variables
unsigned long lastTime = 0;
unsigned long timerDelay = 1000; 

void setup() {
  Serial.begin(9600);
  while (!Serial);
  delay(100);
  
  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi...");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());

  // Initialize fingerprint sensor
  Serial.println("\n\nAdafruit finger detect test");
  finger.begin(57600);
  delay(5);
  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    while (1) { delay(1); }
  }

  // Read sensor parameters
  Serial.println(F("Reading sensor parameters"));
  finger.getParameters();
  Serial.print(F("Status: 0x")); Serial.println(finger.status_reg, HEX);
  Serial.print(F("Sys ID: 0x")); Serial.println(finger.system_id, HEX);
  Serial.print(F("Capacity: ")); Serial.println(finger.capacity);
  Serial.print(F("Security level: ")); Serial.println(finger.security_level);
  Serial.print(F("Device address: ")); Serial.println(finger.device_addr, HEX);
  Serial.print(F("Packet len: ")); Serial.println(finger.packet_len);
  Serial.print(F("Baud rate: ")); Serial.println(finger.baud_rate);

  finger.getTemplateCount();

  if (finger.templateCount == 0) {
    Serial.print("Sensor doesn't contain any fingerprint data. Please run the 'enroll' example.");
  } else {
    Serial.println("Waiting for valid finger...");
    Serial.print("Sensor contains "); Serial.print(finger.templateCount); Serial.println(" templates");
  }
}

void loop() {
  // Check if 2 seconds have passed
  if ((millis() - lastTime) > timerDelay) {
    int id = getFingerprintID();
    if (id > -1) {
      // Map ID to name
      String name;
      if (id == SHREEKAR) {
        name = "Shreekar";
      } else if (id == HEMANTH) {
        name = "Hemanth";
      } else if (id == NAVEEN) {
        name = "Naveen";
      } else {
        name = "Unknown";
      }

      // Send data to Google Sheets
      if (WiFi.status() == WL_CONNECTED) {
        WiFiClientSecure client;
        client.setInsecure(); // Bypass SSL verification for testing

        HTTPClient http;

        String serverPath = serverName + "?value1=" + String(id) + "&value2=" + name;
        
        // Send HTTP GET request
        http.begin(client, serverPath.c_str());
        int httpResponseCode = http.GET();
        
        if (httpResponseCode > 0) {
          Serial.print("HTTP Response code: ");
          Serial.println(httpResponseCode);
          String payload = http.getString();
          Serial.println(payload);
        } else {
          Serial.print("Error code: ");
          Serial.println(httpResponseCode);
        }
        http.end();
      } else {
        Serial.println("WiFi Disconnected");
      }
    } else {
      Serial.println("No finger detected or no match found");
    }
    // Update the last time the GET request was sent
    lastTime = millis();
  }
}

int getFingerprintID() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK) return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK) return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK) {
    if (p == FINGERPRINT_NOTFOUND) {
      Serial.println("No match found");
    } else {
      Serial.println("Unknown error");
    }
    return -1;
  }

  // Found a match!
  Serial.print("Found ID #"); Serial.print(finger.fingerID);
  Serial.print(" with confidence of "); Serial.println(finger.confidence);
  return finger.fingerID;
}
