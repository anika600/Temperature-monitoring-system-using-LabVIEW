const int SENSOR_PIN = A0;
const int BUZZER_PIN = 8;
const int LED_PIN    = 7;

void setup() {
  Serial.begin(9600);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  Serial.println(analogRead(SENSOR_PIN));   // e.g. "48" + CR/LF
  while (Serial.available() > 0) {
    char c = Serial.read();
    if (c == '1') { digitalWrite(BUZZER_PIN, HIGH); digitalWrite(LED_PIN, HIGH); }
    if (c == '0') { digitalWrite(BUZZER_PIN, LOW);  digitalWrite(LED_PIN, LOW);  }
  }
  delay(100);   // about 10 samples per second
}
