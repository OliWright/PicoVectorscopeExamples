#include "picovectorscope.h"

#include "image.hpp"

static constexpr uint32_t kWidth = 300;
static constexpr uint32_t kHeight = 200;

static DisplayList::RasterDisplay s_rasterDisplay;
extern bool g_overrideRasterSpeed;

class Image : public Demo
{
public:
    Image() : Demo(-3, 60) {}
    void Init();
    void Start()
    {
        g_overrideRasterSpeed = true;
    }
    void End()
    {
        g_overrideRasterSpeed = false;
    }
    void UpdateAndRender(DisplayList& displayList, float dt);
};
static Image s_image;

static const uint8_t* scanlineCallback(uint32_t scanline, void*)
{
    return s_imageData[scanline];
}

void Image::Init()
{
    s_rasterDisplay.width = kWidth;
    s_rasterDisplay.height = kHeight;
    s_rasterDisplay.mode = DisplayList::RasterDisplay::Mode::e8BitGamma;
    s_rasterDisplay.scanlineCallback = scanlineCallback;
}

void Image::UpdateAndRender(DisplayList& displayList, float dt)
{
    displayList.PushRasterDisplay(s_rasterDisplay);

    FixedTransform2D transform;
    transform.setAsScale(0.05f);
    transform.translate(FixedTransform2D::Vector2Type(0.3f, 0.7f));
    TextPrint(displayList, transform, "VELOCIRASTER", 0.8f);
}
