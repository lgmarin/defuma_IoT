#include <SPI.h>
#include <TM1637Display.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <index_html.h>

// USING GPIO PINS FOR ESP12 Compatibility!
// D# Pins correspond to nodeMCU V1.2 pins
// TM1637 4d Display Pins
// SCLK -> D2 - GPIO4
// DIO  -> D7 - GPIO13
int tm_DIO = 13;
int tm_CLK = 4;

TM1637Display display(tm_CLK, tm_DIO);

// Define main pins for MAX6675
// SCLK -> D5 - GPIO14
// MISO -> D6 - GPIO12
// CS   -> D8 - GPIO15
int max_CS = 15;
float temperature = 0;

// Buzzer PIN
// BUZZ -> D1 - GPIO5
int BUZZ = 5;

// WebServer Configuration
const char* ssid = "Your_SSID";   //replace with your SSID
const char* password = "Your_Password";    //replace with your password

String temp_low = "24.0";
String temp_high = "26.0";
String last_temperature;

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

// Open WebServer connection at PORT 80
AsyncWebServer server(80);

// String processor to be used to parse data to the web browser
String processor(const String& var){

  if(var == "TEMPERATURE"){
    return last_temperature;
  }
  else if(var == "THRESHOLD_MAX"){
    return temp_high;
  }
  else if(var == "THRESHOLD_MIN"){
    return temp_low;
  }
  return String();
}

// Configure time interval between readings - 1 second
unsigned long previousMillis = 0;     
const long interval = 1000;

double readTemperatureC(uint8_t CS) {
  //READ MAX6675 Temperature in Celsius using SPI Interface
  uint16_t v;

  digitalWrite(CS, LOW);
  v = SPI.transfer(0x00);
  v <<= 8;
  v |= SPI.transfer(0x00);
  digitalWrite(CS, HIGH);

  if (v & 0x4) {
      // No thermocouple attached, return 0
      return NAN; 
  }

  v >>= 3;

  return v*0.25;
}

void setup(){
  Serial.begin(9600);

  // Prepare Display and Wait for MAX initialization
  uint8_t data[] = { 0xff, 0xff, 0xff, 0xff };
  display.setBrightness(0x0f);
  display.setSegments(data);

  //Intiate SPI transaction
  SPI.begin();

  //Set max_CS Pin as OUTPUT and set to HIGH to Intiate MAX 6675
  pinMode(max_CS, OUTPUT);
  digitalWrite(max_CS, HIGH);

  // Set Buzzer PIN as Output
  pinMode(BUZZ, OUTPUT);

  // Delay for MAX init
  // delay(500);

  // Prepare WIFI Connection
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connecting...");
    return;
  }
  Serial.println();
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  display.clear();

  // Configure Server Async calls

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });

  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
    if (request->hasParam("threshold_max") && request->hasParam("threshold_min")) {
      temp_high = request->getParam("threshold_max")->value();
      temp_low = request->getParam("threshold_min")->value();
    }

    Serial.println("Set threshold_max");
    Serial.println(temp_high);
    Serial.println("Set threshold_min");
    Serial.println(temp_low);

    request->send(200, "text/html", "HTTP GET request sent to your ESP.<br><a href=\"/\">Return to Home Page</a>");
  });

  server.onNotFound(notFound);
  server.begin();    
}

void loop(){
  // Use millis() to generate a counter and ditch delay()
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    //Read Temperature Data
    temperature = readTemperatureC(max_CS);

    if (temperature == NAN)
    {
      Serial.print("Error: No Thermocouple connected!");
    } 
    else
    {
      Serial.print("Temperature: ");
      Serial.print(temperature);
      Serial.println(" °C");
      display.showNumberDec(temperature);
    }
    Serial.print("\n");

    // Keep track of the last temperature read
    last_temperature = String(temperature);
    
    if(temperature > temp_high.toFloat() && temperature != NAN){
      String message = String("Temperature is too high. Current temperature: ") + 
                            String(temperature) + String("C");
      Serial.println(message);
      tone(BUZZ, 523, 800);
    }
    else if(temperature < temp_low.toFloat() && temperature != NAN){
      String message = String("Temperature is too low. Current temperature: ") + 
                            String(temperature) + String(" C");
      Serial.println(message);
      tone(BUZZ, 1000, 200);
    }
    else {
      noTone(BUZZ);
    }
  }
}