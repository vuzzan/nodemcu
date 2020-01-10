#include <FS.h>                   //this needs to be first, or it all crashes and burns...

#include <PubSubClient.h>

#include <ESP8266WiFi.h>

#include <ArduinoOTA.h>

#include <ESP8266HTTPClient.h>

//needed for library
#include <DNSServer.h>

#include <ESP8266WebServer.h>

#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include <StringSplitter.h>


#include <DoubleResetDetect.h>
 // maximum number of seconds between resets that
// counts as a double reset 
#define DRD_TIMEOUT 2.0
// address to the block in the RTC user memory
// change it if it collides with another usage 
// of the address block
# define DRD_ADDRESS 0x00

DoubleResetDetect drd(DRD_TIMEOUT, DRD_ADDRESS);
void callback(char * topic, byte * payload, unsigned int length);
//EDIT THESE LINES TO MATCH YOUR SETUP
#define MQTT_SERVER "broker.mqttdashboard.com" //you MQTT IP Address
const char * ssid = "TPLINK006";
const char * password = "84669308";
#define mqtt_user ""
#define mqtt_password ""
#define mqtt_port 1883

# define PINGRING "CC AA 48 48 66 46 0E 91 53 2C 9C 7C 23 2A B1 74 4D 29 9D 33"

//EJ: Data PIN Assignment on WEMOS D1 R2 https://www.wemos.cc/product/d1.html
// if you are using Arduino UNO, you will need to change the "D1 ~ D4" with the corresponding UNO DATA pin number 

static const uint8_t MCU_D0 = 16;
static const uint8_t MCU_D1 = 5;//ok
static const uint8_t MCU_D2 = 4;//ok
static const uint8_t MCU_D3 = 0;//ok
static const uint8_t MCU_D4 = 2;//ok
static const uint8_t MCU_D5 = 14;
static const uint8_t MCU_D6 = 12;//ok
static const uint8_t MCU_D7 = 13;//ok
static const uint8_t MCU_D8 = 15;
static const uint8_t MCU_D9 = 3;
static const uint8_t MCU_D10 = 1;

int switchPin1 = MCU_D1;// wrong
int switchPin2 = MCU_D2;
int switchPin3 = MCU_D3;
int switchPin4 = MCU_D4;
int switchPin5 = MCU_D5;
int switchPin6 = MCU_D6;// wrong
int switchPin7 = MCU_D7;
int switchPin8 = MCU_D8;

//EJ: These are the MQTT Topic that will be used to manage the state of Relays 1 ~ 4
//EJ: Refer to my YAML component entry
//EJ: feel free to replicate the line if you have more relay switch to control, but dont forget to increment the number suffix so as increase switch logics in loop()

/*
char
const * switchTopic1 = "/house/switch1/";
char
const * switchTopic2 = "/house/switch2/";
char
const * switchTopic3 = "/house/switch3/";
char
const * switchTopic4 = "/house/switch4/";
char
const * switchTopic5 = "/house/switch5/";
char
const * switchTopic6 = "/house/switch6/";
char
const * switchTopic7 = "/house/switch7/";
char
const * switchTopic8 = "/house/switch8/";
*/


WiFiClient wifiClient;
PubSubClient client(MQTT_SERVER, mqtt_port, callback, wifiClient);

void setup() {
    Serial.begin(115200);
    Serial.println();
    Serial.println("***************************************");
    Serial.println("* Minimal double reset detect example *");
    Serial.println("***************************************");
    Serial.println();

    if (drd.detect()) {
        Serial.println("** Double reset boot **");

        //WiFiManager
        //Local intialization. Once its business is done, there is no need to keep it around
        WiFiManager wifiManager;

        //exit after config instead of connecting
        wifiManager.setBreakAfterConfig(true);

        //reset settings - for testing
        //wifiManager.resetSettings();

        //tries to connect to last known settings
        //if it does not connect it starts an access point with the specified name
        //here  "AutoConnectAP" with password "password"
        //and goes into a blocking loop awaiting configuration
        if (!wifiManager.autoConnect("AutoConnectAP", "abc123")) {
            Serial.println("failed to connect, we should reset as see if it connects");
            delay(3000);
            ESP.reset();
            delay(5000);
        }

        //if you get here you have connected to the WiFi
        Serial.println("connected...yeey :)");

        Serial.println("local ip");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("** Normal boot **");
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
        delay(100);
        Serial.println("MAC Address");
        Serial.println(WiFi.macAddress());
        //attempt to connect to the WIFI network and then connect to the MQTT server
        reconnect();
        //wait a bit before starting the main loop
        delay(2000);
    }

}
String getConfigFile() {
    Serial.println("http getConfigFile");
    HTTPClient http; //Declare object of class HTTPClient
    String param = "http://li.woodlands2win.com/ip.txt?chipid=";
    param+=ESP.getChipId();
    Serial.println(param.c_str());
    
    http.begin(param.c_str());
    http.addHeader("Content-Type", "text/plain"); //Specify content-type header
    //String postData = "macid=";
    //postData += WiFi.macAddress();
    //Serial.println(postData);
    int httpCode = http.GET(); //Send the request
    Serial.println("http 3");
    String payload = http.getString(); //Get the response payload
    Serial.println("http 4");

    Serial.println(httpCode); //Print HTTP return code
    Serial.println(payload); //Print request response payload

    http.end(); //Close connection
    return payload;
}

void loop() {

    //reconnect if connection is lost
    if (!client.connected() && WiFi.status() == 3) {
        reconnect();
    }

    //maintain MQTT connection
    client.loop();

    //MUST delay to allow ESP8266 WIFI functions to run
    delay(10);
    ArduinoOTA.handle();
}

void callback(char * topic, byte * payload, unsigned int length) {

    //convert topic to string to make it easier to work with
    String topicStr = topic;
    //EJ: Note:  the "topic" value gets overwritten everytime it receives confirmation (callback) message from MQTT

    //Print out some debugging info
    Serial.println("Callback update.");
    Serial.print("Topic: ");
    Serial.println(topicStr);
    
    String rootTopic = "/neochannel/";
    rootTopic += ESP.getChipId();
    rootTopic += "/";
    //  String switchTopic = rootTopic + "1";
    // /neochannel/chipid/gpio1
    
    if (topicStr.indexOf( "/gpio1" )>-1)
    {
        if( length> 1 ){
          StringSplitter *splitter = new StringSplitter((char *)payload, ',', 2);
          int itemCount = splitter->getItemCount();
          Serial.println("SET PIN1.");
          if(itemCount==2){
            String portID = splitter->getItemAtIndex(0);
            switchPin1 = portID.toInt();
            Serial.println("switchPin1=");
            Serial.println(switchPin1);
            pinMode(switchPin1, OUTPUT); // Relay Switch 1
            String howhigh = splitter->getItemAtIndex(1);
            int value=howhigh.toInt();
            if(value==1){
              digitalWrite(switchPin1, HIGH);
            }
            else{
              digitalWrite(switchPin1, LOW);
            }
          }
        }
        else{
          //turn the switch on if the payload is '1' and publish to the MQTT server a confirmation message
          if (payload[0] == '1') {
              digitalWrite(switchPin1, HIGH);
          }
          //turn the switch off if the payload is '0' and publish to the MQTT server a confirmation message
          else if (payload[0] == '0') {
              digitalWrite(switchPin1, LOW);
          }
        }
    }

    // EJ: copy and paste this whole else-if block, should you need to control more switches
    else if (topicStr.indexOf( "/gpio2" )>-1){
        if( length> 1 ){
          StringSplitter *splitter = new StringSplitter((char *)payload, ',', 2);
          int itemCount = splitter->getItemCount();
          if(itemCount==2){
            String portID = splitter->getItemAtIndex(0);
            switchPin2 = portID.toInt();
            pinMode(switchPin2, OUTPUT); // Relay Switch 1
            String howhigh = splitter->getItemAtIndex(1);
            int value=howhigh.toInt();
            if(value==1){
              digitalWrite(switchPin2, HIGH);
            }
            else{
              digitalWrite(switchPin2, LOW);
            }
          }
        }
        else{
          //turn the switch on if the payload is '1' and publish to the MQTT server a confirmation message
          if (payload[0] == '1') {
              digitalWrite(switchPin2, HIGH);
              //client.publish("/house/switchConfirm2/", "1");
          }
  
          //turn the switch off if the payload is '0' and publish to the MQTT server a confirmation message
          else if (payload[0] == '0') {
              digitalWrite(switchPin2, LOW);
              //client.publish("/house/switchConfirm2/", "0");
          }
        }
    } else if (topicStr.indexOf( "/gpio3" )>-1){
        if( length> 1 ){
          StringSplitter *splitter = new StringSplitter((char *)payload, ',', 2);
          int itemCount = splitter->getItemCount();
          if(itemCount==2){
            String portID = splitter->getItemAtIndex(0);
            switchPin3 = portID.toInt();
            pinMode(switchPin3, OUTPUT); // Relay Switch 1
            String howhigh = splitter->getItemAtIndex(1);
            int value=howhigh.toInt();
            if(value==1){
              digitalWrite(switchPin3, HIGH);
            }
            else{
              digitalWrite(switchPin3, LOW);
            }
          }
        }
        else{
          //turn the switch on if the payload is '1' and publish to the MQTT server a confirmation message
          if (payload[0] == '1') {
              digitalWrite(switchPin3, HIGH);
              //client.publish("/house/switchConfirm3/", "1");
          }
  
          //turn the switch off if the payload is '0' and publish to the MQTT server a confirmation message
          else if (payload[0] == '0') {
              digitalWrite(switchPin3, LOW);
              //client.publish("/house/switchConfirm3/", "0");
          }
        }
    } else if (topicStr.indexOf( "/gpio4" )>-1){
        if( length> 1 ){
          StringSplitter *splitter = new StringSplitter((char *)payload, ',', 2);
          int itemCount = splitter->getItemCount();
          if(itemCount==2){
            String portID = splitter->getItemAtIndex(0);
            switchPin4 = portID.toInt();
            pinMode(switchPin4, OUTPUT); // Relay Switch 1
            String howhigh = splitter->getItemAtIndex(1);
            int value=howhigh.toInt();
            if(value==1){
              digitalWrite(switchPin4, HIGH);
            }
            else{
              digitalWrite(switchPin4, LOW);
            }
          }
        }
        else{
          //turn the switch on if the payload is '1' and publish to the MQTT server a confirmation message
          if (payload[0] == '1') {
              digitalWrite(switchPin4, HIGH);
              //client.publish("/house/switchConfirm4/", "1");
          }
  
          //turn the switch off if the payload is '0' and publish to the MQTT server a confirmation message
          else if (payload[0] == '0') {
              digitalWrite(switchPin4, LOW);
              //client.publish("/house/switchConfirm4/", "0");
          }
        }
    ///next 5-8
    } else if (topicStr.indexOf( "/gpio5" )>-1){
         if( length> 1 ){
          StringSplitter *splitter = new StringSplitter((char *)payload, ',', 2);
          int itemCount = splitter->getItemCount();
          if(itemCount==2){
            String portID = splitter->getItemAtIndex(0);
            switchPin5 = portID.toInt();
            pinMode(switchPin5, OUTPUT); // Relay Switch 1
            String howhigh = splitter->getItemAtIndex(1);
            int value=howhigh.toInt();
            if(value==1){
              digitalWrite(switchPin5, HIGH);
            }
            else{
              digitalWrite(switchPin5, LOW);
            }
          }
        }
        else{
          //turn the switch on if the payload is '1' and publish to the MQTT server a confirmation message
          if (payload[0] == '1') {
              digitalWrite(switchPin5, HIGH);
              //client.publish("/house/switchConfirm5/", "1");
          }
  
          //turn the switch off if the payload is '0' and publish to the MQTT server a confirmation message
          else if (payload[0] == '0') {
              digitalWrite(switchPin5, LOW);
              //client.publish("/house/switchConfirm5/", "0");
          }
        }
    //switch 6
    } else if (topicStr.indexOf( "/gpio6" )>-1){
         if( length> 1 ){
          StringSplitter *splitter = new StringSplitter((char *)payload, ',', 2);
          int itemCount = splitter->getItemCount();
          if(itemCount==2){
            String portID = splitter->getItemAtIndex(0);
            switchPin6 = portID.toInt();
            pinMode(switchPin6, OUTPUT); // Relay Switch 1
            String howhigh = splitter->getItemAtIndex(1);
            int value=howhigh.toInt();
            if(value==1){
              digitalWrite(switchPin6, HIGH);
            }
            else{
              digitalWrite(switchPin6, LOW);
            }
          }
        }
        else{
          //turn the switch on if the payload is '1' and publish to the MQTT server a confirmation message
          if (payload[0] == '1') {
              digitalWrite(switchPin6, HIGH);
             // client.publish("/house/switchConfirm6/", "1");
          }
  
          //turn the switch off if the payload is '0' and publish to the MQTT server a confirmation message
          else if (payload[0] == '0') {
              digitalWrite(switchPin6, LOW);
             // client.publish("/house/switchConfirm6/", "0");
          }
        }
    } else if (topicStr.indexOf( "/gpio7" )>-1){
        if( length> 1 ){
          StringSplitter *splitter = new StringSplitter((char *)payload, ',', 2);
          int itemCount = splitter->getItemCount();
          if(itemCount==2){
            String portID = splitter->getItemAtIndex(0);
            switchPin7 = portID.toInt();
            pinMode(switchPin7, OUTPUT); // Relay Switch 1
            String howhigh = splitter->getItemAtIndex(1);
            int value=howhigh.toInt();
            if(value==1){
              digitalWrite(switchPin7, HIGH);
            }
            else{
              digitalWrite(switchPin7, LOW);
            }
          }
        }
        else{
        //turn the switch on if the payload is '1' and publish to the MQTT server a confirmation message
        if (payload[0] == '1') {
            digitalWrite(switchPin7, HIGH);
            //client.publish("/house/switchConfirm7/", "1");
        }

        //turn the switch off if the payload is '0' and publish to the MQTT server a confirmation message
        else if (payload[0] == '0') {
            digitalWrite(switchPin7, LOW);
            //client.publish("/house/switchConfirm7/", "0");
        }
       }
    } else if (topicStr.indexOf( "/gpio8" )>-1){
       if( length> 1 ){
          StringSplitter *splitter = new StringSplitter((char *)payload, ',', 2);
          int itemCount = splitter->getItemCount();
          if(itemCount==2){
            String portID = splitter->getItemAtIndex(0);
            switchPin8 = portID.toInt();
            pinMode(switchPin8, OUTPUT); // Relay Switch 1
            String howhigh = splitter->getItemAtIndex(1);
            int value=howhigh.toInt();
            if(value==1){
              digitalWrite(switchPin8, HIGH);
            }
            else{
              digitalWrite(switchPin8, LOW);
            }
          }
        }
        else{
        //turn the switch on if the payload is '1' and publish to the MQTT server a confirmation message
        if (payload[0] == '1') {
            digitalWrite(switchPin8, HIGH);
            //client.publish("/house/switchConfirm8/", "1");
        }

        //turn the switch off if the payload is '0' and publish to the MQTT server a confirmation message
        else if (payload[0] == '0') {
            digitalWrite(switchPin8, LOW);
            //client.publish("/house/switchConfirm8/", "0");
        }
        }
    }

}

void reconnect() {

    //attempt to connect to the wifi if connection is lost
    if (WiFi.status() != WL_CONNECTED) {
        //debug printing
        //Serial.print("Connecting to ");
        //Serial.println(ssid);

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
        Serial.println(WiFi.macAddress());

        getConfigFile();
    }

    //make sure we are connected to WIFI before attemping to reconnect to MQTT
    if (WiFi.status() == WL_CONNECTED) {
        // Loop until we're reconnected to the MQTT server
        while (!client.connected()) {
            Serial.print("\nAttempting MQTT connection...");

            // Generate client name based on MAC address and last 8 bits of microsecond counter
            String clientName;
            clientName = "esp8266-neo-";
            clientName += ESP.getChipId();
            
            uint8_t mac[6];
            WiFi.macAddress(mac);
            Serial.print("\tMQTT clientName=");
            Serial.print((char * ) clientName.c_str());

            if (client.connect((char * ) clientName.c_str())) { //EJ: Update accordingly with your MQTT account 
                Serial.print("\tMQTT Connected");
                String rootTopic0 = "/neochannel/";
                String rootTopic = rootTopic0 + ESP.getChipId() + "/gpio";

                String switchTopic = rootTopic + "1";
                client.subscribe(switchTopic.c_str());
                Serial.print("\n subscribe ");
                Serial.println(switchTopic.c_str());

                switchTopic = rootTopic + "2";
                client.subscribe(switchTopic.c_str());
                Serial.print("\n subscribe ");
                Serial.println(switchTopic.c_str());

                switchTopic = rootTopic + "3";
                client.subscribe(switchTopic.c_str());
                Serial.print("\n subscribe ");
                Serial.println(switchTopic.c_str());

                switchTopic = rootTopic + "4";
                client.subscribe(switchTopic.c_str());
                Serial.print("\n subscribe ");
                Serial.println(switchTopic.c_str());

                switchTopic = rootTopic + "5";
                client.subscribe(switchTopic.c_str());
                Serial.print("\n subscribe ");
                Serial.println(switchTopic.c_str());

                switchTopic = rootTopic + "6";
                client.subscribe(switchTopic.c_str());
                Serial.print("\n subscribe ");
                Serial.println(switchTopic.c_str());


                switchTopic = rootTopic + "7";
                client.subscribe(switchTopic.c_str());
                Serial.print("\n subscribe ");
                Serial.println(switchTopic.c_str());


                switchTopic = rootTopic + "8";
                client.subscribe(switchTopic.c_str());
                Serial.print("\n subscribe ");
                Serial.println(switchTopic.c_str());


                Serial.print("\n subscribe done");
                //String rootTopic0 = "/neochannel/";
                //String rootTopic = rootTopic0 + ESP.getChipId() + "/gpio";
                String sendLogin = rootTopic0 +"login/";
                String chipIdString = "";
                chipIdString += ESP.getChipId();
                Serial.print("\n Send ChipID: ");
                Serial.println(chipIdString.c_str());
                client.publish(sendLogin.c_str(), chipIdString.c_str());
                //EJ: Do not forget to replicate the above line if you will have more than the above number of relay switches
            }

            //otherwise print failed for debugging
            else {
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
String macToStr(const uint8_t * mac) {

    String result;

    for (int i = 0; i < 6; ++i) {
        result += String(mac[i], 16);

        if (i < 5) {
            result += ':';
        }
    }

    return result;
}
