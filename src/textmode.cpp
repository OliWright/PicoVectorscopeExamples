#include "extras/tilemap.h"
#include "picovectorscope.h"


static constexpr uint32_t kWidth         = 40;
static constexpr uint32_t kHeight        = 25;
static constexpr uint32_t kVisibleWidth  = 20;
static constexpr uint32_t kVisibleHeight = 15;

static uint8_t s_textDisplay[kHeight][kWidth] = {};

static const uint8_t* rowCallback(int32_t row) { return s_textDisplay[row]; }

static void print(uint32_t x, uint32_t y, const char* message)
{
    const char* src = message;
    char*       dst = (char*)s_textDisplay[y] + x;
    char*       end = (char*)s_textDisplay[0] + (kWidth * kHeight);
    while (*src != 0)
    {
        *(dst++) = *(src++);
        if (dst == end)
        {
            dst = (char*)s_textDisplay[0];
        }
    }
}

static TileMap s_tileMap(
    TileMap::Mode::eText, kWidth, kHeight, rowCallback, kVisibleWidth, kVisibleHeight);

class TextMode : public Demo
{
public:
    TextMode() : Demo(-1) {}
    void Init();
    void UpdateAndRender(DisplayList& displayList, float dt);
};
static TextMode s_textMode;

static void checkerboard()
{
    static uint32_t offset = 0;
    offset                 = (offset + 1) & 0xf;
    for (uint32_t y = 0; y < kHeight; ++y)
    {
        for (uint32_t x = 0; x < kWidth; ++x)
        {
            s_textDisplay[y][x] = ((x + y) & 1) ? 'X' : 'O';
        }
    }
}

void TextMode::Init()
{
    checkerboard();
#if 0
    for (uint i = 0; i < 30; ++i)
    {
        print(SimpleRand() % kWidth, SimpleRand() % kHeight, "Hello World");
    }
#endif
    s_textDisplay[0][0] = 'X';
    for (uint i = 0; i < kHeight; ++i)
    {
        s_textDisplay[i][0] = '0' + (uint8_t) i;
    }
}

void TextMode::UpdateAndRender(DisplayList& displayList, float dt)
{
    static int32_t scrollX = 0;
    static int32_t scrollY = 0;
    //if (Buttons::IsJustPressed(Buttons::Id::Left))
    if (Buttons::IsHeld(Buttons::Id::Left))
    {
        --scrollX;
    }
    //if (Buttons::IsJustPressed(Buttons::Id::Right))
    if (Buttons::IsHeld(Buttons::Id::Right))
    {
        ++scrollX;
    }
    //if (Buttons::IsJustPressed(Buttons::Id::Thrust))
    if (Buttons::IsHeld(Buttons::Id::Thrust))
    {
        --scrollY;
    }
    //if (Buttons::IsJustPressed(Buttons::Id::Fire))
    if (Buttons::IsHeld(Buttons::Id::Fire))
    {
        ++scrollY;
    }
    s_tileMap.SetScrollOffsetPixels(scrollX, scrollY);
    s_tileMap.PushToDisplayList(displayList);

    // Draw the border
    displayList.PushVector(0.f, 0.f, 0.f);
    displayList.PushVector(0.f, 1.f, 0.5f);
    displayList.PushVector(1.f, 1.f, 0.5f);
    displayList.PushVector(1.f, 0.f, 0.5f);
    displayList.PushVector(0.f, 0.f, 0.5f);
}
