#include <WiFi.h>
#include <SPIFFS.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

// Piny ESP32
#define motor_pin_1 19 
#define motor_pin_2 18
#define enable_pin 22
// #define button 23
#define sensor_position_0 23
#define ENCA 15
#define ENCB 2
#define relay 5

const char* ssid = "Lamel R&D 2.4GHz";
const char* password = "!lamel2024.";

AsyncWebServer server(80);

int pos = 0;
bool button_is_pressed = false;
bool check = false;
bool start_new_cycle = true;
bool ball_grabbed = false;
volatile int targetPosition = 0;

void readEncoder(){
  int b = digitalRead(ENCB);
  if (b > 0) {
    pos++;
  } else {
    pos--;
  }
}

void get_back_to_0() {

  while (digitalRead(sensor_position_0) == false && check == false) {
    digitalWrite(enable_pin, HIGH);
    digitalWrite(motor_pin_1, HIGH);
    digitalWrite(motor_pin_2, LOW);
  }

  digitalWrite(enable_pin, LOW);
  digitalWrite(motor_pin_1, LOW);
  digitalWrite(motor_pin_2, LOW);

  check = true; 
  start_new_cycle = false;

}

void catchBall() {
  check = false;
  start_new_cycle = true;

  while (digitalRead(sensor_position_0) == LOW && !check && start_new_cycle) {
    digitalWrite(enable_pin, HIGH);
    digitalWrite(motor_pin_1, HIGH);
    digitalWrite(motor_pin_2, LOW);
    Serial.println(pos);
  }

  if (digitalRead(sensor_position_0) == HIGH) {
    digitalWrite(enable_pin, LOW);
    digitalWrite(motor_pin_1, LOW);
    digitalWrite(motor_pin_2, LOW);
    Serial.println("Magnes dojechał do kuli");
    check = true;
    start_new_cycle = false;
  }
}

void one_and_a_quarter() {
  check = false;
  start_new_cycle = true;

  pos = 0;
  digitalWrite(relay, HIGH);
  Serial.println("Ball catched!");

  while (pos >= -4150 && !check && start_new_cycle) {
    digitalWrite(enable_pin, HIGH);
    digitalWrite(motor_pin_1, LOW);
    digitalWrite(motor_pin_2, HIGH);
    Serial.println(pos);
  }

  pos = 0;
  digitalWrite(enable_pin, LOW);
  digitalWrite(motor_pin_1, LOW);
  digitalWrite(motor_pin_2, LOW);

  if (pos >= -4150) {
    Serial.println("Reached 1/4H position");
    check = true;
    delay(2000);
    digitalWrite(relay, LOW);
    delay(4000);
    check = true;
    catchBall();
    start_new_cycle = false;
  }
}

void one_and_a_half() {
  check = false;
  start_new_cycle = true;

  pos = 0;
  digitalWrite(relay, HIGH);
  Serial.println("Ball catched!");

  while (pos >= -8250 && !check && start_new_cycle) {
    digitalWrite(enable_pin, HIGH);
    digitalWrite(motor_pin_1, LOW);
    digitalWrite(motor_pin_2, HIGH);
    Serial.println(pos);
  }

  pos = 0;
  digitalWrite(enable_pin, LOW);
  digitalWrite(motor_pin_1, LOW);
  digitalWrite(motor_pin_2, LOW);

  if (pos >= -8250) {
    Serial.println("Reached 1/2H position");
    check = true;
    delay(2000);
    digitalWrite(relay, LOW);
    delay(4000);
    check = true;
    catchBall();
    start_new_cycle = false;
  }
}

void h() {
  check = false;
  start_new_cycle = true;

  pos = 0;
  digitalWrite(relay, HIGH);
  Serial.println("Ball catched!");

  while (pos >= -15000 && !check && start_new_cycle) {
    digitalWrite(enable_pin, HIGH);
    digitalWrite(motor_pin_1, LOW);
    digitalWrite(motor_pin_2, HIGH);
    Serial.println(pos);
  }

  pos = 0;
  digitalWrite(enable_pin, LOW);
  digitalWrite(motor_pin_1, LOW);
  digitalWrite(motor_pin_2, LOW);

  if (pos >= -15000) {
    Serial.println("Reached 1/2H position");
    check = true;
    delay(2000);
    digitalWrite(relay, LOW);
    delay(4000);
    check = true;
    catchBall();
    start_new_cycle = false;
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(motor_pin_1, OUTPUT);
  pinMode(motor_pin_2, OUTPUT);
  pinMode(enable_pin, OUTPUT);
  pinMode(relay, OUTPUT);
  pinMode(sensor_position_0, INPUT);
  pinMode(ENCA, INPUT);
  pinMode(ENCB, INPUT);

  digitalWrite(relay, LOW);

  attachInterrupt(digitalPinToInterrupt(ENCA), readEncoder, RISING);

  if(!SPIFFS.begin(true)){
    Serial.println("Problem z SPIFFS");
    return;
  }

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  Serial.println(WiFi.localIP());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", String(), false);
  });

  server.serveStatic("/images", SPIFFS, "/images");

  server.on("/heightInput", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->hasParam("value")) {
      targetPosition = request->getParam("value")->value().toInt();
      check = false;
      Serial.print("New target position: ");
      Serial.println(targetPosition);
    }
    request->send(200, "text/plain", "OK");
  });

  server.on("/returnToZero", HTTP_GET, [](AsyncWebServerRequest *request){
    targetPosition = 0;
    check = false;
    Serial.println("Returning to zero position");
    request->send(200, "text/plain", "OK");
  });

  server.on("/catchBall", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("Catching ball");
    request->send(200, "text/plain", "OK");
  });

  server.on("/setPosition", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->hasParam("value")) {
      targetPosition = request->getParam("value")->value().toInt();
      check = false;
      Serial.print("New target position: ");
      Serial.println(targetPosition);
    }
    request->send(200, "text/plain", "OK");
  });

  server.begin();
}

void show_actual_position() {
  Serial.print("Aktualna pozycja: ");
  Serial.println(pos);
  delay(1000);
}

void show_sensor_state() {
  Serial.print("Stan czujnika '0': ");
  Serial.println(digitalRead(sensor_position_0));
  Serial.println("");
  delay(1000);
}

void loop() {
  if (targetPosition != 0 && targetPosition >= -15000 && targetPosition <= 16500 && check == false) {
    
    if (targetPosition == 2500) {
      one_and_a_quarter();
      delay(5000);
      catchBall();
    }

    if (targetPosition == 5000) {
      one_and_a_half();
      delay(5000);
      catchBall();
    }

    if (targetPosition == 5000) {
      h();
      delay(5000);
      catchBall();
    }

  }
}