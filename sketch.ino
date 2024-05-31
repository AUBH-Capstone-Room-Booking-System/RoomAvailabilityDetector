#include <DHT.h>
#include "WiFiEsp.h"
#include "SoftwareSerial.h"

// Pins for SoftwareSerial
SoftwareSerial softserial(A9, A8); // RX, TX

// WiFi network credentials
char ssid[] = "ssid";
char pass[] = "***";

// Server details
const char* serverName = "192.168.100.132"; // Replace with your machine's local network IP address
const int serverPort = 3000;

int status = WL_IDLE_STATUS;

WiFiEspClient client;

#define SOUND_SENSOR_ANALOG_PIN A1 // Analog pin 0
int digit_sensor = 2;
// Define the DHT sensor pin and type
#define DHTPIN 3  // Digital pin 3
#define DHTTYPE DHT11  // DHT 11

// Initialize the DHT sensor
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  // Start serial communication for debugging
  Serial.begin(9600);
  softserial.begin(115200);
  softserial.write("AT+CIOBAUD=9600\r\n");
  softserial.write("AT+RST\r\n");
  softserial.begin(9600);

  // Initialize ESP module
  WiFi.init(&softserial);

  // Check for the presence of the shield
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    while (true);
  }

  // Attempt to connect to WiFi network
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid, pass);
    delay(5000);
  }

  Serial.println("You're connected to the network");
  printWifiStatus();
  pinMode (digit_sensor, INPUT);


  // Initialize the DHT sensor
  dht.begin();

  // Initialize the sound sensor pin
}

void loop() {
  // Read sensor values
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
    int digitValue=digitalRead(digit_sensor);


  // Send sensor data to the server
  sendSensorData(temperature, humidity, digitValue);

  // Wait for 5 seconds before sending next data
  delay(5000);
}

void sendSensorData(float temperature, float humidity, int digitValue) {
  if (client.connect(serverName, serverPort)) {
    Serial.println("Connected to server");

    // Create the URL with sensor values
    String url = "/api/sensor-data?";
    url += "roomNumber=10&temperature=";
    url += temperature;
    url += "&humidity=";
    url += humidity;
    url += "&soundLevel=";
    url += digitValue;
    url += "&airQuality=Good&comfort=4";

    // Make a HTTP GET request
    client.print("GET ");
    client.print(url);
    client.print(" HTTP/1.1\r\n");
    client.print("Host: ");
    client.print(serverName);
    client.print(":");
    client.print(serverPort);
    client.print("\r\n");
    client.print("Connection: close\r\n");
    client.print("\r\n");

    // Wait for a response from the server
    while (client.available()) {
      char c = client.read();
      Serial.print(c);
    }

    // Close the connection
    client.stop();
    Serial.println("Disconnected from server");
  } else {
    Serial.println("Connection to server failed");
  }
}

void printWifiStatus() {
  // Print the SSID of the network you're attached to
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // Print your WiFi shield's IP address
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  
  // Print where to go in the browser
  Serial.println();
  Serial.print("To see this page in action, open a browser to http://");
  Serial.println(ip);
  Serial.println();
}