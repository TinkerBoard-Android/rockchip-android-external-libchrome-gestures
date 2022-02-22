// Copyright 2021 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include "gestures/include/haptic_button_generator_filter_interpreter.h"
#include "gestures/include/unittest_util.h"

namespace gestures {

namespace {
class HapticButtonGeneratorFilterInterpreterTest : public ::testing::Test {};

class HapticButtonGeneratorFilterInterpreterTestInterpreter :
      public Interpreter {
 public:
  HapticButtonGeneratorFilterInterpreterTestInterpreter()
      : Interpreter(NULL, NULL, false) {}
  virtual void SyncInterpret(HardwareState* hwstate, stime_t* timeout) {
    if (return_value_.type != kGestureTypeNull)
      ProduceGesture(return_value_);
  }

  virtual void HandleTimer(stime_t now, stime_t* timeout) {
    ADD_FAILURE() << "HandleTimer on the next interpreter shouldn't be called";
  }

  Gesture return_value_;
};

struct GestureTestInputs {
  stime_t time;
  short touch_count; // -1 for timer callback
  FingerState* fs;
  Gesture gesture;
  stime_t expected_button;
};

} // namespace {}

TEST(HapticButtonGeneratorFilterInterpreterTest, SimpleTest) {
  HapticButtonGeneratorFilterInterpreterTestInterpreter* base_interpreter =
      new HapticButtonGeneratorFilterInterpreterTestInterpreter;
  HapticButtonGeneratorFilterInterpreter interpreter(
      NULL, base_interpreter, NULL);
  HardwareProperties hwprops = {
    0, 0, 100, 100,  // left, top, right, bottom
    10,  // x res (pixels/mm)
    10,  // y res (pixels/mm)
    133, 133,  // scrn DPI X, Y
    -1,  // orientation minimum
    2,   // orientation maximum
    2, 5,  // max fingers, max_touch
    0, 0, 0,  // t5r2, semi, button pad
    0, 0,  // has wheel, vertical wheel is high resolution
    1,  // haptic pad
  };
  TestInterpreterWrapper wrapper(&interpreter, &hwprops);

  interpreter.enabled_.val_ = true;

  FingerState fs[] = {
    // TM, Tm, WM, Wm, pr, orient, x, y, id, flag
    { 0, 0, 0, 0, 50, 0, 10, 1, 1, 0 },
    { 0, 0, 0, 0, 50, 0, 10, 1, 1, 0 },
    { 0, 0, 0, 0, 50, 0, 10, 1, 1, 0 },

    { 0, 0, 0, 0, 50, 0, 10, 1, 1, 0 },
    { 0, 0, 0, 0, 140, 0, 10, 1, 1, 0 },
    { 0, 0, 0, 0, 160, 0, 10, 1, 1, 0 },

    { 0, 0, 0, 0, 160, 0, 10, 1, 1, 0 },
    { 0, 0, 0, 0, 140, 0, 10, 1, 1, 0 },
    { 0, 0, 0, 0, 50, 0, 10, 1, 1, 0 },

    { 0, 0, 0, 0, 50, 0, 10, 1, 1, 0 },
    { 0, 0, 0, 0, 50, 0, 10, 1, 2, 0 },
    { 0, 0, 0, 0, 80, 0, 10, 1, 1, 0 },
    { 0, 0, 0, 0, 80, 0, 10, 1, 2, 0 },

    { 0, 0, 0, 0, 160, 0, 10, 1, 1, 0 },
    { 0, 0, 0, 0, 80, 0, 10, 1, 2, 0 },
  };
  HardwareState hs[] = {
    // Expect to remove button press generated by firmare
    make_hwstate(1.01, 0, 1, 1, &fs[0]),
    make_hwstate(1.02, GESTURES_BUTTON_LEFT, 1, 1, &fs[1]),
    make_hwstate(1.03, 0, 1, 1, &fs[2]),

    // Expect to set button down when going above 'down force threshold' (150)
    make_hwstate(2.01, 0, 1, 1, &fs[3]),
    make_hwstate(2.03, 0, 1, 1, &fs[4]),
    make_hwstate(2.05, 0, 1, 1, &fs[5]),

    // Expect to set button up when going below 'up force threshold' (130)
    make_hwstate(3.01, 0, 1, 1, &fs[6]),
    make_hwstate(3.03, 0, 1, 1, &fs[7]),
    make_hwstate(3.05, 0, 1, 1, &fs[8]),

    // Expect not to set button down when no individual finger goes above the
    // 'down force threshold' (150), even if multiple combined do
    make_hwstate(4.01, 0, 2, 2, &fs[9]),
    make_hwstate(4.03, 0, 2, 2, &fs[11]),

    // Expect to set button down when one of multiple fingers goes above the
    // 'down force threshold'
    make_hwstate(4.05, 0, 2, 2, &fs[13]),

    // Expect to set button up after all fingers leave
    make_hwstate(5.01, 0, 0, 0, NULL),
  };

  stime_t expected_buttons[] = {
    0, 0, 0,
    0, 0, GESTURES_BUTTON_LEFT,
    GESTURES_BUTTON_LEFT, GESTURES_BUTTON_LEFT, 0,
    0, 0,
    GESTURES_BUTTON_LEFT,
    0
  };

  for (size_t i = 0; i < arraysize(hs); i++) {
    stime_t timeout = NO_DEADLINE;
    wrapper.SyncInterpret(&hs[i], &timeout);
    EXPECT_EQ(hs[i].buttons_down, expected_buttons[i]);
  }
}

TEST(HapticButtonGeneratorFilterInterpreterTest, NotHapticTest) {
  HapticButtonGeneratorFilterInterpreterTestInterpreter* base_interpreter =
      new HapticButtonGeneratorFilterInterpreterTestInterpreter;
  HapticButtonGeneratorFilterInterpreter interpreter(
      NULL, base_interpreter, NULL);
  HardwareProperties hwprops = {
    0, 0, 100, 100,  // left, top, right, bottom
    10,  // x res (pixels/mm)
    10,  // y res (pixels/mm)
    133, 133,  // scrn DPI X, Y
    -1,  // orientation minimum
    2,   // orientation maximum
    2, 5,  // max fingers, max_touch
    0, 0, 0,  // t5r2, semi, button pad
    0, 0,  // has wheel, vertical wheel is high resolution
    0,  // haptic pad
  };
  TestInterpreterWrapper wrapper(&interpreter, &hwprops);

  interpreter.enabled_.val_ = true;

  FingerState fs[] = {
    // TM, Tm, WM, Wm, pr, orient, x, y, id, flag
    { 0, 0, 0, 0, 50, 0, 10, 1, 1, 0 },
    { 0, 0, 0, 0, 50, 0, 10, 1, 1, 0 },
    { 0, 0, 0, 0, 50, 0, 10, 1, 1, 0 },

    { 0, 0, 0, 0, 50, 0, 10, 1, 1, 0 },
    { 0, 0, 0, 0, 140, 0, 10, 1, 1, 0 },
    { 0, 0, 0, 0, 160, 0, 10, 1, 1, 0 },
  };
  HardwareState hs[] = {
    // Expect to keep button press generated by firmware
    make_hwstate(1.01, 0, 1, 1, &fs[0]),
    make_hwstate(1.02, GESTURES_BUTTON_LEFT, 1, 1, &fs[1]),
    make_hwstate(1.03, 0, 1, 1, &fs[2]),

    // Expect to not generate a button pres
    make_hwstate(2.01, 0, 1, 1, &fs[3]),
    make_hwstate(2.03, 0, 1, 1, &fs[4]),
    make_hwstate(2.05, 0, 1, 1, &fs[5]),
  };

  stime_t expected_buttons[] = {
    0, GESTURES_BUTTON_LEFT, 0,
    0, 0, 0
  };

  for (size_t i = 0; i < arraysize(hs); i++) {
    stime_t timeout = NO_DEADLINE;
    wrapper.SyncInterpret(&hs[i], &timeout);
    EXPECT_EQ(hs[i].buttons_down, expected_buttons[i]);
  }
}

TEST(HapticButtonGeneratorFilterInterpreterTest,
     GesturePreventsButtonDownTest) {
  HapticButtonGeneratorFilterInterpreterTestInterpreter* base_interpreter =
      new HapticButtonGeneratorFilterInterpreterTestInterpreter;
  HapticButtonGeneratorFilterInterpreter interpreter(
      NULL, base_interpreter, NULL);
  HardwareProperties hwprops = {
    0, 0, 100, 100,  // left, top, right, bottom
    10,  // x res (pixels/mm)
    10,  // y res (pixels/mm)
    133, 133,  // scrn DPI X, Y
    -1,  // orientation minimum
    2,   // orientation maximum
    2, 5,  // max fingers, max_touch
    0, 0, 0,  // t5r2, semi, button pad
    0, 0,  // has wheel, vertical wheel is high resolution
    1,  // haptic pad
  };
  TestInterpreterWrapper wrapper(&interpreter, &hwprops);

  interpreter.enabled_.val_ = true;

  // TM, Tm, WM, Wm, pr, orient, x, y, id, flag
  FingerState fs_low_force[] = {
    { 0, 0, 0, 0, 50, 0, 10, 1, 1, 0 },
    { 0, 0, 0, 0, 50, 0, 10, 1, 2, 0 },
  };
  FingerState fs_high_force[] = {
    { 0, 0, 0, 0, 160, 0, 10, 1, 1, 0 },
    { 0, 0, 0, 0, 160, 0, 10, 1, 2, 0 },
  };

  const Gesture kNull = Gesture();
  const Gesture kScroll = Gesture(kGestureScroll, 0, 0, 20, 0);
  const Gesture kFling = Gesture(kGestureFling, 0, 0, 20, 0,
                                 GESTURES_FLING_START);

  GestureTestInputs inputs[] = {
    // Don't set the button down if a gesture is active.
    {1.00, 2, fs_low_force,  kScroll, GESTURES_BUTTON_NONE},
    {1.01, 2, fs_high_force, kFling,  GESTURES_BUTTON_NONE},

    // If the button is down before a gesture starts, don't prevent the button
    // from going back up.
    {2.000, 2, fs_low_force,  kNull,   GESTURES_BUTTON_NONE},
    {2.010, 2, fs_high_force, kNull,   GESTURES_BUTTON_LEFT},
    {2.030, 2, fs_high_force, kScroll, GESTURES_BUTTON_LEFT},
    {2.040, 2, fs_low_force,  kScroll, GESTURES_BUTTON_NONE},
    {2.050, 2, fs_low_force,  kFling,  GESTURES_BUTTON_NONE},

    // If there is no "ending" gesture event, allow button clicks after a short
    // timeout.
    {3.000, 2, fs_low_force,  kScroll, GESTURES_BUTTON_NONE},
    {3.010, 2, fs_high_force, kNull,   GESTURES_BUTTON_NONE},
    {3.011, 2, fs_high_force, kNull,   GESTURES_BUTTON_NONE},
    {3.011 + interpreter.active_gesture_timeout_, -1, NULL, kNull, 0},
    {3.200 + interpreter.active_gesture_timeout_,
            2, fs_high_force, kNull,   GESTURES_BUTTON_LEFT},
  };

  for (size_t i = 0; i < arraysize(inputs); i++) {
    GestureTestInputs input = inputs[i];
    base_interpreter->return_value_ = input.gesture;
    stime_t timeout = NO_DEADLINE;
    if (input.touch_count == -1) {
      wrapper.HandleTimer(input.time, &timeout);
    } else {
      unsigned short touch_count =
          static_cast<unsigned short>(input.touch_count);
      HardwareState hs = make_hwstate(input.time, 0, touch_count, touch_count,
                                      input.fs);
      wrapper.SyncInterpret(&hs, &timeout);
      EXPECT_EQ(hs.buttons_down, input.expected_button);
    }
  }
}

}  // namespace gestures
