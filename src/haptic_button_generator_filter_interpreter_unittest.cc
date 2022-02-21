// Copyright 2021 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include "gestures/include/haptic_button_generator_filter_interpreter.h"
#include "gestures/include/unittest_util.h"

namespace gestures {

class HapticButtonGeneratorFilterInterpreterTest : public ::testing::Test {};

class HapticButtonGeneratorFilterInterpreterTestInterpreter :
      public Interpreter {
 public:
  HapticButtonGeneratorFilterInterpreterTestInterpreter()
      : Interpreter(NULL, NULL, false) {}
};

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
    wrapper.SyncInterpret(&hs[i], NULL);
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
    wrapper.SyncInterpret(&hs[i], NULL);
    EXPECT_EQ(hs[i].buttons_down, expected_buttons[i]);
  }
}

}  // namespace gestures
