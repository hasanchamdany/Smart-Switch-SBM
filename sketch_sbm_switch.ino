#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <dummy.h>

// definisi wifi
const char *ssid = "huplaa";
const char *password = "super12345";

// digunakan local host laptop dan digunakan untuk nanti membuka Dashboard di Handphone
const char *mqtt_server = "192.168.43.19";
const int mqtt_port = 1883;

// relay state
#define on LOW
#define off HIGH

// untuk client wifi
WiFiClient espClient;
PubSubClient client(espClient);

// define topic mqtt
#define TOPIC_MODE "switch/mode"
#define TOPIC_LAMP_1 "switch/lamp/1"
#define TOPIC_LAMP_2 "switch/lamp/2"

// initial state
String state_mode = "AUTO";
int state_lamp_1 = 0;
int state_lamp_2 = 0;
int state_lamp_3 = 0;
int state_lamp_4 = 0;

// relay
int relayState = off;
const int relay[] = {33, 25, 26, 27};
const int VCCPin = 35;
//int relayState[] = {relay, relay, off, off};

// ssr relay
const int ssrPin = 23;
int ssrState = off;
float vccVal = 0;

// Button
const int BtnPinMode = 2;

// Interrupt
portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;


void relayInit()
{
  for (int i = 0; i < 4; ++i)
  {
    pinMode(relay[i], OUTPUT);
    digitalWrite(relay[i], off);
  }
}

void toggleAllRelay(int paramsRelay)
{
  relayState = paramsRelay;
  for (int i = 0; i < 4; ++i)
  {
    digitalWrite(relay[i], relayState);
  }
}

void toggleRelay(int pin, int paramsRelay)
{
//  ssrState = paramsRelay;
  digitalWrite(pin, paramsRelay);
}

void mqttconnect()
{
  /* Loop until reconnected */
  while (!client.connected())
  {
    Serial.print("MQTT connecting ...");
    String clientId = "ESP32Client";
    if (client.connect(clientId.c_str()))
    {
      Serial.println(" Connected");

      // led
      client.subscribe(TOPIC_MODE);
      client.subscribe(TOPIC_LAMP_1);
      client.subscribe(TOPIC_LAMP_2);
    }
    else
    {
      Serial.print("failed, status code =");
      Serial.print(client.state());
      Serial.println("try again in 5 seconds");
      delay(5000);
    }
  }
}

void callback(char *topic, byte *message, unsigned int length)
{
  Serial.println();
  Serial.print("Message arrived on topic: ");
  Serial.println(topic);
  String messageTemp;

  for (int i = 0; i < length; i++)
  {
    messageTemp += (char)message[i];
  }

  Serial.print("Topic: ");
  Serial.println(topic);
  Serial.print("Message: ");
  Serial.println(messageTemp);

  if (String(topic) == TOPIC_MODE)
  {
    state_mode = messageTemp;
    if (messageTemp == "AUTO") {
      toggleAllRelay(false);
    }
    else if (messageTemp == "MANUAL") {
      toggleAllRelay(true);
    }
  } else if (String(topic) == TOPIC_LAMP_1)
  {
    Serial.println("masuk lamp 1");
    state_lamp_1 = messageTemp == "true" ? HIGH : LOW;
  } else if (String(topic) == TOPIC_LAMP_2)
  {
    Serial.println("masuk lamp 2");
    state_lamp_2 = messageTemp == "true" ? HIGH : LOW;
  }
}

void mqttInit() {
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void IRAM_ATTR ButtonISR(){
   portENTER_CRITICAL_ISR(&mux);
   state_mode = !state_mode;
   portEXIT_CRITICAL_ISR(&mux);
  }

void setup()
{
  Serial.begin(9600);
  pinMode(ssrPin, OUTPUT);
  pinMode(BtnPinMOde,INPUT_PULLUP);
  relayInit();
  mqttInit();
  toggleAllRelay(true);
  attachInterrupt(digitalPinToInterrupt(BtnPinMOde), ButtonISR, FALLING);
  
}

void loop()
{  
  mqttconnect();
  client.loop();
  
  Serial.println(state_mode);
  vccVal = analogRead(VCCPin);

  if(vccVal > 2000){
    Serial.println("nyala");
    toggleRelay(ssrPin, true);
  } else {
    Serial.println("mati");
    toggleRelay(ssrPin, false);
  }
  delay(200);
}
