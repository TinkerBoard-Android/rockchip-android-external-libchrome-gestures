// Copyright 2021 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "gestures/include/haptic_button_generator_filter_interpreter.h"

#include <math.h>

#include "gestures/include/gestures.h"
#include "gestures/include/interpreter.h"
#include "gestures/include/logging.h"
#include "gestures/include/tracer.h"

namespace gestures {

HapticButtonGeneratorFilterInterpreter::HapticButtonGeneratorFilterInterpreter(
    PropRegistry* prop_reg, Interpreter* next, Tracer* tracer)
    : FilterInterpreter(NULL, next, tracer, false),
      sensitivity_(prop_reg, "Haptic Button Sensitivity", 3),
      use_custom_thresholds_(prop_reg,
                             "Use Custom Haptic Button Force Thresholds",
                             0),
      custom_down_threshold_(prop_reg,
                             "Custom Haptic Button Force Threshold Down",
                             150.0),
      custom_up_threshold_(prop_reg,
                            "Custom Haptic Button Force Threshold Up",
                            130.0),
      enabled_(prop_reg, "Enable Haptic Button Generation", false),
      force_scale_(prop_reg, "Force Calibration Slope", 1.0),
      force_translate_(prop_reg, "Force Calibration Offset", 0.0),
      button_down_(false) {
  InitName();
}

void HapticButtonGeneratorFilterInterpreter::Initialize(
    const HardwareProperties* hwprops,
    Metrics* metrics,
    MetricsProperties* mprops,
    GestureConsumer* consumer) {
  is_haptic_pad_ = hwprops->is_haptic_pad;
  FilterInterpreter::Initialize(hwprops, NULL, mprops, consumer);
}

void HapticButtonGeneratorFilterInterpreter::SyncInterpretImpl(
    HardwareState* hwstate, stime_t* timeout) {
  HandleHardwareState(hwstate);
  next_->SyncInterpret(hwstate, timeout);
}

void HapticButtonGeneratorFilterInterpreter::HandleHardwareState(
    HardwareState* hwstate) {
  if (!enabled_.val_ || !is_haptic_pad_)
    return;

  // Ignore the button state generated by the haptic touchpad.
  hwstate->buttons_down = 0;

  // Determine force thresholds.
  double down_threshold;
  double up_threshold;
  if (use_custom_thresholds_.val_) {
    down_threshold = custom_down_threshold_.val_;
    up_threshold = custom_up_threshold_.val_;
  }
  else {
    down_threshold = down_thresholds_[sensitivity_.val_ - 1];
    up_threshold = up_thresholds_[sensitivity_.val_ - 1];
  }

  // Determine total force on touchpad in grams
  double force = 0.0;
  for (short i = 0; i < hwstate->finger_cnt; i++) {
    force = fmax(force, hwstate->fingers[i].pressure);
  }
  force *= force_scale_.val_;
  force += force_translate_.val_;

  // Set the button state
  if (button_down_) {
    if (force < up_threshold)
      button_down_ = false;
    else
      hwstate->buttons_down = GESTURES_BUTTON_LEFT;
  }
  else if (force > down_threshold) {
    button_down_ = true;
    hwstate->buttons_down = GESTURES_BUTTON_LEFT;
  }
}

}  // namespace gestures
