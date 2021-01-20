
#include "motors.h"
#include <arduino.h>
#include "settings.h"
#include "digitalWriteFast.h"

/****************************************************************************/
/*   MOTORS and PWM                                                         */
/****************************************************************************/

/**
 * Direct register access could be used here for enhanced performance
 */
void setLeftMotorPWM(int pwm) {
  pwm = MOTOR_LEFT_POLARITY * constrain(pwm, -255, 255);
  if (pwm < 0) {
    digitalWriteFast(MOTOR_LEFT_DIR, 1);
    analogWrite(MOTOR_LEFT_PWM, -pwm);
  } else {
    digitalWriteFast(MOTOR_LEFT_DIR, 0);
    analogWrite(MOTOR_LEFT_PWM, pwm);
  }
}

void setRightMotorPWM(int pwm) {
  pwm = MOTOR_RIGHT_POLARITY * constrain(pwm, -255, 255);
  if (pwm < 0) {
    digitalWriteFast(MOTOR_RIGHT_DIR, 1);
    analogWrite(MOTOR_RIGHT_PWM, -pwm);
  } else {
    digitalWriteFast(MOTOR_RIGHT_DIR, 0);
    analogWrite(MOTOR_RIGHT_PWM, pwm);
  }
}

void setLeftMotorVolts(float volts) {
  int motorPWM = (255 * volts) / batteryVolts;
  setLeftMotorPWM(motorPWM);
}

void setRightMotorVolts(float volts) {
  int motorPWM = (255 * volts) / batteryVolts;
  setRightMotorPWM(motorPWM);
}

void pwmSetup(int frequency = PWM_488_HZ) {
  switch (frequency) {
    case PWM_31_KHZ:
      // Divide by 1. frequency = 31.25 kHz;
      bitClear(TCCR1B, CS11);
      bitSet(TCCR1B, CS10);
      break;
    case PWM_3900_HZ:
      // Divide by 8. frequency = 3.91 kHz;
      bitSet(TCCR1B, CS11);
      bitClear(TCCR1B, CS10);
      break;
    case PWM_488_HZ:
    default:
      // Divide by 64. frequency = 488Hz;
      bitSet(TCCR1B, CS11);
      bitSet(TCCR1B, CS10);
      break;
  }
}

void motorSetup() {
  pinMode(MOTOR_LEFT_DIR, OUTPUT);
  pinMode(MOTOR_RIGHT_DIR, OUTPUT);
  pinMode(MOTOR_LEFT_PWM, OUTPUT);
  pinMode(MOTOR_RIGHT_PWM, OUTPUT);
  digitalWriteFast(MOTOR_LEFT_PWM, 0);
  digitalWriteFast(MOTOR_LEFT_DIR, 0);
  digitalWriteFast(MOTOR_RIGHT_PWM, 0);
  digitalWriteFast(MOTOR_RIGHT_DIR, 0);
  pwmSetup();
}

void motorBeep(int freq) {
  unsigned int delay = 1000000 / freq;
  int count = freq / 4;
  for (int i = 0; i < count; i++) {
    setLeftMotorVolts(MAX_MOTOR_VOLTS);
    setRightMotorVolts(MAX_MOTOR_VOLTS);
    delayMicroseconds(delay / 2);
    setLeftMotorVolts(-MAX_MOTOR_VOLTS);
    setRightMotorVolts(-MAX_MOTOR_VOLTS);
    delayMicroseconds(delay / 2);
  }
  setLeftMotorVolts(0);
  setRightMotorVolts(0);
}

/****************************************************************************/