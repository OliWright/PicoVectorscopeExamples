// Some predefined shapes and base classes for making cool 2D games.
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

// This probably isn't a good place for this.
// Will probably move it to 'extras', or delete entirely.

#pragma once
#include "picovectorscope.h"

enum class GameShape
{
    eAsteroid0,
    eAsteroid1,
    eAsteroid2,
    eAsteroid3,
    eSaucer,
    eShip,
    eThrust,

    eCount
};
void PushGameShape(DisplayList& displayList, const FixedTransform2D& transform, GameShape shape, Intensity intensity);

uint32_t FragmentGameShape(GameShape shape,
                           const FixedTransform2D& transform,
                           Fragment* outFragments,
                           uint32_t outFragmentsCapacity);


typedef FixedPoint<4,20,int32_t,int32_t> GameScalar;
typedef Vector2<GameScalar> GameVector2;
typedef FixedPoint<1,20,int32_t,int32_t> Velocity;
typedef Vector2<Velocity> VelocityVector2;
typedef FixedPoint<1,20,int32_t,int32_t> Acceleration;
typedef Vector2<Acceleration> AccelerationVector2;

template<typename T>
static inline void wrap(Vector2<T>& vec)
{
    vec.x = vec.x.frac();
    vec.y = vec.y.frac();
}

// Base class for all objects in the game.
// This is classic OOP.
struct BaseObject
{
    GameVector2 m_position;
    VelocityVector2 m_velocity;
    Intensity m_brightness;
    bool m_active;

    BaseObject() : m_active(false) {}

    inline void Move()
    {
        m_position.x += m_velocity.x;
        m_position.y += m_velocity.y;
        wrap(m_position);
    }
};

// An intermediate class for objects using a GameShape with scale and rotation
struct ShapeObject : public BaseObject
{
    GameShape m_shape;
    GameScalar m_rotation;
    GameScalar m_scale;

    ShapeObject() : BaseObject() {}

    void Rotate(GameScalar deltaAngle);

    void CalcTransform(FixedTransform2D& outTransform);

    void UpdateAndDraw(DisplayList& displayList);
};
