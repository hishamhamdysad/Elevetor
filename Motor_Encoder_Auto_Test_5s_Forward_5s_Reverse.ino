// Motor + Encoder Auto Test (5s Forward / 5s Reverse)

const int PWM_PIN = 10;
const int IN1_PIN = 8;
const int IN2_PIN = 9;

const int ENCODER_A = 2;
const int ENCODER_B = 3;

const int PULSES_PER_REV = 20;

volatile long encoderCount = 0;

unsigned long lastTime = 0;
long lastCount = 0;

unsigned long motorTimer = 0;

int motorPwm = 150;
bool motorDir = true; // forward

void encoderISR() {
  int a = digitalRead(ENCODER_A);
  int b = digitalRead(ENCODER_B);

  if (a == b) {
    encoderCount++;
  } else {
    encoderCount--;
  }
}

void setMotor(int pwmValue, bool dir) {
  pwmValue = constrain(pwmValue, 0, 255);

  if (dir) {
    digitalWrite(IN1_PIN, HIGH);
    digitalWrite(IN2_PIN, LOW);
  } else {
    digitalWrite(IN1_PIN, LOW);
    digitalWrite(IN2_PIN, HIGH);
  }

  analogWrite(PWM_PIN, pwmValue);
}

void stopMotor() {
  analogWrite(PWM_PIN, 0);
  digitalWrite(IN1_PIN, LOW);
  digitalWrite(IN2_PIN, LOW);
}

void setup() {
  Serial.begin(115200);

  pinMode(PWM_PIN, OUTPUT);
  pinMode(IN1_PIN, OUTPUT);
  pinMode(IN2_PIN, OUTPUT);

  pinMode(ENCODER_A, INPUT_PULLUP);
  pinMode(ENCODER_B, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(ENCODER_A), encoderISR, CHANGE);

  Serial.println("Auto Motor Test (5s F / 5s R)");

  setMotor(motorPwm, motorDir);
  motorTimer = millis();
  lastTime = millis();
}

void loop() {
  unsigned long now = millis();

  // تغيير الاتجاه كل 5 ثواني
  if (now - motorTimer >= 5000) {
    motorDir = !motorDir; // عكس الاتجاه
    setMotor(motorPwm, motorDir);
    motorTimer = now;
  }

  // حساب السرعة
  if (now - lastTime >= 500) {
    noInterrupts();
    long currentCount = encoderCount;
    interrupts();

    long deltaCount = currentCount - lastCount;
    float dt = (now - lastTime) / 1000.0;

    float revs = (float)deltaCount / PULSES_PER_REV;
    float rpm = (revs / dt) * 60.0;

    Serial.print("Dir: ");
    Serial.print(motorDir ? "FWD" : "REV");
    Serial.print(" | PWM: ");
    Serial.print(motorPwm);
    Serial.print(" | Count: ");
    Serial.print(currentCount);
    Serial.print(" | RPM: ");
    Serial.println(rpm);

    lastCount = currentCount;
    lastTime = now;
  }
}