#include <AdafruitIO_WiFi.h>
#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

// Import required libraries
#include "ESP8266WiFi.h"
#include <Wire.h>
#include <Adafruit_MotorShield.h>

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;
// or... use WiFiFlientSecure for SSL
//WiFiClientSecure client;

// Create the motor shield object with the default I2C address
Adafruit_MotorShield AFMS = Adafruit_MotorShield(); 
 
// And connect 2 DC motors to port M3 & M4 !
Adafruit_DCMotor *L_MOTOR = AFMS.getMotor(1);
Adafruit_DCMotor *R_MOTOR = AFMS.getMotor(2);

#define AIO_SERVER     "io.adafruit.com"
#define AIO_SERVERPORT 1883 
#define AIO_USERNAME   "xxxxxxxx"
#define AIO_KEY        "xxxxxxxxxxxxxxxxxxxxxxx"

// WiFi parameters
const char* ssid = "xxxxxxxx";
const char* password = "xxxxxxxxx";

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
Adafruit_MQTT_Subscribe forward  = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/audreya-fwd");
Adafruit_MQTT_Subscribe backward = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/audrey.a-bkwd");
Adafruit_MQTT_Subscribe left     = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/audrey.a-left");
Adafruit_MQTT_Subscribe right    = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/audrey.a-right");
Adafruit_MQTT_Subscribe stop_    = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/audrey.a-stop");
Adafruit_MQTT_Publish   conn     = Adafruit_MQTT_Publish(&mqtt,   AIO_USERNAME "/feeds/audrey.a-connection");
//Adafruit_MQTT_Publish   conn     = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/audrey.a-connection, MQTT_QOS_1");
void MQTT_connect();

void setup(void) {  
  // Start Serial
  Serial.begin(115200);
  while(! Serial);
  Serial.println(" ... Start Setup ... ");
  Serial.println(" Connecting to Adafruit IO");
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("WiFi connected");
  Serial.println("IP address: "); Serial.println(WiFi.localIP());

  MQTT_connect();
 
  mqtt.subscribe(&forward);
  mqtt.subscribe(&backward);
  mqtt.subscribe(&left);
  mqtt.subscribe(&right);
  mqtt.subscribe(&stop_);
  //mqtt.subscribe(&conn);  

  // Init motor shield
  AFMS.begin();  
  
  Serial.println(" ... End   Setup ... ");  
}

void loop() {
  Serial.println("--- Start Loop ---");
  
  MQTT_connect();

  // this is our 'wait for incoming subscription packets' busy subloop
  // try to spend your time here

  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(5000))) {
    Serial.println(" Subscribing... ");
    if (subscription == &forward) {
      Serial.print(F("Got: "));
      Serial.println((char *)forward.lastread);
      r_forward();
    } else if ( subscription == &backward ) {
      Serial.print(F("Got: "));
      Serial.println((char *)backward.lastread);

    } else if ( subscription == &left ) {
      Serial.print(F("Got: "));
      Serial.println((char *)left.lastread);
    
    } else if ( subscription == &right ) {
      Serial.print(F("Got: "));
      Serial.println((char *)right.lastread);
    
    } else if ( subscription == &stop_ ) {
      Serial.print(F("Got: "));
      Serial.println((char *)stop_.lastread);
    
    } else {
      Serial.print((char *)subscription);
    }
  }

  // Now we can publish stuff!
  delay(1000);
  Serial.print(F("\nSending connection data "));
  //Serial.print();
  Serial.print("...");
  if (! conn.publish("foobar")) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!"));
  }
  //Serial.println("WiFi connected");
  //Serial.println("IP address: "); Serial.println(WiFi.localIP());
  Serial.println("--- End Loop ---");
  
}

void r_stop(AdafruitIO_Data *data) {
  
  // Stop
  Serial.println(" --- Stop --- ");
  
  L_MOTOR->setSpeed(0);
  L_MOTOR->run( RELEASE );
 
  R_MOTOR->setSpeed(0);
  R_MOTOR->run( RELEASE );
  
}

void r_forward() {
  
  // Stop
  Serial.println(" --- Forward --- ");
  
  L_MOTOR->setSpeed(100);
  L_MOTOR->run( FORWARD );
 
  R_MOTOR->setSpeed(100);
  R_MOTOR->run( FORWARD );
  
}

void r_left(AdafruitIO_Data *data) {
  
  Serial.println(" --- Left --- ");
 
  L_MOTOR->setSpeed(50);
  L_MOTOR->run( BACKWARD );
 
  R_MOTOR->setSpeed(50);
  R_MOTOR->run( FORWARD );
  
}

void r_right(AdafruitIO_Data *data) {
  
  // Stop
    Serial.println(" --- Right --- ");
  
  L_MOTOR->setSpeed(50);
  L_MOTOR->run( FORWARD );
 
  R_MOTOR->setSpeed(50);
  R_MOTOR->run( BACKWARD );
  
}

void r_backward(AdafruitIO_Data *data) {
  
  // Stop
  
  L_MOTOR->setSpeed(75);
  L_MOTOR->run( BACKWARD );
 
  R_MOTOR->setSpeed(75);
  R_MOTOR->run( BACKWARD );
  
}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    Serial.println(" MQTT Connected! ");
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(8000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  Serial.println("MQTT Connected!");
}