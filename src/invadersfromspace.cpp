// Invaders From Space! A sample game for PicoVectorscope using
// a 1 bit per pixel bitmap raster display.
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
#include "extras/bitmap.h"
#include "extras/bitmapfont.h"

#include "invadersfromspacesprites.h"
#include "invadersdemo.hpp"
#include "buttonrecorder.h"

namespace InvadersFromSpace
{

static constexpr uint kDisplayWidth        = 200;
static constexpr uint kDisplayHeight       = 150;
static constexpr uint kDisplayPitchInBytes = (kDisplayWidth + 7) >> 3;
static constexpr uint kNumLives            = 3;
static constexpr uint kNumInvadersX        = 9;
static constexpr uint kNumInvadersY        = 5;
static constexpr int  kInvaderSpacingX     = 18;
static constexpr int  kInvaderSpacingY     = 16;
static constexpr int  kShieldSpacingX      = 48;
static constexpr int  kPlayerY             = kDisplayHeight - 16;
static constexpr int  kShieldY             = kPlayerY - 24;
static constexpr int  kInvaderDX           = 2;
static constexpr int  kInvaderDY           = 8;
static constexpr uint kMaxBullets          = 16;
static constexpr uint kMaxExplosions       = 4;
static constexpr uint kNumBulletOwners     = kNumInvadersX + 1;
static constexpr uint kPlayerBulletOwnerId = kNumInvadersX;

static const char kTextPlay[]      = "PLAY";
static const char kTextInvaders[]  = "INVADERS";
static const char kTextFromSpace[] = "FROM SPACE!";
static const char kTextGameOver[]  = "GAME OVER";

static uint8_t                    s_displayBitmap[kDisplayHeight][kDisplayPitchInBytes] = {};
static DisplayList::RasterDisplay s_rasterDisplay;

static const uint8_t* scanlineCallback(uint32_t scanline, void*)
{
    return s_displayBitmap[scanline];
}

// Some convenience wrappers for bitmap functions
static void drawText(int x, int y, const char* message, bool centred)
{
    DrawTextTo1bitDisplay(s_displayBitmap[0], kDisplayPitchInBytes, x, y, message, centred);
}
static void clearRect(int x, int y, int width, int height)
{
    ClearRectOn1bitDisplay(s_displayBitmap[0], kDisplayPitchInBytes, x, y, width, height);
}
template <typename T>
static void drawSprite(
    int x, int y, int width, int height, T sprite, SpriteMode spriteMode = SpriteMode::Default)
{
    DrawSpriteTo1bitDisplay(s_displayBitmap[0], kDisplayPitchInBytes, x, y, width, height, sprite[0],
                            sizeof(sprite[0]), spriteMode);
}
static void drawCentredTextSlowly(const char* text, int len, int x, int y, int frameCount)
{
    if ((frameCount >= 0) && ((frameCount & 7) == 0))
    {
        int charIdx = (frameCount >> 3);
        if (charIdx < len)
        {
            // Draw the next character
            const char message[2] = { text[charIdx], 0 };
            DrawTextTo1bitDisplay(s_displayBitmap[0], kDisplayPitchInBytes,
                                  x - (len << 2) + (charIdx << 3), y, message, false);
        }
    }
}

static void clearScreen()
{
    uint*       pixels = (uint*)s_displayBitmap[0];
    const uint* end    = pixels + ((kDisplayHeight * kDisplayPitchInBytes) >> 2);
    while (pixels != end)
    {
        *pixels++ = 0;
    }
}

static void drawShields()
{
    int x = (kDisplayWidth >> 1) - (kShieldSpacingX >> 1) - kShieldSpacingX - 12;
    for (uint i = 0; i < 4; ++i)
    {
        drawSprite(x, kShieldY, 24, 16, kSpriteShield);
        x += kShieldSpacingX;
    }
}

// Explosions draw a sprite, and then erase themselves after a set number of ticks
struct Explosion
{
    int16_t m_x;
    int16_t m_y;
    uint8_t m_tick;
    enum class Type
    {
        Invader,
        Shield,
    };
    Type m_type;

    void Clear()
    {
        switch (m_type)
        {
        case Type::Invader:
            clearRect(m_x, m_y, 16, 8);
            break;
        case Type::Shield:
            drawSprite(m_x, m_y, 8, 8, kSpriteShieldExplosion, SpriteMode::ClearPixels);
            break;
        }
    }

    static void Add(int16_t x, int16_t y, Type explosionType)
    {
        for (uint i = 0; i < kMaxExplosions; ++i)
        {
            Explosion& explosion = s_explosions[i];
            if (explosion.m_tick == 0)
            {
                // This one is available
                explosion.m_x    = x;
                explosion.m_y    = y;
                explosion.m_tick = (explosionType == Type::Invader) ? 20 : 10;
                explosion.m_type = explosionType;
                switch (explosionType)
                {
                case Type::Invader:
                    drawSprite(x, y, 16, 8, kSpriteInvaderExplosion);
                    break;
                case Type::Shield:
                    drawSprite(x, y, 8, 8, kSpriteShieldExplosion, SpriteMode::SetPixels);
                    break;
                }
            }
        }
    }

    static void UpdateAndDrawAll()
    {
        for (uint i = 0; i < kMaxExplosions; ++i)
        {
            Explosion& explosion = s_explosions[i];
            if (explosion.m_tick > 0)
            {
                if (--explosion.m_tick == 0)
                {
                    explosion.Clear();
                }
            }
        }
    }

    static void ClearAll()
    {
        for (uint i = 0; i < kMaxExplosions; ++i)
        {
            Explosion& explosion = s_explosions[i];
            if (explosion.m_tick > 0)
            {
                clearRect(explosion.m_x, explosion.m_y, 16, 8);
                explosion.m_tick = 0;
            }
        }
    }

    static Explosion s_explosions[kMaxExplosions];
};
Explosion Explosion::s_explosions[kMaxExplosions] = {};

struct Bullet
{
    int16_t m_x;
    int16_t m_y;
    int16_t m_dy;
    bool    m_active = 0;
    int8_t  m_frame;
    int8_t  m_sprite;

    void Destroy()
    {
        if (m_active)
        {
            //--s_numActive;
            m_active = false;
            clearRect(m_x - 1, m_y, 3, 9);

            for (uint i = 0; i < kNumBulletOwners; ++i)
            {
                if (s_ownerBullets[i] == this)
                {
                    s_ownerBullets[i] = nullptr;
                    break;
                }
            }
        }
    }

    void MoveAndDraw()
    {
        int testPointY = m_y + m_dy;
        if (m_dy > 0)
        {
            // Going down.  Baddy bullet.
            testPointY += 9;
            if (testPointY > (kPlayerY + 8))
            {
                Destroy();
                return;
            }
        }
        else
        {
            // Going up.  Goody bullet.
            if (testPointY < 8)
            {
                Destroy();
                return;
            }
        }
        m_frame = (m_frame + 1) & 3;
        // Test the pixel to see if we've hit something
        if ((s_displayBitmap[testPointY][m_x >> 3] & (0x80 >> (m_x & 7))) != 0)
        {
            Destroy();
            m_y = testPointY;
            HitSomething();
        }
        else
        {
            m_y += m_dy;
            drawSprite(m_x - 1, m_y, 3, 9, kSpritesBullets[m_sprite][m_frame]);
        }
    }

    void HitSomething();

    static void MoveAndDrawAll()
    {
        for (uint i = 0; i < kMaxBullets; ++i)
        {
            if (s_bullets[i].m_active)
            {
                s_bullets[i].MoveAndDraw();
            }
        }
    }

    static Bullet* FindInactive()
    {
        for (uint i = 0; i < kMaxBullets; ++i)
        {
            if (!s_bullets[i].m_active)
            {
                return s_bullets + i;
            }
        }
        return nullptr;
    }

    static void DestroyAll()
    {
        for (uint i = 0; i < kMaxBullets; ++i)
        {
            s_bullets[i].Destroy();
        }
    }

    static Bullet  s_bullets[kMaxBullets];
    static Bullet* s_ownerBullets[kNumInvadersX + 1];
};
Bullet  Bullet::s_bullets[kMaxBullets];
Bullet* Bullet::s_ownerBullets[kNumInvadersX + 1] = {};

struct Invader
{
    int16_t m_x;
    int16_t m_y;
    uint8_t m_sprite;
    uint8_t m_frame;
    bool    m_alive;

    void Destroy()
    {
        if (m_alive)
        {
            m_alive = false;
            clearRect(m_x, m_y, 16, 8);
            Explosion::Add(m_x, m_y, Explosion::Type::Invader);
            --s_numAlive;
        }
    }

    void UpdateAndDraw(int32_t dx, int32_t dy, int column)
    {
        m_frame ^= 1;
        clearRect(m_x, m_y, 16, 8);
        m_x += dx;
        m_y += dy;
        drawSprite(m_x, m_y, 16, 8, kSpritesInvaders[m_sprite][m_frame]);

        // Should we shoot?
        // Is there already an alive bullet for this column?
        if (Bullet::s_ownerBullets[column] != nullptr)
        {
            return;
        }
        // Are we the lowest invader in the column?
        for (int y = kNumInvadersY - 1; y >= 0; --y)
        {
            Invader& invader = s_invaders[y][column];
            if (invader.m_alive)
            {
                // This is the lowest invader in the column
                if (&invader == this)
                {
                    // And it's us!
                    uint32_t rand = SimpleRand();
                    if ((rand & 0x7) == 0)
                    {
                        // Fire!
                        Bullet* bullet = Bullet::FindInactive();
                        if (bullet != nullptr)
                        {
                            bullet->m_x                    = m_x + 8;
                            bullet->m_y                    = m_y + 9;
                            bullet->m_sprite               = ((rand >> 8) % 3);
                            bullet->m_dy                   = 1;
                            bullet->m_active               = true;
                            Bullet::s_ownerBullets[column] = bullet;
                        }
                    }
                }
                break;
            }
        }
    }

    bool IsHit(int x, int y) const
    {
        return (x >= m_x) && (x < (m_x + 16)) && (y >= m_y) && (y < (m_y + 8));
    }

    static bool HitAny(int x, int y)
    {
        // Lazy brute-force iterate over all the invaders to see if we've hit any
        for (uint i = 0; i < kNumInvadersY; ++i)
        {
            for (uint j = 0; j < kNumInvadersX; ++j)
            {
                Invader& invader = s_invaders[i][j];
                if (invader.m_alive && invader.IsHit(x, y))
                {
                    invader.Destroy();
                    return true;
                }
            }
        }
        return false;
    }

    static void ConfigureAllForLevel(uint level)
    {
        DestroyAll();
        uint              top          = 20;
        uint              left         = 20;
        constexpr uint8_t kRowSprites[] = { 2, 1, 1, 0, 0 };
        static_assert(count_of(kRowSprites) >= kNumInvadersY, "");
        for (uint i = 0; i < kNumInvadersY; ++i)
        {
            for (uint j = 0; j < kNumInvadersX; ++j)
            {
                Invader& invader = s_invaders[i][j];
                invader.m_x      = left + (j * kInvaderSpacingX);
                invader.m_y      = top + (i * kInvaderSpacingY);
                invader.m_sprite = kRowSprites[i];
                invader.m_frame  = 0;
                invader.m_alive  = true;
                drawSprite(invader.m_x, invader.m_y, 16, 8, kSpritesInvaders[invader.m_sprite][0]);
            }
        }
        s_direction = Direction::eDownToLeft;
        s_updateX   = kNumInvadersX - 1;
        s_updateY   = kNumInvadersY - 1;
        s_limit     = top;
        s_numAlive  = kNumInvadersX * kNumInvadersY;
    }

    static void DestroyAll()
    {
        for (uint i = 0; i < kNumInvadersY; ++i)
        {
            for (uint j = 0; j < kNumInvadersX; ++j)
            {
                s_invaders[i][j].Destroy();
            }
        }
        s_numAlive = 0;
    }

    enum class Direction
    {
        eLeft,
        eRight,
        eDownToLeft,
        eDownToRight,
    };

    // Pick the next invader to update, return true if we've gone around them all.
    static bool Advance()
    {
        if (s_numAlive == 0)
        {
            return true;
        }
        bool wrapped = false;
        do
        {
            if (++s_updateX == kNumInvadersX)
            {
                s_updateX = 0;
                if (++s_updateY == kNumInvadersY)
                {
                    s_updateY = 0;
                    assert(!wrapped);
                    wrapped = true;
                }
            }
        } while (!s_invaders[s_updateY][s_updateX].m_alive);
        return wrapped;
    }

    static void UpdateAll()
    {
        if (s_numAlive == 0)
        {
            return;
        }
        if (Advance())
        {
            // Check to see if we've hit the limit and need to change direction
            switch (s_direction)
            {
            case Direction::eLeft:
                if (s_limit < 4)
                {
                    s_direction = Direction::eDownToRight;
                    s_limit     = 0;
                }
                break;
            case Direction::eRight:
                if (s_limit > (int32_t)(kDisplayWidth - 16 - 4))
                {
                    s_direction = Direction::eDownToLeft;
                    s_limit     = 0;
                }
                break;
            case Direction::eDownToLeft:
                s_direction = Direction::eLeft;
                s_limit     = kDisplayWidth;
                break;
            case Direction::eDownToRight:
                s_direction = Direction::eRight;
                s_limit     = 0;
                break;
            }
        }

        // Update our selected invader
        Invader& invader = s_invaders[s_updateY][s_updateX];
        switch (s_direction)
        {
        case Direction::eLeft:
            invader.UpdateAndDraw(-kInvaderDX, 0, s_updateX);
            s_limit = (invader.m_x < s_limit) ? invader.m_x : s_limit;
            break;
        case Direction::eRight:
            invader.UpdateAndDraw(kInvaderDX, 0, s_updateX);
            s_limit = (invader.m_x > s_limit) ? invader.m_x : s_limit;
            break;
        case Direction::eDownToLeft:
        case Direction::eDownToRight:
            invader.UpdateAndDraw(0, kInvaderDY, s_updateX);
            s_limit = (invader.m_y > s_limit) ? invader.m_y : s_limit;
            break;
        }
    }

    static bool HaveAnyReachedTheBottom()
    {
        switch (s_direction)
        {
        default:
            break;
        case Direction::eDownToLeft:
        case Direction::eDownToRight:
            return (s_limit + 8) >= kPlayerY;
        }
        return false;
    }

    static Invader   s_invaders[kNumInvadersY][kNumInvadersX];
    static Direction s_direction;
    static uint      s_updateX;
    static uint      s_updateY;
    static int32_t   s_limit;
    static uint      s_numAlive;
};

Invader            Invader::s_invaders[kNumInvadersY][kNumInvadersX];
Invader::Direction Invader::s_direction;
uint               Invader::s_updateX;
uint               Invader::s_updateY;
int32_t            Invader::s_limit;
uint               Invader::s_numAlive = 0;

struct PlayerShip
{
    uint m_x = 0;
    enum class State
    {
        eInactive,
        eActive,
        eExploding,
    };
    State m_state          = State::eInactive;
    uint  m_explosionFrame = 0;

    void Deactivate()
    {
        if (m_state != State::eInactive)
        {
            clearRect(m_x, kPlayerY, 16, 8);
            m_state = State::eInactive;
        }
    }

    void Activate()
    {
        if (m_state != State::eActive)
        {
            Deactivate();
            m_x     = 0;
            m_state = State::eActive;
        }
    }

    void UpdateAndDraw()
    {
        switch (m_state)
        {
        case State::eInactive:
            break;
        case State::eActive:
            if (Buttons::IsHeld(Buttons::Id::Left) && (m_x > 1))
            {
                --m_x;
            }
            if (Buttons::IsHeld(Buttons::Id::Right) && (m_x < (kDisplayWidth - 16 - 1)))
            {
                ++m_x;
            }
            if (Buttons::IsJustPressed(Buttons::Id::Fire))
            {
                if (Bullet::s_ownerBullets[kPlayerBulletOwnerId] == nullptr)
                {
                    Bullet* bullet = Bullet::FindInactive();
                    if (bullet)
                    {
                        bullet->m_x                                  = (m_x + 8);
                        bullet->m_y                                  = kPlayerY - 4;
                        bullet->m_sprite                             = 3;
                        bullet->m_dy                                 = -2;
                        bullet->m_active                             = true;
                        Bullet::s_ownerBullets[kPlayerBulletOwnerId] = bullet;
                    }
                }
            }
            drawSprite(m_x, kPlayerY, 16, 8, kSpritePlayer[0]);
            break;
        case State::eExploding:
            ++m_explosionFrame;
            drawSprite(m_x, kPlayerY, 16, 8, kSpritePlayer[1 + ((m_explosionFrame >> 2) & 1)]);
            break;
        }
    }
};
static PlayerShip s_playerShip;

// High level game state
struct GameState
{
    enum State
    {
        AttractModeTitle,
        AttractModeDemo,
        GameStartLevel,
        GameStartDelay,
        Game,
        GamePlayerDestroyed,
        GameInterLevel,
        GameOver,

        Count
    };

    typedef FixedPoint<10, 13, int32_t, int32_t, false> Duration;

    struct StateInfo
    {
        Duration m_duration;
        State    m_autoNextState;
    };

    static const StateInfo s_stateInfo[];

    static State    s_currentState;
    static Duration s_timeInCurrentState;
    static uint     s_numFramesInCurrentState;
    static uint     s_numLives;
    static uint     s_score;
    static uint     s_level;

    static void ChangeState(State newState)
    {
        s_timeInCurrentState      = 0;
        s_numFramesInCurrentState = 0;
        s_currentState            = newState;

        // State initialisation
        switch (s_currentState)
        {
        case State::AttractModeTitle:
            // Nice clean title screen
            clearScreen();
            drawShields();
            Invader::DestroyAll();
            Bullet::DestroyAll();
            Explosion::ClearAll();
            break;

        case State::AttractModeDemo:
            ButtonRecorder::PlaybackEnable(s_demoStream, s_numDemoStreamNybbles);
            clearScreen();
            drawShields();
            Invader::ConfigureAllForLevel(0);
            break;

        case State::GameStartLevel:
            Explosion::ClearAll();
            clearScreen();
            drawShields();
            Invader::ConfigureAllForLevel(s_level);
            ChangeState((s_playerShip.m_state == PlayerShip::State::eActive) ? State::Game :
                                                                               State::GameStartDelay);
            break;

        case State::GamePlayerDestroyed:
            --s_numLives;
            break;

        case State::GameStartDelay:
            if (s_numLives == 0)
            {
                ButtonRecorder::RecordingEnable(false);
                ChangeState(State::GameOver);
            }
            if (s_playerShip.m_state == PlayerShip::State::eExploding)
            {
                s_playerShip.Deactivate();
            }
            break;

        case State::Game:
            // Active the player ship
            s_playerShip.Activate();
            break;

        case State::GameOver:
            break;

        default:
            break;
        }
    }

    static void Update(Duration dt)
    {
        const StateInfo& stateInfo = s_stateInfo[(int)s_currentState];
        s_timeInCurrentState += dt;
        ++s_numFramesInCurrentState;
        if (stateInfo.m_duration != 0)
        {
            // Auto change-state after timeout
            if (s_timeInCurrentState > stateInfo.m_duration)
            {
                ChangeState(stateInfo.m_autoNextState);
            }
        }

        switch (s_currentState)
        {
        case State::AttractModeTitle:
            drawCentredTextSlowly(kTextPlay, sizeof(kTextPlay) - 1, kDisplayWidth >> 1, 40,
                                  s_numFramesInCurrentState - 1);
            drawCentredTextSlowly(kTextInvaders, sizeof(kTextInvaders) - 1, kDisplayWidth >> 1, 50,
                                  s_numFramesInCurrentState - 30);
            drawCentredTextSlowly(kTextFromSpace, sizeof(kTextFromSpace) - 1, kDisplayWidth >> 1,
                                  65, s_numFramesInCurrentState - 150);
            // Deliberate fall-through

        case State::AttractModeDemo:
            Invader::UpdateAll();
            Bullet::MoveAndDrawAll();
            Explosion::UpdateAndDrawAll();
            if (Buttons::IsJustPressed(Buttons::Id::Fire))
            {
                // Start a new game
                s_numLives = kNumLives;
                s_score    = 0;
                s_level    = 0;
                ButtonRecorder::RecordingEnable(true);
                ChangeState(State::GameStartLevel);
            }
            break;

        case State::GameStartDelay:
            Invader::UpdateAll();
            break;

        case State::Game:
            Invader::UpdateAll();
            s_playerShip.UpdateAndDraw();
            Bullet::MoveAndDrawAll();
            Explosion::UpdateAndDrawAll();
            if (Invader::s_numAlive == 0)
            {
                ChangeState(State::GameInterLevel);
            }
            else if (Invader::HaveAnyReachedTheBottom())
            {
                ChangeState(State::GameOver);
            }
            break;

        case State::GameInterLevel:
            s_playerShip.UpdateAndDraw();
            Bullet::MoveAndDrawAll();
            Explosion::UpdateAndDrawAll();
            break;

        case State::GamePlayerDestroyed:
            s_playerShip.UpdateAndDraw();
            Explosion::UpdateAndDrawAll();
            break;

        case State::GameOver:
            drawCentredTextSlowly(kTextGameOver, sizeof(kTextGameOver) - 1, kDisplayWidth >> 1, 40,
                                  s_numFramesInCurrentState);
            if ((s_timeInCurrentState > Duration(3.f)) && Buttons::IsJustPressed(Buttons::Id::Fire))
            {
                ChangeState(State::AttractModeTitle);
            }
            break;

        default:
            break;
        }
    }
};

const GameState::StateInfo GameState::s_stateInfo[] = {
    { 7.f, State::AttractModeDemo }, // AttractModeTitle
    { 10.f, State::AttractModeTitle }, // AttractModeDemo
    {}, // GameStartLevel
    { 2.f, State::Game }, // GameStartDelay
    {}, // Game
    { 1.f, State::GameStartDelay }, // GamePlayerDestroyed
    { 3.f, State::GameStartLevel }, // GameInterLevel
    { 10.f, State::AttractModeTitle }, // GameOver
};
static_assert(count_of(GameState::s_stateInfo) == (int)GameState::State::Count, "");
GameState::State    GameState::s_currentState            = GameState::State::AttractModeTitle;
GameState::Duration GameState::s_timeInCurrentState      = 0.f;
uint                GameState::s_numFramesInCurrentState = 0;
uint                GameState::s_numLives;
uint                GameState::s_score;
uint                GameState::s_level;

// The body of Bullet::HitSomething is pulled out here, because there's
// a circular dependency between bullets and the things they can hit.
void Bullet::HitSomething()
{
    // We've hit something.
    bool dealtWith = false;
    // But what?
    if (m_dy < 0)
    {
        // Test the invaders
        dealtWith = Invader::HitAny(m_x, m_y);
    }
    else if (m_y >= kPlayerY)
    {
        // Very likely hit the player.
        s_playerShip.m_state = PlayerShip::State::eExploding;
        GameState::ChangeState(GameState::GamePlayerDestroyed);
        dealtWith = true;
    }
    if (!dealtWith)
    {
        Explosion::Add(m_x - 4, m_y - 4, Explosion::Type::Shield);
    }
}

// The PicoVectorscope Demo class
class InvadersFromSpace : public Demo
{
public:
    InvadersFromSpace() : Demo(-1) {}
    void Init();
    void UpdateAndRender(DisplayList& displayList, float dt);
};
static InvadersFromSpace s_invadersFromSpace;

void InvadersFromSpace::Init()
{
    s_rasterDisplay.width            = kDisplayWidth;
    s_rasterDisplay.height           = kDisplayHeight;
    s_rasterDisplay.mode             = DisplayList::RasterDisplay::Mode::e1Bit;
    s_rasterDisplay.scanlineCallback = scanlineCallback;
    // s_rasterDisplay.topLeft = DisplayListVector2(0.25f, 1.f);
    // s_rasterDisplay.bottomRight = DisplayListVector2(0.75f, 0.f);
}

void InvadersFromSpace::UpdateAndRender(DisplayList& displayList, float dt)
{
    ButtonRecorder::Update();
    GameState::Update(GameState::Duration(dt));
    displayList.PushRasterDisplay(s_rasterDisplay);
}

}
