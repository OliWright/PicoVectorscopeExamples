// Absolutely minimal 3D spinny cube demo.
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
#include "extras/shapes3d.h"

static const StandardFixedTranslationVector points[] = {
    StandardFixedTranslationVector(-1, -1, -1),
    StandardFixedTranslationVector( 1, -1, -1),
    StandardFixedTranslationVector(-1,  1, -1),
    StandardFixedTranslationVector( 1,  1, -1),
    StandardFixedTranslationVector(-1, -1,  1),
    StandardFixedTranslationVector( 1, -1,  1),
    StandardFixedTranslationVector(-1,  1,  1),
    StandardFixedTranslationVector( 1,  1,  1),
};

static const uint16_t edges[][2] = {
    {0, 1}, {1, 3}, {3, 2}, {2, 0},
    {4, 5}, {5, 7}, {7, 6}, {6, 4},
    {0, 4}, {1, 5}, {3, 7}, {2, 6},
};

static constexpr Shape3D kCubeShape = SHAPE_3D(points, edges);

static LogChannel s_cubeLog(false);
class SpinnyCubeDemo : public Demo
{
public:
    SpinnyCubeDemo() : Demo(-10, 60) {}
    void UpdateAndRender(DisplayList& displayList, float dt);
};
static SpinnyCubeDemo s_spinnyCubeDemo;

static void drawCube(DisplayList& displayList, const Camera& camera, Intensity intensity,
                     SinTable::Index x, SinTable::Index y, SinTable::Index z,
                     const StandardFixedTranslationVector& position)
{
    // Create the spinny cube's transform
    FixedTransform3D modelToWorld;
    modelToWorld.setRotationXYZ(x, y, z);
    modelToWorld.setTranslation(position);

    kCubeShape.Draw(displayList, modelToWorld, camera, intensity);
}

void SpinnyCubeDemo::UpdateAndRender(DisplayList& displayList, float dt)
{
    static SinTable::Index x = 0;
    static SinTable::Index y = 0;
    static SinTable::Index z = 0;
    static SinTable::Index c = 0;
    SinTable::Index t = dt;

    c += t * 0.8f;
    if (c > (kPi * 2.f))
    {
        c -= kPi * 2.f;
    }

    t *= 0.8f; // Global spinny speed adjust

    // Modify the rotation angles of the spinny cube
    x += t * 0.5f;
    if (x > (kPi * 2.f))
    {
        x -= kPi * 2.f;
    }
    y += t * 0.7f;
    if (y > (kPi * 2.f))
    {
        y -= kPi * 2.f;
    }
    z += t * 0.8f;
    if (z > (kPi * 2.f))
    {
        z -= kPi * 2.f;
    }

    FixedTransform3D viewToWorld;
    viewToWorld.setAsIdentity();
    viewToWorld.t.z = ((StandardFixedTranslationScalar) SinTable::LookUp(c)) * 4.f;

    Camera camera;
    camera.SetCameraToWorld(viewToWorld);
    camera.Calculate();

    static const SinTable::Index gap = 0.1f;
    typedef FixedPoint<6,0,int32_t,int32_t,false> IntT;
    const uint kNumCubes = 1;
    for(uint i = 0; i < kNumCubes; ++i)
    {
        drawCube(displayList, camera, 0.7f,
                 x + (gap * 0.5f * (IntT)i), y + (gap * 0.7f * (IntT)i), z + (gap * 0.8f * (IntT)i),
                 StandardFixedTranslationVector(0, 0, 2 + i * 4));
    }
}
