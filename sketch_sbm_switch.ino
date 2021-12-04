#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <dummy.h>

// definisi wifi
const char *ssid = "HASANSBM";
const char *password = "HASANSBM";

// digunakan local host laptop dan digunakan untuk nanti membuka Dashboard di Handphone
const char *mqtt_server = "192.168.137.1";
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
const int relayPin[] = {27, 33};


// ssr relay
const int ssrPin[] = {22, 23};
const int switchPin[] = {34, 36};
//float ssrVal[] = {0,0};

// Button
const int BtnPinMode = 2;
int buttonState = 0;
int button_time = 0;
int last_button_time = 0;

// Interrupt
//portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

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
    state_lamp_1 = messageTemp == "true" ? HIGH : LOW;
  } else if (String(topic) == TOPIC_LAMP_2)
  {
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

void init()
{
  // relay
  pinMode(relayPin[0], OUTPUT);
  pinMode(relayPin[1], OUTPUT);
  digitalWrite(relayPin[0], LOW);
  digitalWrite(relayPin[1], LOW);

  //ssr
  pinMode(ssrPin[0], OUTPUT);
  pinMode(ssrPin[1], OUTPUT);

  // mode
  state_mode = "MANUAL";

}

void toggleAllRelay(int paramsRelay)
{
  digitalWrite(relayPin[0], paramsRelay);
  digitalWrite(relayPin[1], paramsRelay);
}

void toggleRelay(int pin, int paramsRelay)
{
  digitalWrite(pin, paramsRelay);
}

void ssrHandle(float paramsSSRVal, int paramsSSRPin) {
  if (paramsSSRVal > 2000 ) {
    Serial.println("nyala");
    toggleRelay(paramsSSRPin, true);
  } else {
    Serial.println("mati");
    toggleRelay(paramsSSRPin, false);
  }
}

void ssr() {
  // read vcc``
  float val1 = analogRead(36);
  delay(10);
  val1 = analogRead(36);
  delay(10);
  Serial.println();

  //  Serial.println("Status SSR");

  // ssr 1
  Serial.print("SSR 1: ");
  Serial.print(val1);
  Serial.print("; ");
  //  ssrHandle(val1, ssrPin[0]);

  delay(1000);
  float val2 = analogRead(15);
  delay(10);
  val2 = analogRead(15);
  delay(10);

  // ssr 2
  Serial.print("SSR 2: ");
  Serial.print(val2);
  Serial.print("; ");
  //  ssrHandle(val2, ssrPin[1]);
}

void setup()
{
  Serial.begin(115200);
  init();
  mqttInit();
}

void loop()
{
  //  // testing
  //  toggleAllRelay(true);
  //  delay(2000);
  //  toggleAllRelay(false);
  //  delay(2000);

//  mqttconnect();
//  client.loop();
//  Serial.println(state_mode);

  ssr();
//  delay(200);
}
