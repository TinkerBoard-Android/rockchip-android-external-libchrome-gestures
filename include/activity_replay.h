// Copyright (c) 2011 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef GESTURES_ACTIVITY_REPLAY_H_
#define GESTURES_ACTIVITY_REPLAY_H_

#include <string>
#include <tr1/memory>
#include <set>

#include <base/values.h>

#include "gestures/include/activity_log.h"
#include "gestures/include/gestures.h"
#include "gestures/include/interpreter.h"

// This class can parse a JSON log, as generated by ActivityLog and replay
// it on an interpreter.

namespace gestures {

class PropRegistry;

class ActivityReplay {
 public:
  explicit ActivityReplay(PropRegistry* prop_reg);
  // Returns true on success.
  bool Parse(const std::string& data);
  // An empty set means honor all properties
  bool Parse(const std::string& data, const std::set<std::string>& honor_props);

  // If there is any unexpected behavior, replay continues, but EXPECT_*
  // reports failure, otherwise no failure is reported.
  void Replay(Interpreter* interpreter);

 private:
  // These return true on success
  bool ParseProperties(DictionaryValue* dict,
                       const std::set<std::string>& honor_props);
  bool ParseHardwareProperties(DictionaryValue* obj,
                               HardwareProperties* out_props);
  bool ParseEntry(DictionaryValue* entry);
  bool ParseHardwareState(DictionaryValue* entry);
  bool ParseFingerState(DictionaryValue* entry, FingerState* out_fs);
  bool ParseTimerCallback(DictionaryValue* entry);
  bool ParseCallbackRequest(DictionaryValue* entry);
  bool ParseGesture(DictionaryValue* entry);
  bool ParseGestureMove(DictionaryValue* entry, Gesture* out_gs);
  bool ParseGestureScroll(DictionaryValue* entry, Gesture* out_gs);
  bool ParseGestureSwipe(DictionaryValue* entry, Gesture* out_gs);
  bool ParseGestureSwipeLift(DictionaryValue* entry, Gesture* out_gs);
  bool ParseGesturePinch(DictionaryValue* entry, Gesture* out_gs);
  bool ParseGestureButtonsChange(DictionaryValue* entry, Gesture* out_gs);
  bool ParseGestureFling(DictionaryValue* entry, Gesture* out_gs);
  bool ParsePropChange(DictionaryValue* entry);

  bool ReplayPropChange(const ActivityLog::PropChangeEntry& entry);

  ActivityLog log_;
  HardwareProperties hwprops_;
  PropRegistry* prop_reg_;
  std::vector<std::tr1::shared_ptr<const std::string> > names_;
};

}  // namespace gestures

#endif  // GESTURES_ACTIVITY_REPLAY_H_
