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

#include "buttonrecorder.h"
#include "picovectorscope.h"

#if ENABLE_DEMO_RECORDING
static LogChannel ButtonRecorderStream(true);
static bool s_recording = false;
#endif
static uint32_t s_demoPlaybackNybbleIdx = 0;
static const uint32_t* s_demoStream = nullptr;
static uint32_t s_numDemoStreamNybbles = 0;

#if ENABLE_DEMO_RECORDING
void ButtonRecorder::RecordingEnable(bool enable)
{
    if(enable && !s_recording)
    {
        LOG_INFO(ButtonRecorderStream, "0x%08x, // Seed\n", g_randSeed);
    }
    s_recording = enable;
}
#endif

void ButtonRecorder::PlaybackEnable(const uint32_t* data, uint32_t numNybbles)
{
    s_demoStream = data;
    s_numDemoStreamNybbles = numNybbles;
    g_randSeed = s_demoStream[0];
    s_demoPlaybackNybbleIdx = 8;

}

void ButtonRecorder::Update()
{
#if ENABLE_DEMO_RECORDING
    static uint32_t data = 0;
    static uint32_t nybbleIdx = 0;
    static uint32_t wordIdx = 0;
    if(s_recording || (nybbleIdx != 0))
    {
        uint32_t nybble = 0;
        if(Buttons::IsHeld(Buttons::Id::Left))   nybble |= 1;
        if(Buttons::IsHeld(Buttons::Id::Right))  nybble |= 2;
        if(Buttons::IsHeld(Buttons::Id::Thrust)) nybble |= 4;
        if(Buttons::IsHeld(Buttons::Id::Fire))   nybble |= 8;
        data |= nybble << (nybbleIdx * 4);
        if(++nybbleIdx == 8)
        {
            LOG_INFO(ButtonRecorderStream, "0x%08x, ", data);
            nybbleIdx = 0;
            data = 0;
            if(++wordIdx == 8)
            {
                LOG_INFO(ButtonRecorderStream, "\n");
                wordIdx = 0;
            }
        }
    }
#endif
    if(s_demoPlaybackNybbleIdx != 0)
    {
        uint32_t nybble = s_demoStream[s_demoPlaybackNybbleIdx >> 3] >> ((s_demoPlaybackNybbleIdx & 7) << 2);
        Buttons::FakePress(Buttons::Id::Left,   (nybble & 1) != 0);
        Buttons::FakePress(Buttons::Id::Right,  (nybble & 2) != 0);
        Buttons::FakePress(Buttons::Id::Thrust, (nybble & 4) != 0);
        Buttons::FakePress(Buttons::Id::Fire,   (nybble & 8) != 0);
        if(++s_demoPlaybackNybbleIdx == s_numDemoStreamNybbles)
        {
            s_demoPlaybackNybbleIdx = 0;
        }
    }
}
