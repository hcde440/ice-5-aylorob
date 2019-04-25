
#include <ESP8266WiFi.h>    
#include <PubSubClient.h>   
#include <ESP8266HTTPClient.h>                            
#include <ArduinoJson.h>                                  
#include <Adafruit_Sensor.h>                              
#include <DHT.h>
#include <DHT_U.h>
#include <Wire.h>
#include <Adafruit_MPL115A2.h>    //Include these libraries
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "config.h"

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//////////
//So to clarify, we are connecting to and MQTT server
//that has a login and password authentication
//I hope you remember the user and password
//////////

#define mqtt_server "mediatedspaces.net"  //this is its address, unique to the server
#define mqtt_user "hcdeiot"               //this is its server login, unique to the server
#define mqtt_password "esp8266"           //this is it server password, unique to the server

//////////
//We also need to publish and subscribe to topics, for this sketch are going
//to adopt a topic/subtopic addressing scheme: topic/subtopic
//////////

WiFiClient espClient;             //blah blah blah, espClient
PubSubClient mqtt(espClient);     //blah blah blah, tie PubSub (mqtt) client to WiFi client

//////////
//We need a 'truly' unique client ID for our esp8266, all client names on the server must be unique.
//Every device, app, other MQTT server, etc that connects to an MQTT server must have a unique client ID.
//This is the only way the server can keep every device separate and deal with them as individual devices/apps.
//The client ID is unique to the device.
//////////

char mac[6]; //A MAC address is a 'truly' unique ID for each device, lets use that as our 'truly' unique user ID!!!

//////////
//In our loop(), we are going to create a c-string that will be our message to the MQTT server, we will
//be generous and give ourselves 200 characters in our array, if we need more, just change this number
//////////

char message[201]; //201, as last character in the array is the NULL character, denoting the end of the array

/////SETUP/////
void setup() {
  
  Serial.begin(115200);
  
  // wait for serial monitor to open
  while(! Serial);
  
  Serial.print("This board is running: ");
  Serial.println(F(__FILE__));                            //These four lines give description of of file name and date 
  Serial.print("Complied: ");
  Serial.println(F(__DATE__ " " __TIME__));

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x32)
  display.clearDisplay();                     // Clear any existing image
  display.setTextSize(1);                     // Set text size
  display.setTextColor(WHITE);                // Set text color
  display.setCursor(0, 0);                    // Put cursor back on top left
  display.println("Starting up.");            // Test and write up
  display.display();                          // Display the display
  
  setup_wifi();
  mqtt.setServer(mqtt_server, 1883);
  mqtt.setCallback(callback); //register the callback function
}

/////SETUP_WIFI/////
void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);
  WiFi.begin(wifi_ssid, wifi_password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");  //get the unique MAC address to use as MQTT client ID, a 'truly' unique ID.
  Serial.println(WiFi.macAddress());  //.macAddress returns a byte array 6 bytes representing the MAC address
}                                     //5C:CF:7F:F0:B0:C1 for example

/////CONNECT/RECONNECT/////Monitor the connection to MQTT server, if down, reconnect
void reconnect() {
  // Loop until we're reconnected
  while (!mqtt.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (mqtt.connect(mac, mqtt_user, mqtt_password)) { //<<---using MAC as client ID, always unique!!!
      Serial.println("connected");
      mqtt.subscribe("weather/+"); //we are subscribing to 'theTopic' and all subtopics below that topic
    } else {                        //please change 'theTopic' to reflect your topic you are subscribing to
      Serial.print("failed, rc=");
      Serial.print(mqtt.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

/////LOOP/////
void loop() {

  if (!mqtt.connected()) {                                      //Reconnect if lost connection
    reconnect();
  }

  mqtt.loop();                                                  //this keeps the mqtt connection 'active'

}//end Loop


/////CALLBACK/////
//The callback is where we attacch a listener to the incoming messages from the server.
//By subscribing to a specific channel or topic, we can listen to those topics we wish to hear.
//We place the callback in a separate tab so we can edit it easier . . . (will not appear in separate
//tab on github!)
/////

void callback(char* topic, byte* payload, unsigned int length) {
  
  Serial.println();
  Serial.println("//////////////////");                 //To seperate the sends
  Serial.println();
  
  Serial.println();
  Serial.print("Message arrived [");
  Serial.print(topic); //'topic' refers to the incoming topic name, the 1st argument of the callback function
  Serial.println("] ");

  DynamicJsonBuffer  jsonBuffer; //blah blah blah a DJB
  JsonObject& root = jsonBuffer.parseObject(payload); //parse it!

  if (!root.success()) { //well?
    Serial.println("parseObject() failed, are you sure this message is JSON formatted.");
    return;
  }

  String temp = root["temp"].as<String>();
  String humd = root["humd"].as<String>();
  String pres = root["pres"].as<String>();

  Serial.print("The temperature in F is: ");
  Serial.println(temp);
  Serial.print("The humidity is: ");
  Serial.println(humd);
  Serial.print("The pressure in KPa is: ");
  Serial.println(pres);

  display.clearDisplay();                           //Clear test display off

  display.setCursor(0, 0);                          //Reset cursor
  display.print("Temp is: ");                       //Print all this to display
  display.println(temp);
  display.print("Humd is: ");
  display.println(humd);
  display.print("Pres is: ");
  display.println(pres);
  display.display();                                //Display the display
  
}
