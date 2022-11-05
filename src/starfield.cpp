// Classic Starfield demo, using the PicoVectorscope framework.
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

#include "picovectorscope.h"

// Compact storage for stars.  16-bit fixed-point scalars.
typedef FixedPoint<8, 7, int16_t, int32_t, false> CompactStarCoordScalar;
struct StarCoord
{
    CompactStarCoordScalar x, y, z;
};
static constexpr uint32_t kNumStars = 1000;
static StarCoord          s_stars[kNumStars];

// Fixed-point type for performing most of our intermediate maths operations. 32-bit.
typedef FixedPoint<8, 18, int32_t, int32_t, false> StarCoordIntermediate;

// Constants and derived constants
constexpr StarCoordIntermediate kProj       = 0.25f; //< Controls the perspective projection
constexpr StarCoordIntermediate kNearZ      = 32.f; //< How near do stars get before they wrap
constexpr StarCoordIntermediate kFarZ       = kNearZ + CompactStarCoordScalar::kMax;
constexpr StarCoordIntermediate kRecipFarZ  = kFarZ.recip();
constexpr StarCoordIntermediate kBrightnessAtFarZ
    = Mul<CompactStarCoordScalar::kNumWholeBits, -4, 12>(kNearZ, kRecipFarZ);

static CompactStarCoordScalar s_starSpeed(2.f);

// Delcare the PicoVectorscope Demo
class Starfield : public Demo
{
public:
    Starfield() : Demo() {}
    void Init();
    void UpdateAndRender(DisplayList& displayList, float dt);
};
static Starfield s_starfield;

void Starfield::Init()
{
    for (StarCoord& star : s_stars)
    {
        star.x = CompactStarCoordScalar::randFullRange();
        star.y = CompactStarCoordScalar::randFullRange();
        star.z = CompactStarCoordScalar((CompactStarCoordScalar::StorageType)(
            ((CompactStarCoordScalar::StorageType)SimpleRand()) & 0x7fff));
    }
}

void Starfield::UpdateAndRender(DisplayList& displayList, float dt)
{
    if (Buttons::IsJustPressed(Buttons::Id::Left)) s_starSpeed *= 0.75f;
    if (Buttons::IsJustPressed(Buttons::Id::Right)) s_starSpeed *= 1.25f;

    for (StarCoord& star : s_stars)
    {
        star.z -= s_starSpeed;
        if (star.z < 0) // Wrap?
        {
            star.z += CompactStarCoordScalar(CompactStarCoordScalar::kMaxStorageType);
        }

        // Careful fixed-point maths to maintain precision
        StarCoordIntermediate recipZ    = (StarCoordIntermediate(star.z) + kNearZ).recip();
        StarCoordIntermediate projOverZ = Mul<0, 0>(kProj, recipZ);
        StarCoordIntermediate screenX
            = Mul<-4, CompactStarCoordScalar::kNumWholeBits>(projOverZ, star.x) + 0.5f;
        if ((screenX < 1) && (screenX > 0))
        {
            StarCoordIntermediate screenY
                = Mul<-4, CompactStarCoordScalar::kNumWholeBits>(projOverZ, star.y) + 0.5f;
            if ((screenY < 1) && (screenY > 0))
            {
                // The star is on the screen.  Calculate a brightness, and draw it.
                StarCoordIntermediate brightness
                    = Mul<CompactStarCoordScalar::kNumWholeBits, -4, 12>(kNearZ, recipZ);
                brightness -= kBrightnessAtFarZ; // Ensure brightness 0 at FarZ
                displayList.PushPoint(screenX, screenY, brightness);
            }
        }
    }
}
