#include <Arduino.h>

// motor
const int MICROSTEPS = 8;
const int NATIVE_STEPS = 200;
const double STEPS_PER_ROTATION = NATIVE_STEPS * MICROSTEPS;
const double SPEED_FACTOR = 1.0;

// TMC2209 <────> Board
const int STEP_PIN = 5;
const int DIR_PIN = 6;
const int EN_PIN = 4;
const int BOOT_PIN = 0;

// geometry (mm)
const double RADIUS_UPPER = 282.5;
const double RADIUS_LOWER = 287.0;
const double MOUNT_DIST_UPPER = 45.0;
const double MOUNT_DIST_LOWER = 45.0;
const double THREAD_PITCH = 1.25;

const double ROTATION_EARTH = 7.2921e-5;

int taken_steps = 0;
int64_t time_start;
bool running = false;

double calc_rod_length(double theta) {
  double delta_x = RADIUS_UPPER * cos(theta) - MOUNT_DIST_UPPER * sin(theta) - RADIUS_LOWER;
  double delta_y = RADIUS_UPPER * sin(theta) + MOUNT_DIST_UPPER * cos(theta) + MOUNT_DIST_LOWER;
  return sqrt(delta_x*delta_x + delta_y*delta_y);
}

const double ROD_LENGTH_START = calc_rod_length(0.0);

double calc_steps(double t) {
  double angle = ROTATION_EARTH * t * SPEED_FACTOR;
  double rod_length = calc_rod_length(angle);
  double steps = (rod_length - ROD_LENGTH_START) / THREAD_PITCH * STEPS_PER_ROTATION;
  return steps;
}

void setup() {
  pinMode(EN_PIN, OUTPUT);
  digitalWrite(EN_PIN, LOW);

  pinMode(STEP_PIN, OUTPUT);
  digitalWrite(STEP_PIN, LOW);

  pinMode(DIR_PIN, OUTPUT);
  digitalWrite(DIR_PIN, HIGH);

  pinMode(BOOT_PIN, INPUT_PULLUP);
}


void loop() {
  if (!running && digitalRead(BOOT_PIN) == LOW) {
    running = true;
    taken_steps = 0;
    time_start = esp_timer_get_time();
  }

  if (!running) return;

  double elapsed_time = (esp_timer_get_time() - time_start) / 1000000.0;
  double calculated_steps = calc_steps(elapsed_time);
  if (taken_steps < calculated_steps) {
    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(STEP_PIN, LOW);
    taken_steps++;
  }
}