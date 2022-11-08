// Sprites, for the game "Invaders From Space!"
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

#include "pico/types.h"

namespace InvadersFromSpace
{

extern const uint8_t kSpritesInvaders[3][2][8][2];
extern const uint8_t kSpriteShield[16][3];
extern const uint8_t kSpritePlayer[3][8][2];
extern const uint8_t kSpriteInvaderExplosion[8][2];
extern const uint8_t kSpritesBullets[4][4][9][1];
extern const uint8_t kSpriteShieldExplosion[8][1];

}