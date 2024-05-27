// #define motor_pin_1 11 {
// #define motor_pin_2 10

// #define enable_pin 4
// #define button 12  
                        // PINY DO ARDUINO
// #define ENCA 2
// #define ENCB 3

// #define relay 5 }

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

//piny esp32v
#define motor_pin_1 19 
#define motor_pin_2 18

#define enable_pin 22
#define button 23

#define ENCA 15
#define ENCB 2

#define relay 4 

//z gory do dolu pozycja enkodera (kY-) -23000
//z dolu do gory pozycja enkdora (kY+) 18500

const char* ssid = "REPLACE_WITH_YOUR_SSID";
const char* password = "REPLACE_WITH_YOUR_PASSWORD";

AsyncWebServer server(80);

int pos = 0;
bool button_is_pressed = false;

const char index_html[] PROGMEM = R"rawliteral(
  <!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title></title>
    <!-- <link rel="stylesheet" href="style.css"> -->

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
            <img src="../src/images/myscreen-logo.webp">
            <form id="controlForm">
                <input type="text" id="heightInput" name="heightInput" placeholder="Wprowadź wartość">
                <button id="accept_button" type="button" onclick="return_value()">Zatwierdź</button>
            </form>
            <div class="button_return_to_0">
                <button class="button_return_to_0_level">Powrót do pozycji 0</button>
            </div>
        </div>
    </div>
    
    <script>
        function return_value() {
            var heightInputValue = document.getElementById('heightInput').value;
            console.log(heightInputValue);
        }
        document.getElementById('controlForm').addEventListener('submit', function(event) {
            event.preventDefault();
            return_value();
        });
    </script>
        
</body>
</html>
)rawliteral";

String processor(const String& var){
  Serial.println(var);
  if (var == "heightInputValue"){
    return heightInputValue;
  }
  return String();
}

void setup() {

  Serial.begin(9600);

  pinMode(motor_pin_1, OUTPUT);
  pinMode(motor_pin_2, OUTPUT);

  pinMode(enable_pin, OUTPUT);
  pinMode(relay, OUTPUT);

  pinMode(button, INPUT_PULLUP);

  pinMode(ENCA, INPUT);
  pinMode(ENCB, INPUT);

  // digitalWrite(enable_pin, HIGH);
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

}

void loop() {

  if (digitalRead(button) == LOW) {
    button_is_pressed = true;
    if (button_is_pressed == true) {
      Serial.println("Wcisnieto przycisk");
    }
  }

  magnes_z_gory_do_dolu();

  delay(5000);

  magnes_z_dolu_do_gory();

}

void readEncoder(){

  int b = digitalRead(ENCB);

  if (b>0) {
    pos++;
  }
  else {
    pos--;
  }
}

void magnes_z_gory_do_dolu() { //funkcja dzialala  w tym stanie /nw czemu (ale podejrzewam) mierzylismy to 18000 jednostke to z gory do dolu a faktycznie wyszlo 15000

  while (button_is_pressed == true) {

    Serial.println(pos);

    //if (pos >= -5) {
      while (pos >= -5) { //zmienione z 5 jakby sie w ogole zesralo zmieni na 100 z powrotem
        digitalWrite(enable_pin, HIGH);
        digitalWrite(motor_pin_1, HIGH);
        digitalWrite(motor_pin_2, LOW);
        Serial.println(pos);
      } 
      
      if (pos <= -15500) { //
        digitalWrite(enable_pin, LOW);
        digitalWrite(motor_pin_1, LOW);
        digitalWrite(motor_pin_2, LOW);
        digitalWrite(relay, LOW);
        // button_is_pressed = false;
        Serial.println("Osiagnieto maksymalna pozycje");
        pos = 0;
        break;
      }
  }
}

void magnes_z_dolu_do_gory() {

  pos = 0;

  while (button_is_pressed == true) {

    Serial.println(pos);

    // digitalWrite(enable_pin, LOW); //wydupcyc to jakby nie dzialalo
    
    //if (pos >= -5) {
    Serial.println("pos >= 0");
    while (pos >= 0) {
      digitalWrite(enable_pin, HIGH);
      digitalWrite(motor_pin_2, HIGH);
      digitalWrite(motor_pin_1, LOW);
      Serial.println(pos); 

      if (pos >= 16500) {
        digitalWrite(enable_pin, LOW);
        digitalWrite(motor_pin_1, LOW);
        digitalWrite(motor_pin_2, LOW);
        button_is_pressed = false;
        digitalWrite(enable_pin, LOW);
        Serial.println("Osiagnieto maksymalna pozycje");
        delay(3000);
        digitalWrite(relay, HIGH);
        break;
      }
    }
  }
}