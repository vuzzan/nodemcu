// Original sketch courtesy of ItKindaWorks
//ItKindaWorks - Creative Commons 2016
//github.com/ItKindaWorks
//
//Requires PubSubClient found here: https://github.com/knolleary/pubsubclient
//
//ESP8266 Simple MQTT switch controller


#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>
#include <ESP8266HTTPClient.h>

void callback(char* topic, byte* payload, unsigned int length);

//EDIT THESE LINES TO MATCH YOUR SETUP
#define MQTT_SERVER "broker.mqttdashboard.com"  //you MQTT IP Address
const char* ssid = "TPLINK006";
const char* password = "84669308";
#define mqtt_user "" 
#define mqtt_password ""
#define mqtt_port 1883

#define PINGRING "CC AA 48 48 66 46 0E 91 53 2C 9C 7C 23 2A B1 74 4D 29 9D 33"

//EJ: Data PIN Assignment on WEMOS D1 R2 https://www.wemos.cc/product/d1.html
// if you are using Arduino UNO, you will need to change the "D1 ~ D4" with the corresponding UNO DATA pin number 

static const uint8_t MCU_D0   = 16;
static const uint8_t MCU_D1   = 5;
static const uint8_t MCU_D2   = 4;
static const uint8_t MCU_D3   = 0;
static const uint8_t MCU_D4   = 2;
static const uint8_t MCU_D5   = 14;
static const uint8_t MCU_D6   = 12;
static const uint8_t MCU_D7   = 13;
static const uint8_t MCU_D8   = 15;
static const uint8_t MCU_D9   = 3;
static const uint8_t MCU_D10  = 1;

const int switchPin1 = MCU_D1;
const int switchPin2 = MCU_D2;
const int switchPin3 = MCU_D3;
const int switchPin4 = MCU_D5;
const int switchPin5 = MCU_D4;
const int switchPin6 = MCU_D6;
const int switchPin7 = MCU_D7;
const int switchPin8 = MCU_D8;

//EJ: These are the MQTT Topic that will be used to manage the state of Relays 1 ~ 4
//EJ: Refer to my YAML component entry
//EJ: feel free to replicate the line if you have more relay switch to control, but dont forget to increment the number suffix so as increase switch logics in loop()

char const* switchTopic1 = "/house/switch1/";
char const* switchTopic2 = "/house/switch2/";
char const* switchTopic3 = "/house/switch3/";
char const* switchTopic4 = "/house/switch4/";
char const* switchTopic5 = "/house/switch5/";
char const* switchTopic6 = "/house/switch6/";
char const* switchTopic7 = "/house/switch7/";
char const* switchTopic8 = "/house/switch8/";


WiFiClient wifiClient;
PubSubClient client(MQTT_SERVER, mqtt_port, callback, wifiClient);
//PubSubClient client(wifiClient);

void setup() {
  //initialize the switch as an output and set to LOW (off)
  pinMode(switchPin1, OUTPUT); // Relay Switch 1
  digitalWrite(switchPin1, LOW);

  pinMode(switchPin2, OUTPUT); // Relay Switch 2
  digitalWrite(switchPin2, LOW);

  pinMode(switchPin3, OUTPUT); // Relay Switch 3
  digitalWrite(switchPin3, LOW);

  pinMode(switchPin4, OUTPUT); // Relay Switch 4
  digitalWrite(switchPin4, LOW);

 pinMode(switchPin5, OUTPUT); // Relay Switch 5
  digitalWrite(switchPin5, LOW);

  pinMode(switchPin6, OUTPUT); // Relay Switch 6
  digitalWrite(switchPin6, LOW);

  pinMode(switchPin7, OUTPUT); // Relay Switch 7
  digitalWrite(switchPin7, LOW);

  pinMode(switchPin8, OUTPUT); // Relay Switch 8
  digitalWrite(switchPin8, LOW);
  
  ArduinoOTA.setHostname("My Arduino WEMO"); // A name given to your ESP8266 module when discovering it as a port in ARDUINO IDE
  ArduinoOTA.begin(); // OTA initialization

  //start the serial line for debugging
  Serial.begin(115200);
  delay(100);

  //start wifi subsystem
  Serial.println("Wifi begin.");
  WiFi.begin(ssid, password);
  //Serial.println("setServer.");
  //client.setServer(MQTT_SERVER, mqtt_port);
  //Serial.println("setServer callback.");
  //client.setCallback(callback);
  Serial.println("MAC Address");
  Serial.println( WiFi.macAddress() );
  //attempt to connect to the WIFI network and then connect to the MQTT server
  reconnect();
  //wait a bit before starting the main loop
  delay(2000);
}
String getConfigFile(){
  Serial.println("http getConf");
  HTTPClient http;    //Declare object of class HTTPClient
  //http.begin("https://so789.xyz/app/gallery.txt", PINGRING);      //Specify request destination
  http.begin("http://li.woodlands2win.com/ip.txt");
  http.addHeader("Content-Type", "text/plain");  //Specify content-type header
  String postData = "macid=";
  postData += WiFi.macAddress();
  Serial.println("http post");
  Serial.println(postData);
  int httpCode = http.GET();   //Send the request
  Serial.println("http 3");
  String payload = http.getString();                  //Get the response payload
  Serial.println("http 4");
 
  Serial.println(httpCode);   //Print HTTP return code
  Serial.println(payload);    //Print request response payload
 
  http.end();  //Close connection
  return payload;
}

void loop(){

  //reconnect if connection is lost
  if (!client.connected() && WiFi.status() == 3) {reconnect();}

  //maintain MQTT connection
  client.loop();

  //MUST delay to allow ESP8266 WIFI functions to run
  delay(10); 
  ArduinoOTA.handle();
}

void callback(char* topic, byte* payload, unsigned int length) {

  //convert topic to string to make it easier to work with
  String topicStr = topic; 
  //EJ: Note:  the "topic" value gets overwritten everytime it receives confirmation (callback) message from MQTT

  //Print out some debugging info
  Serial.println("Callback update.");
  Serial.print("Topic: ");
  Serial.println(topicStr);

   if (topicStr == "/house/switch1/") 
    {
      Serial.println("Process 1");
     //turn the switch on if the payload is '1' and publish to the MQTT server a confirmation message
     if(payload[0] == '1'){
      Serial.println("Confirm Process 1");
       digitalWrite(switchPin1, HIGH);
       client.publish("/house/switchConfirm1/", "1");
       }

      //turn the switch off if the payload is '0' and publish to the MQTT server a confirmation message
     else if (payload[0] == '0'){
      Serial.println("Confirm Process 0");
       digitalWrite(switchPin1, LOW);
       client.publish("/house/switchConfirm1/", "0");
       }
       else{
        Serial.println("Confirm Process .....");
       }
     }

     // EJ: copy and paste this whole else-if block, should you need to control more switches
     else if (topicStr == "/house/switch2/") 
     {
     //turn the switch on if the payload is '1' and publish to the MQTT server a confirmation message
     if(payload[0] == '1'){
       digitalWrite(switchPin2, HIGH);
       client.publish("/house/switchConfirm2/", "1");
       }

      //turn the switch off if the payload is '0' and publish to the MQTT server a confirmation message
     else if (payload[0] == '0'){
       digitalWrite(switchPin2, LOW);
       client.publish("/house/switchConfirm2/", "0");
       }
     }
     else if (topicStr == "/house/switch3/") 
     {
     //turn the switch on if the payload is '1' and publish to the MQTT server a confirmation message
     if(payload[0] == '1'){
       digitalWrite(switchPin3, HIGH);
       client.publish("/house/switchConfirm3/", "1");
       }

      //turn the switch off if the payload is '0' and publish to the MQTT server a confirmation message
     else if (payload[0] == '0'){
       digitalWrite(switchPin3, LOW);
       client.publish("/house/switchConfirm3/", "0");
       }
     }
     else if (topicStr == "/house/switch4/") 
     {
     //turn the switch on if the payload is '1' and publish to the MQTT server a confirmation message
     if(payload[0] == '1'){
       digitalWrite(switchPin4, HIGH);
       client.publish("/house/switchConfirm4/", "1");
       }

      //turn the switch off if the payload is '0' and publish to the MQTT server a confirmation message
     else if (payload[0] == '0'){
       digitalWrite(switchPin4, LOW);
       client.publish("/house/switchConfirm4/", "0");
       }
     }


     ///next 5-8

      else if (topicStr == "/house/switch5/") 
     {
     //turn the switch on if the payload is '1' and publish to the MQTT server a confirmation message
     if(payload[0] == '1')
     {
       digitalWrite(switchPin5, HIGH);
       client.publish("/house/switchConfirm5/", "1");
       }

      //turn the switch off if the payload is '0' and publish to the MQTT server a confirmation message
     else if (payload[0] == '0')
     {
       digitalWrite(switchPin5, LOW);
       client.publish("/house/switchConfirm5/", "0");
       }
     }

     //switch 6

      else if (topicStr == "/house/switch6/") 
     {
     //turn the switch on if the payload is '1' and publish to the MQTT server a confirmation message
     if(payload[0] == '1')
     {
       digitalWrite(switchPin6, HIGH);
       client.publish("/house/switchConfirm6/", "1");
       }

      //turn the switch off if the payload is '0' and publish to the MQTT server a confirmation message
     else if (payload[0] == '0')
     {
       digitalWrite(switchPin6, LOW);
       client.publish("/house/switchConfirm6/", "0");
       }
     }

 else if (topicStr == "/house/switch7/") 
     {
     //turn the switch on if the payload is '1' and publish to the MQTT server a confirmation message
     if(payload[0] == '1')
     {
       digitalWrite(switchPin7, HIGH);
       client.publish("/house/switchConfirm7/", "1");
       }

      //turn the switch off if the payload is '0' and publish to the MQTT server a confirmation message
     else if (payload[0] == '0')
     {
       digitalWrite(switchPin7, LOW);
       client.publish("/house/switchConfirm7/", "0");
       }
     }


     else if (topicStr == "/house/switch8/") 
     {
     //turn the switch on if the payload is '1' and publish to the MQTT server a confirmation message
     if(payload[0] == '1')
     {
       digitalWrite(switchPin8, HIGH);
       client.publish("/house/switchConfirm8/", "1");
       }

      //turn the switch off if the payload is '0' and publish to the MQTT server a confirmation message
     else if (payload[0] == '0')
     {
       digitalWrite(switchPin8, LOW);
       client.publish("/house/switchConfirm8/", "0");
       }
     }

      
}


void reconnect() {

  //attempt to connect to the wifi if connection is lost
  if(WiFi.status() != WL_CONNECTED){
    //debug printing
    Serial.print("Connecting to ");
    Serial.println(ssid);

    //loop while we wait for connection
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }

    //print out some more debug once connected
    Serial.println("");
    Serial.println("WiFi connected");  
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.print("MAC Address: ");
    Serial.println( WiFi.macAddress() );

    getConfigFile();
  }

  //make sure we are connected to WIFI before attemping to reconnect to MQTT
  if(WiFi.status() == WL_CONNECTED){
  // Loop until we're reconnected to the MQTT server
    while (!client.connected()) {
      Serial.print("\nAttempting MQTT connection...");

      // Generate client name based on MAC address and last 8 bits of microsecond counter
      String clientName;
      clientName = "esp8266-123456neo";
      uint8_t mac[6];
      WiFi.macAddress(mac);
      //clientName += macToStr(mac);
      Serial.print("\tMQTT clientName=");
      Serial.print((char*) clientName.c_str());
     //if connected, subscribe to the topic(s) we want to be notified about
      //EJ: Delete "mqtt_username", and "mqtt_password" here if you are not using any 
      //if (client.connect((char*) clientName.c_str(),"admin", "Qazxsw2@")) {  //EJ: Update accordingly with your MQTT account 
      // if (client.connect((char*) clientName.c_str(),mqtt_user, mqtt_password)) {  //EJ: Update accordingly with your MQTT account 
       if (client.connect((char*) clientName.c_str())) {  //EJ: Update accordingly with your MQTT account 
        Serial.print("\tMQTT Connected");
        client.subscribe(switchTopic1);
        client.subscribe(switchTopic2);
        client.subscribe(switchTopic3);
        client.subscribe(switchTopic4);
        client.subscribe(switchTopic5);
        client.subscribe(switchTopic6);
        client.subscribe(switchTopic7);
        client.subscribe(switchTopic8);
        Serial.print("\n subscribe done");
        client.publish("/house/switchConfirm1/", "1");
        client.publish("/house/switchConfirm2/", "1");
        client.publish("/house/switchConfirm3/", "1");
        client.publish("/house/switchConfirm4/", "1");
        
        //EJ: Do not forget to replicate the above line if you will have more than the above number of relay switches
      }

      //otherwise print failed for debugging
      else{
          //Serial.println("\tFailed."); abort();
          Serial.print("\nfailed, rc=");
          Serial.print(client.state());
          Serial.println(" try again in 5 seconds...\n");
          // Wait 5 seconds before retrying
          delay(5000);
        }
     
    }
  }
}

//generate unique name from MAC addr
String macToStr(const uint8_t* mac){

  String result;

  for (int i = 0; i < 6; ++i) {
    result += String(mac[i], 16);

    if (i < 5){
      result += ':';
    }
  }

  return result;
}
