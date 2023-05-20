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
static const uint32_t kNumPoints = sizeof(points) / sizeof(points[0]);
static const int edges[][2] = {
    {0, 1}, {1, 3}, {3, 2}, {2, 0},
    {4, 5}, {5, 7}, {7, 6}, {6, 4},
    {0, 4}, {1, 5}, {3, 7}, {2, 6},
};
static const uint32_t kNumEdges = sizeof(edges) / sizeof(edges[0]);

static LogChannel s_cubeLog(false);
class Cube : public Demo
{
public:
    Cube() : Demo(-10, 60) {}
    void UpdateAndRender(DisplayList& displayList, float dt);
};
static Cube s_cube;

void Cube::UpdateAndRender(DisplayList& displayList, float dt)
{
    static SinTable::Index x = 0;
    static SinTable::Index y = 0;
    static SinTable::Index z = 0;
    SinTable::Index t = dt;

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

    // Create the spinny cube's transform
    FixedTransform3D transform;
    transform.setRotationXYZ(x, y, z);
    transform.setTranslation(StandardFixedTranslationVector(0, 0, 5));

    // Transform all the points to view space, then screen space
    StandardFixedTranslationVector viewSpacePoints[kNumPoints];
    DisplayListVector2 screenSpacePoints[kNumPoints];
    for (uint32_t i = 0; i < kNumPoints; ++i)
    {
        transform.transformVector(viewSpacePoints[i], points[i]);
        screenSpacePoints[i].x = (viewSpacePoints[i].x / viewSpacePoints[i].z) * (3.f / 4.f) + 0.5f;
        screenSpacePoints[i].y = (viewSpacePoints[i].y / viewSpacePoints[i].z) + 0.5f;
    }

    // Draw all the edges
    int previousPoint = -1;
    for (uint32_t i = 0; i < kNumEdges; ++i)
    {
        if(edges[i][0] != previousPoint)
        {
            // Start a new line
            displayList.PushVector(screenSpacePoints[edges[i][0]], 0);
        }
        previousPoint = edges[i][1];
        displayList.PushVector(screenSpacePoints[previousPoint], 0.5f);
    }
}
