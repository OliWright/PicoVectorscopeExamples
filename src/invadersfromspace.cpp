#include "picovectorscope.h"

#include "extras/bitmap.h"
#include "extras/bitmapfont.h"

#include "invadersfromspacesprites.h"

namespace InvadersFromSpace
{

static constexpr uint kWidth = 200;
static constexpr uint kHeight = 150;
static constexpr uint kNumLives = 3;
static constexpr uint kNumInvadersX = 9;
static constexpr uint kNumInvadersY = 5;
static constexpr int kInvaderSpacingX = 18;
static constexpr int kInvaderSpacingY = 16;
static constexpr int kShieldSpacingX = 48;
static constexpr int kShieldY = kHeight - 40;
static constexpr int kPlayerY = kShieldY + 24;
static constexpr uint kMaxBullets = 16;

static const char* s_title = "INVADERS\nFROM\nSPACE";
static const BurnLength s_titleTotalBurnLength = CalcBurnLength(s_title);
static const char* s_gameOver = "GAME\nOVER";
static const BurnLength s_gameOverTotalBurnLength = CalcBurnLength(s_gameOver);
static FixedTransform2D s_titleTextTransform;

static uint8_t s_bitmapDisplay[kHeight][kWidth >> 3] = {};
static DisplayList::RasterDisplay s_rasterDisplay;

static void draw16x8BitSprite(const uint16_t* sprite, uint x, uint y)
{
    for(uint i = 0; i < 8; ++i)
    {
        // Manually unrolled to write a 16-bit sprite row
        uint8_t* pixels = s_bitmapDisplay[y + i] + (x >> 3);
        uint16_t spriteRow = sprite[i];

        // Setup to write whatever pixels we can to the first byte
        uint shift = x & 7;
        int32_t spriteShift = 16 - 8 + (int32_t) shift;
        uint8_t maskToWrite = 0xff >> shift;
        
        uint8_t pixelBlock = *pixels;

        // Clear the pixels we're going to write
        pixelBlock &= ~maskToWrite;
        pixelBlock |= (uint8_t) (spriteRow >> spriteShift);
        *pixels++ = pixelBlock;
        pixelBlock = *pixels;

        // The second (middle) byte is always written in its entirity
        // No masking required
        *pixels++ = spriteRow >> (spriteShift - 8);

        spriteShift -= 16;
        if(spriteShift == -8)
        {
            continue;
        }

        // The third byte needs masking
        pixelBlock = *pixels;
        maskToWrite = 0xff << -spriteShift;
        pixelBlock &= ~maskToWrite;
        pixelBlock |= (uint8_t) (spriteRow << -spriteShift);
        *pixels = pixelBlock;
    }
}

static void clear16x8BitRect(uint x, uint y)
{
#if 1
    ClearRectOn1bitDisplay(s_bitmapDisplay[0], kWidth >> 3,  x, y, 16, 8 );
#else
    for(uint i = 0; i < 8; ++i)
    {
        // Manually unrolled to write a 16-bit sprite row
        uint8_t* pixels = s_bitmapDisplay[y + i] + (x >> 3);

        // Setup to write whatever pixels we can to the first byte
        uint shift = x & 7;
        int32_t spriteShift = 16 - 8 + (int32_t) shift;
        uint8_t maskToWrite = 0xff >> shift;
        
        uint8_t pixelBlock = *pixels;

        // Clear the pixels we want clearing
        pixelBlock &= ~maskToWrite;
        *pixels++ = pixelBlock;
        pixelBlock = *pixels;

        // The second (middle) byte is always written in its entirity
        // No masking required
        *pixels++ = 0;

        spriteShift -= 16;
        if(spriteShift == -8)
        {
            continue;
        }

        // The third byte needs masking
        pixelBlock = *pixels;
        maskToWrite = 0xff << -spriteShift;
        pixelBlock &= ~maskToWrite;
        *pixels = pixelBlock;
    }
#endif
}

static void drawText(int x, int y, const char* message, bool centred)
{
    DrawTextTo1bitDisplay(s_bitmapDisplay[0], kWidth>>3, x, y, message, centred);
}

static void clearRect(int x, int y, int width, int height)
{
    ClearRectOn1bitDisplay(s_bitmapDisplay[0], kWidth>>3, x, y, width, height);
}

template<typename T>
static void drawSprite(int x, int y, int width, int height, T sprite)
{
    DrawSpriteTo1bitDisplay(s_bitmapDisplay[0], kWidth>>3, x, y, width, height, sprite[0], sizeof(sprite[0]));
}

static void clearScreen()
{
    uint *pixels = (uint*) s_bitmapDisplay[0];
    const uint* end = pixels + ((kHeight * (kWidth >> 3)) >> 2);
    while(pixels != end)
    {
        *pixels++ = 0;
    }
}

static void drawShields()
{
    int x = (kWidth >> 1) - (kShieldSpacingX >> 1) - kShieldSpacingX - 12;
    for(uint i = 0; i < 4; ++i)
    {
        drawSprite(x, kShieldY, 24, 16, s_shieldSprite);
        //DrawSpriteTo1bitDisplay(s_bitmapDisplay[0], kWidth>>3, x, kShieldY, 24, 16, s_shieldSprite[0], sizeof(s_shieldSprite[0]));
        x += kShieldSpacingX;
    }
}

struct Invader
{
    int16_t x;
    int16_t y;
    uint8_t sprite;
    uint8_t anim;
    bool alive;

    void ClearPixelRect()
    {
        clear16x8BitRect(x, y);
    }

    void Move(int32_t dx, int32_t dy)
    {
        anim ^= 1;
        clear16x8BitRect(x, y);
        x += dx;
        y += dy;
        draw16x8BitSprite(s_invaderSprites[sprite][anim], x, y);
    }

    bool IsHit(int hitX, int hitY) const
    {
        return (hitX >= x) && (hitX < (x + 16)) && (hitY >= y) && (hitY < (y + 8));
    }

    static bool HitAny(int hitX, int hitY)
    {
        // Lazy brute-force iterate over all the invaders to see if we've hit any
        for(uint y = 0; y < kNumInvadersY; ++y)
        {
            for(uint x = 0; x < kNumInvadersX; ++x)
            {
                Invader& invader = s_invaders[y][x];
                if(invader.alive && invader.IsHit(hitX, hitY))
                {
                    invader.alive = false;
                    invader.ClearPixelRect();
                    return true;
                }
            }
        }
        return false;
    }

    static void ConfigureAllForLevel(uint level)
    {
        DestroyAll();
        uint top = 20;
        uint left = 20;
        constexpr uint8_t rowSprites[] = {2,1,1,0,0};
        static_assert(count_of(rowSprites) == kNumInvadersY, "");
        for(uint y = 0; y < kNumInvadersY; ++y)
        {
            for(uint x = 0; x < kNumInvadersX; ++x)
            {
                Invader& invader = s_invaders[y][x];
                invader.x = left + (x * kInvaderSpacingX);
                invader.y = top + (y * kInvaderSpacingY);
                invader.sprite = rowSprites[y];
                invader.anim = 0;
                invader.alive = true;
                draw16x8BitSprite(s_invaderSprites[invader.sprite][0], invader.x, invader.y);
            }
        }
        s_direction = Direction::eDownToLeft;
        s_updateX = kNumInvadersX - 1;
        s_updateY = kNumInvadersY - 1;
        s_limit = top;
        s_numAlive = kNumInvadersX * kNumInvadersY;
    }

    static void DestroyAll()
    {
        for(uint y = 0; y < kNumInvadersY; ++y)
        {
            for(uint x = 0; x < kNumInvadersX; ++x)
            {
                Invader& invader = s_invaders[y][x];
                invader.alive = false;
                invader.ClearPixelRect();
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

    static bool Advance()
    {
        if(s_numAlive == 0)
        {
            return true;
        }
        bool wrapped = false;
        do
        {
            if(++s_updateX == kNumInvadersX)
            {
                s_updateX = 0;
                if(++s_updateY == kNumInvadersY)
                {
                    s_updateY = 0;
                    assert(!wrapped);
                    wrapped = true;
                }
            }
        } while(!s_invaders[s_updateY][s_updateX].alive);
        return wrapped;
    }

    static void UpdateAll()
    {
        if(s_numAlive == 0)
        {
            return;
        }
        if(Advance())
        {
            switch(s_direction)
            {
                case Direction::eLeft:
                    if(s_limit < 4)
                    {
                        s_direction = Direction::eDownToRight;
                        s_limit = 0;
                    }
                    break;
                case Direction::eRight:
                    if(s_limit > (int32_t)(kWidth - 16 - 4))
                    {
                        s_direction = Direction::eDownToLeft;
                        s_limit = 0;
                    }
                    break;
                case Direction::eDownToLeft:
                    s_direction = Direction::eLeft;
                    s_limit = kWidth;
                    break;
                case Direction::eDownToRight:
                    s_direction = Direction::eRight;
                    s_limit = 0;
                    break;
            }
        }

        Invader& invader = s_invaders[s_updateY][s_updateX];
        switch(s_direction)
        {
            case Direction::eLeft:
                invader.Move(-2, 0);
                s_limit = (invader.x < s_limit) ? invader.x : s_limit;
                break;
            case Direction::eRight:
                invader.Move(2, 0);
                s_limit = (invader.x > s_limit) ? invader.x : s_limit;
                break;
            case Direction::eDownToLeft:
            case Direction::eDownToRight:
                invader.Move(0, 2);
                s_limit = (invader.y > s_limit) ? invader.y : s_limit;
                break;
        }
    }

    static Invader s_invaders[kNumInvadersY][kNumInvadersX];
    static Direction s_direction;
    static uint s_updateX;
    static uint s_updateY;
    static int32_t s_limit;
    static uint s_numAlive;
};

Invader Invader::s_invaders[kNumInvadersY][kNumInvadersX];
Invader::Direction Invader::s_direction;
uint Invader::s_updateX;
uint Invader::s_updateY;
int32_t Invader::s_limit;
uint Invader::s_numAlive = 0;

struct Bullet
{
    int x;
    int y;
    int dy;
    bool active = 0;
    int frame;
    int sprite;

    void Activate()
    {
        if(!active)
        {
            //++s_numActive;
            active = true;
        }
    }
    void Destroy()
    {
        if(active)
        {
            //--s_numActive;
            active = false;
            clearRect(x-1, y, 3, 9);
        }
    }

    void MoveAndDraw()
    {
        int testPointY = y + dy;
        if(dy > 0)
        {
            testPointY += 9;
            if(y > (kPlayerY + 8))
            {
                Destroy();
                return;
            }
        }
        else
        {
            if( y < 8)
            {
                Destroy();
                return;
            }
        }
        y += dy;
        frame = (frame + 1) & 3;
        // Test the pixel to see if we've hit something
        uint8_t pixel = s_bitmapDisplay[testPointY][x >> 3] & (0x80 >> (x & 7));
        if(pixel != 0)
        {
            // We've hit something.
            // But what?
            Destroy();
            if(Invader::HitAny(x, testPointY))
            {
                return;
            }
            return;
        }

        drawSprite(x-1, y, 3, 9, s_bulletSprites[sprite][frame]);
    }

    static void MoveAndDrawAll()
    {
        for(uint i = 0; i < kMaxBullets; ++i)
        {
            if(s_bullets[i].active)
            {
                s_bullets[i].MoveAndDraw();
            }
        }
    }

    static Bullet& Get(int idx)
    {
        return s_bullets[idx];
    }

    static Bullet* FindInactive()
    {
        for(uint i = 0; i < kMaxBullets; ++i)
        {
            if(!s_bullets[i].active)
            {
                return s_bullets + i;
            }
        }
        return nullptr;
    }

    static void DestroyAll()
    {
        for(uint i = 0; i < kMaxBullets; ++i)
        {
            Get(i).Destroy();
        }
    }

    static Bullet s_bullets[kMaxBullets];
};

Bullet Bullet::s_bullets[kMaxBullets];

struct PlayerShip
{
    uint x = 0;
    bool drawn = false;

    void Erase()
    {
        if(drawn)
        {
            clearRect(x, kPlayerY, 16, 8);
            drawn = false;
        }
    }

    void Reset()
    {
        Erase();
        x = 0;
    }

    void UpdateAndDraw()
    {
        if(Buttons::IsHeld(Buttons::Id::Left) && (x > 1))
        {
            --x;
        }
        if(Buttons::IsHeld(Buttons::Id::Right) && (x < (kWidth - 16 - 1)))
        {
            ++x;
        }
        if(Buttons::IsJustPressed(Buttons::Id::Fire))
        {
            Bullet* bullet = Bullet::FindInactive();
            if(bullet)
            {
                bullet->x = (x + 8);
                bullet->y = kPlayerY - 9;
                bullet->sprite = 3;
                bullet->dy = -2;
                bullet->Activate();
            }
        }
        drawSprite(x, kPlayerY, 16, 8, s_playerSprite);
    }
};

static PlayerShip s_playerShip;

// High level game state
struct InvadersGameState
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

    static State s_currentState;
    static Duration s_timeInCurrentState;
    static uint s_numFramesInCurrentState;
    static uint s_numLives;
    static uint s_score;
    static uint s_level;

    static void ChangeState(State newState)
    {
        s_timeInCurrentState = 0;
        s_numFramesInCurrentState = 0;
        s_currentState = newState;

        // State initialisation
        switch(s_currentState)
        {
            case State::AttractModeTitle:
                // Nice clean title screen
                clearScreen();
                drawShields();
                Invader::DestroyAll();
                Bullet::DestroyAll();
                break;

            case State::AttractModeDemo:
                clearScreen();
                drawShields();
                Invader::ConfigureAllForLevel(0);;
                break;

            case State::GameStartLevel:
                clearScreen();
                drawShields();
                Invader::ConfigureAllForLevel(s_level);;
                ChangeState(State::GameStartDelay);
                break;

            case State::GameStartDelay:
                if(s_numLives == 0)
                {
                    ChangeState(State::GameOver);
                }
                break;

            case State::Game:
                // Active the player ship
                //s_playerShip.Reset();
                //s_playerShip.m_active = true;
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
        if(stateInfo.m_duration != 0)
        {
            // Auto change-state after timeout
            if(s_timeInCurrentState > stateInfo.m_duration)
            {
                ChangeState(stateInfo.m_autoNextState);
            }
        }

        switch(s_currentState)
        {
            case State::AttractModeTitle:
                if(s_numFramesInCurrentState == 0)
                {
                    drawText(kWidth >> 1, 60, "PLAY", true);
                }
                if(s_numFramesInCurrentState == 30)
                {
                    drawText(kWidth >> 1, 80, "INVADERS", true);
                }
                if(s_numFramesInCurrentState == 120)
                {
                    drawText(kWidth >> 1, 100, "FROM", true);
                }
                if(s_numFramesInCurrentState == 140)
                {
                    drawText(kWidth >> 1, 120, "SPACE", true);
                }
                // Deliberate fall-through

            case State::AttractModeDemo:
                Invader::UpdateAll();
                if(Buttons::IsJustPressed(Buttons::Id::Fire))
                {
                    // Start a new game
                    s_numLives = kNumLives;
                    s_score = 0;
                    s_level = 0;
                    ChangeState(State::GameStartLevel);
                }
                break;

            case State::GameStartDelay:
            {
                Invader::UpdateAll();
            }

            case State::Game:
                Invader::UpdateAll();
                s_playerShip.UpdateAndDraw();
                Bullet::MoveAndDrawAll();
#if 0
                if(!s_playerShip.m_active)
                {
                    // Player ship has been destroyed
                    --s_numLives;
                    ChangeState(State::GamePlayerDestroyed);
                }
                else if(Asteroid::s_numActive == 0)
                {
                    // Level clear
                    ++s_level;
                    ChangeState(State::GameInterLevel);
                }
#endif
                break;

            case State::GameOver:
                if((s_timeInCurrentState > Duration(3.f)) && Buttons::IsJustPressed(Buttons::Id::Fire))
                {
                    ChangeState(State::AttractModeTitle);
                }
                break;

            default:
                break;
        }

    }
};

const InvadersGameState::StateInfo InvadersGameState::s_stateInfo[] = {
    {8.f, State::AttractModeDemo}, // AttractModeTitle
    {7.f, State::AttractModeTitle}, // AttractModeDemo
    {0}, // GameStartLevel
    {2.f, State::Game}, // GameStartDelay
    {0}, // Game
    {1.f, State::GameStartDelay}, // GamePlayerDestroyed
    {3.f, State::GameStartLevel}, // GameInterLevel
    {10.f, State::AttractModeTitle}, // GameOver
};
static_assert(count_of(InvadersGameState::s_stateInfo) == (int) InvadersGameState::State::Count, "");
InvadersGameState::State InvadersGameState::s_currentState = InvadersGameState::State::AttractModeTitle;
InvadersGameState::Duration InvadersGameState::s_timeInCurrentState = 0.f;
uint InvadersGameState::s_numFramesInCurrentState = 0;
uint InvadersGameState::s_numLives;
uint InvadersGameState::s_score;
uint InvadersGameState::s_level;


class InvadersFromSpace : public Demo
{
public:
    InvadersFromSpace() : Demo(-1) {}
    void Init();
    void UpdateAndRender(DisplayList& displayList, float dt);
};
static InvadersFromSpace s_invadersFromSpace;

static const uint8_t* scanlineCallback(uint32_t scanline, void*)
{
    return s_bitmapDisplay[scanline];
}

static void checkerboard()
{
    static uint offset = 0;
    offset  = (offset + 1) & 0xf;
    for(uint y = 0; y < kHeight; ++y)
    {
        uint gy = y >> 3;
        uint8_t pixelBlock = 0;
        for(uint x = 0; x < kWidth; ++x)
        {
            uint gx = (x + offset) >> 3;
            pixelBlock |= ((gx + gy) & 1) << (7 - (x & 7));
            if((x & 7) == 7)
            {
                s_bitmapDisplay[y][x >> 3] = pixelBlock;
                pixelBlock = 0;
            }
        }
    }
}

void InvadersFromSpace::Init()
{
    CalcTextTransform(DisplayListVector2(0.5f, 0.7f), 0.08f, s_titleTextTransform);

    s_rasterDisplay.width = kWidth;
    s_rasterDisplay.height = kHeight;
    s_rasterDisplay.mode = DisplayList::RasterDisplay::Mode::e1Bit;
    s_rasterDisplay.scanlineCallback = scanlineCallback;
    //s_rasterDisplay.topLeft = DisplayListVector2(0.25f, 1.f);
    //s_rasterDisplay.bottomRight = DisplayListVector2(0.75f, 0.f);
}

void InvadersFromSpace::UpdateAndRender(DisplayList& displayList, float dt)
{
    InvadersGameState::Update(InvadersGameState::Duration(dt));
    displayList.PushRasterDisplay(s_rasterDisplay);

    FixedTransform2D transform;
    transform.setAsScale(0.1f);
    transform.translate(FixedTransform2D::Vector2Type(0.1f, 0.9f));
    //TextPrint(displayList, transform, "RASTER\nVECTOR", 1.f);
}
    
}
