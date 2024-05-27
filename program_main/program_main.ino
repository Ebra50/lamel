#include <WiFi.h>
#include <SPIFFS.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

// Piny ESP32
#define motor_pin_1 19 
#define motor_pin_2 18
#define enable_pin 22
#define sensor_position_max 21
#define sensor_position_0 23
#define ENCA 15
#define ENCB 2
#define relay 5
#define fsrPin 34

// const char* ssid = "Lamel R&D 2.4GHz";
// const char* password = "!lamel2024.";

const char* ssid = "123";
const char* password = "cfel1234@";

AsyncWebServer server(80);

int pos = 0;

bool check = false;
bool go_for_ball = false;
bool checkbox_value = false;
bool start_new_cycle = true;
bool return_to_0_bool = false;
bool targetPosition_bool = true;

volatile int targetPosition = 0;

const float unitsPerCm = 165.0; // kazdy centymetr to 165 jednostek enkodera

void setup() {
  Serial.begin(115200);

  pinMode(ENCA, INPUT);
  pinMode(ENCB, INPUT);
  pinMode(relay, OUTPUT);
  pinMode(fsrPin, INPUT);
  pinMode(enable_pin, OUTPUT);
  pinMode(motor_pin_1, OUTPUT);
  pinMode(motor_pin_2, OUTPUT);
  pinMode(sensor_position_0, INPUT);
  pinMode(sensor_position_max, INPUT);

  digitalWrite(relay, LOW);

  attachInterrupt(digitalPinToInterrupt(ENCA), readEncoder, RISING);

  if (!SPIFFS.begin(true)) {
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
      setPosition(targetPosition);
      check = false;
      Serial.print("New target position: ");
      Serial.println(targetPosition);
    }
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

  server.on("/returnToZero", HTTP_GET, [](AsyncWebServerRequest *request){
    targetPosition = 0;
    check = false;
    return_to_0_bool = true;
    Serial.println("Returning to zero position");
    request->send(200, "text/plain", "OK");
  });

  server.on("/catchBall", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("Catching ball");
    check = false;
    go_for_ball = true;
    request->send(200, "text/plain", "OK");
  });

  server.on("/CheckBoxLoop", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->hasParam("value")) {
      String value = request->getParam("value")->value();
      checkbox_value = (value == "true");
      Serial.print("Checkbox value: ");
      Serial.println(checkbox_value ? "true" : "false");
    }
    request->send(200, "text/plain", "OK");
  });

  server.on("/getFSRValue", HTTP_GET, [](AsyncWebServerRequest *request){
    float fsrValue = readFSR();
    String response = String(fsrValue);
    request->send(200, "text/plain", response);
  });

  server.begin();
}

int cmToEncoderUnits(float cm) { //zamiana tych "jednostek" enkodera na cm aby wygodniej sie wpisywalo w panelu
  return (int)(cm * unitsPerCm);
}

void setPosition(float cm) {
  targetPosition = cmToEncoderUnits(cm);
  Serial.print("Setting position to: ");
  Serial.print(cm);
  Serial.print(" cm (");
  Serial.print(targetPosition);
  Serial.println(" units)");
  check = false;
  targetPosition_bool = true;
}

float readFSR() {
  int fsrReading = analogRead(fsrPin);
  float weightInGrams = map(fsrReading, 0, 4095, 0, 2000);

  Serial.print("FSR Reading: ");
  Serial.print(fsrReading);
  Serial.print(" / Weight: ");
  Serial.print(weightInGrams);
  Serial.println(" g");

  return weightInGrams;

  delay(1000);

}

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
    // Serial.println(pos);
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
  targetPosition_bool = false;

  pos = 0;
  digitalWrite(relay, HIGH);
  Serial.println("Ball catched!");

  while (pos >= -4150 && !check && start_new_cycle) {
    digitalWrite(enable_pin, HIGH);
    digitalWrite(motor_pin_1, LOW);
    digitalWrite(motor_pin_2, HIGH);
    // Serial.println(pos);
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
    delay(3000);

    float fsrValue = readFSR();
    Serial.print("FSR value: ");
    Serial.println(fsrValue);
    check = true;
    catchBall();
    start_new_cycle = false;
  }
}

void one_and_a_half() {
  check = false;
  start_new_cycle = true;
  targetPosition_bool = false;

  pos = 0;
  digitalWrite(relay, HIGH);
  Serial.println("Ball catched!");

  while (pos >= -8250 && !check && start_new_cycle) {
    digitalWrite(enable_pin, HIGH);
    digitalWrite(motor_pin_1, LOW);
    digitalWrite(motor_pin_2, HIGH);
    // Serial.println(pos);
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
    delay(3000);
    check = true;
    catchBall();
    start_new_cycle = false;
  }
}

void h() {
  check = false;
  start_new_cycle = true;
  targetPosition_bool = false;

  pos = 0;
  digitalWrite(relay, HIGH);
  Serial.println("Ball catched!");

  while (pos >= -15000 && !check && start_new_cycle) {
    digitalWrite(enable_pin, HIGH);
    digitalWrite(motor_pin_1, LOW);
    digitalWrite(motor_pin_2, HIGH);
    // Serial.println(pos);
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
    delay(3000);
    check = true;
    catchBall();
    start_new_cycle = false;
  }
}

void go_to_targetPosition() {
  check = false;
  start_new_cycle = true;

  pos = 0;
  digitalWrite(relay, HIGH);
  Serial.println("Ball catched!");

  while (pos >= -targetPosition && !check && start_new_cycle) {
    digitalWrite(enable_pin, HIGH);
    digitalWrite(motor_pin_1, LOW);
    digitalWrite(motor_pin_2, HIGH);
    // Serial.println(pos);
  }

  pos = 0;
  digitalWrite(enable_pin, LOW);
  digitalWrite(motor_pin_1, LOW);
  digitalWrite(motor_pin_2, LOW);

  if (pos >= -targetPosition) {
    Serial.println("Reached targeted position");
    check = true;
    delay(2000);
    digitalWrite(relay, LOW);
    check = true;
    // float fsrValue = readFSR(); // Read FSR value here
    // Serial.print("FSR value: ");
    // Serial.println(fsrValue);
    delay(3000);
    start_new_cycle = false;
  }
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
    while (checkbox_value == true) {
      if (targetPosition == 2500) {
        one_and_a_quarter();
        delay(5000);
        catchBall();
        targetPosition_bool = false;
      }

      if (targetPosition == 5000) {
        one_and_a_half();
        delay(5000);
        catchBall();
        targetPosition_bool = false;
      }

      if (targetPosition == 10000) {
        h();
        delay(5000);
        catchBall();
        targetPosition_bool = false;
      }

      if (targetPosition_bool == true) {
        go_to_targetPosition();
        delay(5000);
        catchBall();
      }
    }

    if (targetPosition == 2500) {
      one_and_a_quarter();
      delay(5000);
      catchBall();
      targetPosition_bool = false;
    }
    if (targetPosition == 5000) {
      one_and_a_half();
      delay(5000);
      catchBall();
      targetPosition_bool = false;
    }
    if (targetPosition == 10000) {
      h();
      delay(5000);
      catchBall();
      targetPosition_bool = false;
    }
    if (targetPosition_bool == true) {
      go_to_targetPosition();
      delay(5000);
      catchBall();
    }
  }
}
