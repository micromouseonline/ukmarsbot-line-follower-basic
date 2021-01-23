
#include "motion.h"
#include <arduino.h>
#include "defaults.h"
#include "encoders.h"
#include "motors.h"
#include "settings.h"
#include "profile.h"

/***
 * Speeds are used for feedback because the position is changed at the start
 * of each profile
 */
Profile fwd(&encoderSpeed);
Profile rot(&encoderOmega);

void Profile::reset() {
  noInterrupts();
  mCurrentSpeed = 0;
  mTargetSpeed = 0;
  mPosition = 0;
  mAdjustment = 0;
  mError = 0;
  mOldError = 0;
  mState = FINISHED;
  interrupts();
}

void Profile::startMove(float distance, float topSpeed, float endSpeed, float acceleration) {
  int8_t sign = 1;
  if (distance < 0) {
    distance = -distance;
    sign = -1;
  }
  if (distance < 1.0) {
    mState = FINISHED;
    return;
  }
  if (endSpeed > topSpeed) {
    endSpeed = topSpeed;
  }
  noInterrupts();
  mDirection = sign;
  mPosition = 0;
  mEndPosition = distance;
  mTargetSpeed = sign * fabsf(topSpeed);
  mEndSpeed = sign * fabsf(endSpeed);
  mAcceleration = fabsf(acceleration);
  mState = ACCELERATE;
  interrupts();
}

void Profile::move(float distance, float topSpeed, float endSpeed, float acceleration) {
  startMove(distance, topSpeed, endSpeed, acceleration);
  while (!isFinished()) {
    // do nothing
  }
}

bool Profile::isFinished() {
  return mState == FINISHED;
}

float Profile::getBraking(float distance, float speed, float endspeed) {
  speed = fabsf(speed);
  endspeed = fabsf(endspeed);
  if (endspeed > speed) {
    return 0;
  }
  distance = fabsf(distance);
  float result = ((speed * speed) - (endspeed * endspeed)) / (2 * distance);
  return result;
}

void Profile::adjust(float adjustment) {
  if (motionEnabled) {
    mAdjustment = adjustment;
  }
}

void Profile::update() {
  if (functionButtonPressed()) {
    mState = FINISHED;
  }
  ///////
  if (mState == ACCELERATE) {
    float deceleration = getBraking(fabsf(mEndPosition) - fabsf(mPosition), mCurrentSpeed, mEndSpeed);
    if (deceleration >= mAcceleration) {
      mState = BRAKE;
      mTargetSpeed = mEndSpeed;
      if (fabsf(mEndSpeed) < mAcceleration * LOOP_INTERVAL) {
        mTargetSpeed = mDirection * mAcceleration * LOOP_INTERVAL;
      }
    }
  }
  /////////
  mCurrentSpeed += mAdjustment;
  /////////
  if (mCurrentSpeed < mTargetSpeed) {
    mCurrentSpeed += mAcceleration * LOOP_INTERVAL;
    if (mCurrentSpeed > mTargetSpeed) {
      mCurrentSpeed = mTargetSpeed;
    }
  } else if (mCurrentSpeed > mTargetSpeed) { 
    mCurrentSpeed -= mAcceleration * LOOP_INTERVAL;
    if (mCurrentSpeed < mTargetSpeed) {
      mCurrentSpeed = mTargetSpeed;
    }
  }
  ///////////
  mPosition += (mCurrentSpeed)*LOOP_INTERVAL;
  ///////////
  if (mState == BRAKE) {
    if (fabsf(mCurrentSpeed) - fabsf(mTargetSpeed) < 0.1) {
      mState = FINISHED;
      mCurrentSpeed = mEndSpeed;
    }
    if ((fabsf(mPosition)) >= fabsf(mEndPosition)) {
      mState = FINISHED;
      mCurrentSpeed = mEndSpeed;
    }
  }
}

void Profile::setMode(int mode) {
  if (mMode == mode) {
    return;
  }
  if (mode == AUTOMATIC) {
    // mError = 0;
    mOldError = mError;
  }
  mMode = mode;
}

void Profile::controllerUpdate() {
  float input = *mFeedback;
  mError += mCurrentSpeed * LOOP_INTERVAL;
  mError -= input * LOOP_INTERVAL;
  float dError = mError - mOldError;
  mOldError = mError;
  if (mMode == MANUAL) {
    return;
  }
  mControlOutput = mKP * mError + mKD * dError;
}