#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

// Piny ESP32
#define motor_pin_1 19 
#define motor_pin_2 18
#define enable_pin 22
#define button 23
#define ENCA 15
#define ENCB 2
#define relay 5

const char* ssid = "Lamel R&D 2.4GHz";
const char* password = "!lamel2024.";

AsyncWebServer server(80);

int pos = 0;
bool button_is_pressed = false;
bool rozjebie_cos = false;
volatile int targetPosition = 0;

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 Motor Control</title>
    <style>
    body {
        font-family: "Barlow", Sans-serif;
        font-weight: 500;
        margin: 0;
        padding: 0;
        display: flex;
        justify-content: center;
        align-items: center;
        height: 100vh;
        background-color: #f0f0f0;
    }
    .container {
        text-align: center;
    }
    img {
        max-width: 300px;
        margin-bottom: 20px;
    }
    #controlForm {
        margin-bottom: 20px;
        background-color: #ff0000;
    }

    #controlForm input[type="text"]::placeholder {
        color: #666;
        opacity: 1;
    }
    #controlForm button {
        font-family: "Barlow", Sans-serif;
        font-weight: 500;
        display: block; 
        margin: 0 auto; 
        margin-top: 5%;
        padding: 10px;
        height: 40px;
        border: none; 
        border-radius: 5px;
        background-color: #6b30c4; 
        color: white; 
        cursor: pointer; 
        transition: background-color 0.3s ease; 
    }

    #controlForm input[type="text"]::placeholder {
        color: #999; 
    }

    #controlForm button:hover {
        background-color: #4a1a7f; 
    }

    .button_return_to_0_level {
        font-family: "Barlow", Sans-serif;
        font-weight: 500;
        display: block; 
        margin: 0 auto; 
        margin-top: 5%;
        padding: 10px; 
        width: 200px;
        height: 40px;
        border: none;
        border-radius: 5px; 
        background-color: #6b30c4; 
        color: white; 
        cursor: pointer;
        transition: background-color 0.3s ease; 
    }

    .button_return_to_0_level:hover {
        background-color: #4a1a7f; 
    }

    </style>
</head>
<body>
    <div class="main-container">
        <div class="container">
            <img src="src/images/myscreen-logo.webp"> 
            <form id="controlForm">
                <input type="text" id="heightInput" name="heightInput" placeholder="Wprowadź wartość">
                <button id="accept_button" type="button" onclick="sendHeight()">Zatwierdź</button>
            </form>
            <div class="button_return_to_0">
                <button class="button_return_to_0_level" onclick="returnToZero()">Powrót do pozycji 0</button>
            </div>
        </div>
    </div>
    <script>
        function sendHeight() {
            var heightInputValue = document.getElementById('heightInput').value;
            var xhr = new XMLHttpRequest();
            xhr.open("GET", "/setHeight?value=" + heightInputValue, true);
            xhr.send();
        }

        function returnToZero() {
            var xhr = new XMLHttpRequest();
            xhr.open("GET", "/returnToZero", true);
            xhr.send();
        }
    </script>
</body>
</html>
)rawliteral";

String processor(const String& var){
  return String();
}

void setup() {
  Serial.begin(115200);

  pinMode(motor_pin_1, OUTPUT);
  pinMode(motor_pin_2, OUTPUT);
  pinMode(enable_pin, OUTPUT);
  pinMode(relay, OUTPUT);
  pinMode(button, INPUT_PULLUP);
  pinMode(ENCA, INPUT);
  pinMode(ENCB, INPUT);

  attachInterrupt(digitalPinToInterrupt(ENCA), readEncoder, RISING);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println(WiFi.localIP());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });

  server.on("/setHeight", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->hasParam("value")) {
      String heightValue = request->getParam("value")->value();
      targetPosition = heightValue.toInt();
      Serial.println("Target Position: " + String(targetPosition));
      request->send(200, "text/plain", "Height set to: " + heightValue);
    } else {
      request->send(200, "text/plain", "No value received");
    }
  });

  server.on("/returnToZero", HTTP_GET, [](AsyncWebServerRequest *request){
    targetPosition = 0;
    Serial.println("Returning to zero");
    request->send(200, "text/plain", "Returning to zero");
  });

  server.begin();
}

void readEncoder(){
  int b = digitalRead(ENCB);
  if (b > 0) {
    pos++;
  } else {
    pos--;
  }
}

void loop() {
  if (targetPosition != 0 && targetPosition >= -15000 && targetPosition <= 16500 && rozjebie_cos == false) {
    down();
    Serial.print("Silnik pracuje - pozycja: ");
    Serial.println(pos);
  }
}

void down() {
  if (pos > targetPosition) { // Jeśli aktualna pozycja enkodera jest większa niż wartość docelowa
    digitalWrite(enable_pin, HIGH); // Włącz silnik w dół
    digitalWrite(motor_pin_1, HIGH);
    digitalWrite(motor_pin_2, LOW);
  } else if (pos <= targetPosition) { // Jeśli aktualna pozycja enkodera osiągnęła lub przekroczyła wartość docelową
    digitalWrite(enable_pin, LOW); // Zatrzymaj silnik
    digitalWrite(motor_pin_1, LOW);
    digitalWrite(motor_pin_2, LOW);
    // digitalWrite(relay, LOW);
    Serial.println("Osiągnięto maksymalną pozycję w dół");
    pos = 0; // Zresetuj pozycję enkodera
  }

  rozjebie_cos = true;

}


//pozycja od "0" do dolu ~ -15500
//z dolu do gory pozycja enkdora (kY+) z dolu do gory ~ 16500 jakis jednostek
// todo: zeby wpisywac metry? ale trzeba policzyc co gdzie i jak wiadomix
