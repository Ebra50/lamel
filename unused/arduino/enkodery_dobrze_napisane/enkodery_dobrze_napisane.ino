#define motor_pin_1 11
#define motor_pin_2 10

#define enable_pin 4
#define button 12

#define ENCA 2
#define ENCB 3

#define relay 5 

//z gory do dolu pozycja enkodera (kY-) -23000
//z dolu do gory pozycja enkdora (kY+) 18500

int pos = 0;
bool button_is_pressed = false;

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
      
      if (pos <= -14000) { //
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

      if (pos >= 15000) {
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