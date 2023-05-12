// Helper for recording buttons for game demos.
//
// Copyright (C) 2022 Oli Wright
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// A copy of the GNU General Public License can be found in the file
// LICENSE.txt in the root of this project.
// If not, see <https://www.gnu.org/licenses/>.
//
// oli.wright.github@gmail.com


#pragma once
#include "picovectorscope.h"

#define ENABLE_DEMO_RECORDING 0

class ButtonRecorder
{
public:
#if ENABLE_DEMO_RECORDING
    static void RecordingEnable(bool enable);
#else
    // Stub out the recording functions
    static void RecordingEnable(bool) {}
#endif
    static void PlaybackEnable(const uint32_t* data, uint32_t numNybbles);
    static void Update();
};