#include <ESP32Servo.h>  // Library to control servo motor on ESP32

// ── Pin Definitions ──────────────────────────────────────────
#define MQ6_DO_PIN    35  // MQ-6 digital output → GPIO35
#define BUZZER_PIN    25  // Active buzzer → GPIO25
#define RED_LED       26  // Red LED (gas detected) → GPIO26
#define GREEN_LED     14  // Green LED (clear air) → GPIO14
#define SERVO_PIN     18  // Servo signal wire → GPIO18
#define RESET_BUTTON  27  // Tactile reset button → GPIO27

Servo m1;                      // Create servo object
bool alarmLatched = false;     // false = clear, true = gas detected & locked

void setup() {
  Serial.begin(115200);                        // Start serial monitor at 115200 baud

  pinMode(MQ6_DO_PIN, INPUT);                  // MQ-6 DO pin as input
  pinMode(BUZZER_PIN, OUTPUT);                 // Buzzer as output
  pinMode(RED_LED, OUTPUT);                    // Red LED as output
  pinMode(GREEN_LED, OUTPUT);                  // Green LED as output
  pinMode(RESET_BUTTON, INPUT_PULLUP);         // Button input, internal pullup (no resistor needed)

  digitalWrite(BUZZER_PIN, LOW);               // Buzzer OFF at start
  digitalWrite(RED_LED, LOW);                  // Red LED OFF at start
  digitalWrite(GREEN_LED, HIGH);               // Green LED ON at start (air is clear)

  m1.attach(SERVO_PIN);                        // Attach servo to GPIO18
  m1.write(0);                                 // Servo starts at 0° (regulator open)
  Serial.println("Ready!");                    // Print ready message on serial monitor
}

void loop() {
  int gasState    = digitalRead(MQ6_DO_PIN);   // Read MQ-6 → LOW = gas, HIGH = clear
  int buttonState = digitalRead(RESET_BUTTON); // Read button → LOW = pressed, HIGH = not pressed

  // ── Gas Detection ─────────────────────────────────────────
  if (gasState == LOW) {                       // If MQ-6 detects gas
    alarmLatched = true;                       // Lock the alarm ON (stays ON even if gas removed)
  }

  // ── Manual Reset ──────────────────────────────────────────
  if (buttonState == LOW && alarmLatched && gasState == HIGH) {
    // Button pressed + alarm was latched + gas is now gone → safe to reset
    alarmLatched = false;                      // Unlock the alarm
    Serial.println("Reset pressed - Regulator opening!");
  }

  // ── Output Control ────────────────────────────────────────
  if (alarmLatched) {
    // ALARM STATE → gas was detected
    digitalWrite(BUZZER_PIN, HIGH);            // Buzzer ON
    digitalWrite(RED_LED, HIGH);               // Red LED ON
    digitalWrite(GREEN_LED, LOW);              // Green LED OFF
    m1.write(95);                              // Servo moves to 95° → regulator CLOSED
    Serial.println("GAS DETECTED");
  } else {
    // CLEAR STATE → no gas / after reset
    digitalWrite(BUZZER_PIN, LOW);             // Buzzer OFF
    digitalWrite(RED_LED, LOW);                // Red LED OFF
    digitalWrite(GREEN_LED, HIGH);             // Green LED ON
    m1.write(0);                               // Servo stays at 0° → regulator OPEN
    Serial.println("Clear");
  }

  delay(300);  // Wait 300ms before next reading (prevents rapid repeated triggers)
}
