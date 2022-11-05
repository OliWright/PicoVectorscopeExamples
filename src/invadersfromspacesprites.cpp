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

#include "invadersfromspacesprites.h"

const uint16_t s_invaderSprites[3][2][8] = {
    {
        {
            0b0000001111000000,
            0b0001111111111000,
            0b0011111111111100,
            0b0011100110011100,
            0b0011111111111100,
            0b0000011001100000,
            0b0000110110110000,
            0b0011000000001100,
        },
        {
            0b0000001111000000,
            0b0001111111111000,
            0b0011111111111100,
            0b0011100110011100,
            0b0011111111111100,
            0b0000011001100000,
            0b0000110110110000,
            0b0000011001100000,
        },
    },
    {
        {
            0b0000010000010000,
            0b0001001000100100,
            0b0001011111110100,
            0b0001110111011100,
            0b0001111111111100,
            0b0000111111111000,
            0b0000010000010000,
            0b0000100000001000,
        },
        {
            0b0000010000010000,
            0b0000001000100000,
            0b0000011111110000,
            0b0001110111011100,
            0b0001111111111100,
            0b0001011111110100,
            0b0001010000010100,
            0b0000001101100000,
        },
    },
    {
        {
            0b0000000110000000,
            0b0000001111000000,
            0b0000011111100000,
            0b0000110110110000,
            0b0000111111110000,
            0b0000001001000000,
            0b0000010110100000,
            0b0000101001010000,
        },
        {
            0b0000000110000000,
            0b0000001111000000,
            0b0000011111100000,
            0b0000110110110000,
            0b0000111111110000,
            0b0000010110100000,
            0b0000100000010000,
            0b0000010000100000,
        },
    },
};

// clang-format off
const uint8_t s_shieldSprite[16][3] ={
    { 0b00000111, 0b11111111, 0b11100000 },
    { 0b00001111, 0b11111111, 0b11110000 },
    { 0b00011111, 0b11111111, 0b11111000 },
    { 0b00111111, 0b11111111, 0b11111100 },
    { 0b01111111, 0b11111111, 0b11111110 },
    { 0b01111111, 0b11111111, 0b11111110 },
    { 0b01111111, 0b11111111, 0b11111110 },
    { 0b01111111, 0b11111111, 0b11111110 },
    { 0b01111111, 0b11111111, 0b11111110 },
    { 0b01111111, 0b11111111, 0b11111110 },
    { 0b01111111, 0b11111111, 0b11111110 },
    { 0b01111111, 0b11111111, 0b11111110 },
    { 0b01111111, 0b00000000, 0b11111110 },
    { 0b01111110, 0b00000000, 0b01111110 },
    { 0b01111100, 0b00000000, 0b00111110 },
    { 0b01111100, 0b00000000, 0b00111110 },
};

const uint8_t s_playerSprite[8][2] ={
    { 0b00000000, 0b10000000 },
    { 0b00000001, 0b11000000 },
    { 0b00000001, 0b11000000 },
    { 0b00011111, 0b11111100 },
    { 0b00111111, 0b11111110 },
    { 0b00111111, 0b11111110 },
    { 0b00111111, 0b11111110 },
    { 0b00111111, 0b11111110 },
};
// clang-format on

const uint8_t s_bulletSprites[4][4][9][1] = {
    {
        {
            { 0b00000000 },
            { 0b00000000 },
            { 0b01000000 },
            { 0b10000000 },
            { 0b01000000 },
            { 0b00100000 },
            { 0b01000000 },
            { 0b10000000 },
            { 0b01000000 },
        },
        {
            { 0b00000000 },
            { 0b00000000 },
            { 0b10000000 },
            { 0b01000000 },
            { 0b00100000 },
            { 0b01000000 },
            { 0b10000000 },
            { 0b01000000 },
            { 0b00100000 },
        },
        {
            { 0b00000000 },
            { 0b00000000 },
            { 0b01000000 },
            { 0b00100000 },
            { 0b01000000 },
            { 0b10000000 },
            { 0b01000000 },
            { 0b00100000 },
            { 0b01000000 },
        },
        {
            { 0b00000000 },
            { 0b00000000 },
            { 0b00100000 },
            { 0b01000000 },
            { 0b10000000 },
            { 0b01000000 },
            { 0b00100000 },
            { 0b01000000 },
            { 0b10000000 },
        },
    },
    {
        {
            { 0b00000000 },
            { 0b00000000 },
            { 0b01000000 },
            { 0b01000000 },
            { 0b01000000 },
            { 0b01000000 },
            { 0b01000000 },
            { 0b11100000 },
            { 0b00000000 },
        },
        {
            { 0b00000000 },
            { 0b00000000 },
            { 0b01000000 },
            { 0b01000000 },
            { 0b01000000 },
            { 0b11100000 },
            { 0b01000000 },
            { 0b01000000 },
            { 0b00000000 },
        },
        {
            { 0b00000000 },
            { 0b00000000 },
            { 0b01000000 },
            { 0b01000000 },
            { 0b11100000 },
            { 0b01000000 },
            { 0b01000000 },
            { 0b01000000 },
            { 0b00000000 },
        },
        {
            { 0b00000000 },
            { 0b00000000 },
            { 0b11100000 },
            { 0b01000000 },
            { 0b01000000 },
            { 0b01000000 },
            { 0b01000000 },
            { 0b01000000 },
            { 0b00000000 },
        },
    },
    {
        {
            { 0b00000000 },
            { 0b00000000 },
            { 0b01000000 },
            { 0b01000000 },
            { 0b01000000 },
            { 0b01000000 },
            { 0b01000000 },
            { 0b01000000 },
            { 0b01000000 },
        },
        {
            { 0b00000000 },
            { 0b00000000 },
            { 0b01000000 },
            { 0b01000000 },
            { 0b11000000 },
            { 0b01100000 },
            { 0b01000000 },
            { 0b11000000 },
            { 0b01100000 },
        },
        {
            { 0b00000000 },
            { 0b00000000 },
            { 0b01000000 },
            { 0b01000000 },
            { 0b01000000 },
            { 0b01000000 },
            { 0b01000000 },
            { 0b01000000 },
            { 0b01000000 },
        },
        {
            { 0b00000000 },
            { 0b00000000 },
            { 0b01100000 },
            { 0b11000000 },
            { 0b01000000 },
            { 0b01100000 },
            { 0b11000000 },
            { 0b01000000 },
            { 0b01000000 },
        },
    },
    {
        {
            { 0b01000000 },
            { 0b01000000 },
            { 0b01000000 },
            { 0b01000000 },
            { 0b01000000 },
            { 0b01000000 },
            { 0b01000000 },
            { 0b00000000 },
            { 0b00000000 },
        },
        {
            { 0b01000000 },
            { 0b01000000 },
            { 0b01000000 },
            { 0b01000000 },
            { 0b01000000 },
            { 0b01000000 },
            { 0b01000000 },
            { 0b00000000 },
            { 0b00000000 },
        },
        {
            { 0b01000000 },
            { 0b01000000 },
            { 0b01000000 },
            { 0b01000000 },
            { 0b01000000 },
            { 0b01000000 },
            { 0b01000000 },
            { 0b00000000 },
            { 0b00000000 },
        },
        {
            { 0b01000000 },
            { 0b01000000 },
            { 0b01000000 },
            { 0b01000000 },
            { 0b01000000 },
            { 0b01000000 },
            { 0b01000000 },
            { 0b00000000 },
            { 0b00000000 },
        },
    },
};
