#include <Servo.h>

const int PWM_PIN   = 5;
const int IN1_PIN   = 8;
const int IN2_PIN   = 9;
const int ENCODER_A = 2;
const int ENCODER_B = 3;
const int SERVO_PIN = 11;
const int LIMIT_TOP = 4;

// ===== Positions =====
long floors[]         = {0, 2363, 4610, 7100, 9450};
const int totalFloors = 5;

// ===== Encoder =====
volatile long encoderCount = 0;

// ===== Control =====
int motorSpeed = 150;
int tolerance  = 50;

// ===== Servo =====
Servo doorServo;
const int DOOR_OPEN_ANGLE   = 90;
const int DOOR_CLOSED_ANGLE = 0;

// ===== State Machine =====
int  currentFloorIndex = 0;
int  targetFloorIndex  = 0;      // ← الطابق المطلوب من الكيبورد
bool movingUp          = true;
bool elevatorBusy      = false;  // ← true أثناء الحركة أو فتح/غلق الباب

unsigned long stopTimer  = 0;
unsigned long closeTimer = 0;
bool waiting     = false;
bool doorOpened  = false;
bool doorClosing = false;

// ===== Serial throttle =====
unsigned long lastPrintTime = 0;

// ===== ISR =====
void encoderISR() {
  if (digitalRead(ENCODER_B) == HIGH) encoderCount++;
  else                                  encoderCount--;
}

// ===== Safe encoder helpers =====
long readEncoder() {
  noInterrupts();
  long val = encoderCount;
  interrupts();
  return val;
}

void writeEncoder(long val) {
  noInterrupts();
  encoderCount = val;
  interrupts();
}

// ===== Motor =====
void setMotor(int speed, bool dir) {
  speed = constrain(speed, 0, 255);
  digitalWrite(IN1_PIN, dir ? HIGH : LOW);
  digitalWrite(IN2_PIN, dir ? LOW  : HIGH);
  analogWrite(PWM_PIN, speed);
}

void stopMotor() {
  analogWrite(PWM_PIN, 0);
  digitalWrite(IN1_PIN, LOW);
  digitalWrite(IN2_PIN, LOW);
}

// ===== Door =====
void openDoor()  { doorServo.write(DOOR_OPEN_ANGLE);   Serial.println("Door OPEN");   }
void closeDoor() { doorServo.write(DOOR_CLOSED_ANGLE); Serial.println("Door CLOSED"); }

// ===== Print status =====
void printStatus() {
  Serial.println("-------------------------------");
  Serial.print  ("Current floor : "); Serial.println(currentFloorIndex + 1);
  Serial.print  ("Target  floor : "); Serial.println(targetFloorIndex  + 1);
  Serial.print  ("Encoder pos   : "); Serial.println(readEncoder());
  Serial.print  ("Busy          : "); Serial.println(elevatorBusy ? "YES" : "NO");
  Serial.println("-------------------------------");
  Serial.println("Press 1-5 to call a floor.");
}

// ===== Homing =====
void homing() {
  Serial.println("Homing...");
  detachInterrupt(digitalPinToInterrupt(ENCODER_A));

  long pos = readEncoder();
  while (abs(pos) > tolerance) {
    bool driveDown = (pos > 0);
    setMotor(120, !driveDown);
    pos = readEncoder();
  }

  stopMotor();
  writeEncoder(0);
  currentFloorIndex = 0;
  targetFloorIndex  = 0;
  movingUp          = true;
  elevatorBusy      = false;

  attachInterrupt(digitalPinToInterrupt(ENCODER_A), encoderISR, RISING);

  Serial.println("Homing Done. Elevator at Floor 1.");
  unsigned long t = millis();
  while (millis() - t < 500) {}
}

// ===== Limit Switch =====
void handleLimitSwitch() {
  stopMotor();
  writeEncoder(floors[totalFloors - 1]);
  currentFloorIndex = totalFloors - 1;
  targetFloorIndex  = totalFloors - 1;
  movingUp          = false;
  waiting           = false;
  doorOpened        = false;
  doorClosing       = false;
  elevatorBusy      = false;
  Serial.println("LIMIT HIT! Stopped at top.");
}

// ===== Keyboard Input =====
void handleKeyboard() {
  if (!Serial.available()) return;

  char key = Serial.read();

  // تجاهل newline و carriage return
  if (key == '\n' || key == '\r') return;

  // أرقام 1 إلى 5 فقط
  if (key >= '1' && key <= '5') {
    int requestedFloor = key - '1';   // تحويل الحرف لـ index (0-4)

    if (elevatorBusy) {
      Serial.print("Elevator busy! Request queued for floor: ");
      Serial.println(requestedFloor + 1);
      // نحفظ الطلب — سيُنفَّذ بعد انتهاء الحركة الحالية
      targetFloorIndex = requestedFloor;
      return;
    }

    if (requestedFloor == currentFloorIndex) {
      Serial.print("Already at floor ");
      Serial.println(requestedFloor + 1);
      // افتح الباب فقط
      openDoor();
      doorOpened  = true;
      doorClosing = false;
      waiting     = true;
      elevatorBusy = true;
      stopTimer   = millis();
      return;
    }

    targetFloorIndex = requestedFloor;
    movingUp         = (targetFloorIndex > currentFloorIndex);
    elevatorBusy     = true;
    waiting          = false;
    doorOpened       = false;
    doorClosing      = false;

    Serial.print("Going to floor: ");
    Serial.println(targetFloorIndex + 1);

  } else {
    // مفتاح غير معروف
    Serial.print("Unknown key '");
    Serial.print(key);
    Serial.println("'. Press 1-5 to select a floor.");
  }
}

// ===== Setup =====
void setup() {
  Serial.begin(115200);

  pinMode(PWM_PIN,   OUTPUT);
  pinMode(IN1_PIN,   OUTPUT);
  pinMode(IN2_PIN,   OUTPUT);
  pinMode(ENCODER_A, INPUT_PULLUP);
  pinMode(ENCODER_B, INPUT_PULLUP);
  pinMode(LIMIT_TOP, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(ENCODER_A), encoderISR, RISING);

  doorServo.attach(SERVO_PIN);
  closeDoor();

  homing();

  Serial.println("=== MANUAL MODE ===");
  Serial.println("Press 1-5 to select a floor.");
}

// ===== Loop =====
void loop() {

  // --- Limit switch ---
  if (digitalRead(LIMIT_TOP) == LOW) {
    handleLimitSwitch();
    delay(200);
    return;
  }

  // --- قراءة الكيبورد دايماً ---
  handleKeyboard();

  // --- لو الاسانسير مش busy، استنى أوامر ---
  if (!elevatorBusy) return;

  // ===================================================
  //  State machine — تُنفَّذ فقط لما elevatorBusy = true
  // ===================================================
  long currentPos = readEncoder();
  long target     = floors[targetFloorIndex];
  long error      = target - currentPos;

  // --- وصلنا للطابق المطلوب ---
  if (abs(error) < tolerance) {
    stopMotor();

    if (!waiting) {
      currentFloorIndex = targetFloorIndex;
      Serial.print("Arrived at floor: ");
      Serial.println(currentFloorIndex + 1);
      openDoor();
      doorOpened  = true;
      doorClosing = false;
      waiting     = true;
      stopTimer   = millis();
    }

    // بعد 1 ثانية اغلق الباب
    if (millis() - stopTimer >= 1000 && !doorClosing && doorOpened) {
      closeDoor();
      doorOpened  = false;
      doorClosing = true;
      closeTimer  = millis();
    }

    // بعد 400ms من الغلق — انتهى كل شيء
    if (doorClosing && millis() - closeTimer >= 400) {
      doorClosing  = false;
      waiting      = false;
      elevatorBusy = false;   // ← حرر الاسانسير

      Serial.print("Floor ");
      Serial.print(currentFloorIndex + 1);
      Serial.println(" ready. Press 1-5 for next floor.");
    }

    return;
  }

  // --- لسه بيتحرك ---
  setMotor(motorSpeed, error > 0);

  if (millis() - lastPrintTime >= 200) {
    lastPrintTime = millis();
    Serial.print("Target: "); Serial.print(target);
    Serial.print(" | Pos: "); Serial.print(currentPos);
    Serial.print(" | Error: "); Serial.println(error);
  }
}
