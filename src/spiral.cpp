#include "picovectorscope.h"

static const ShapeVector2 points[] = {
    ShapeVector2(-1, -1),
    ShapeVector2(1, -1),
    ShapeVector2(1, 1),
    ShapeVector2(-1, 1),
};
static const uint32_t kNumPoints = sizeof(points) / sizeof(points[0]);

static LogChannel s_spiralLog(false);
class Spiral : public Demo
{
public:
    Spiral() : Demo(-4, 150) {}
    void UpdateAndRender(DisplayList& displayList, float dt);
};
static Spiral s_spiral;

void Spiral::UpdateAndRender(DisplayList& displayList, float dt)
{
    static float phase = 0.f;
    phase += dt * 0.1;
    if (phase > (kPi * 2.f))
    {
        phase -= kPi * 2.f;
    }
    Intensity intensity = SinTable::LookUp(phase) * 0.5f + 0.5f;
    intensity = 0.5f;

    constexpr uint32_t kCount = 15;

#if 1
    /*static*/ uint32_t brightIdx = 0;
    if(++brightIdx == kCount)
    {
        brightIdx = 0;
    }
#else
    static float brightCycle = 0.f;
    brightCycle += (dt * 20.f);
    if(brightCycle >= (float) kCount)
    {
        brightCycle -= (float) kCount;
    }
#endif

    PushShapeToDisplayList(displayList, points, kNumPoints, 1.f, true /*closed*/);

    FixedTransform2D transform;
    FixedTransform2D::Vector2Type centre(0.5f, 0.5f);
    SinTableValue s, c;
    SinTable::SinCos(phase, s, c);
    transform.setAsRotation(s, c, centre);
    // float s, c;
    // sincosf(phase, &s, &c);
    // transform.setAsRotation(s, c, centre);
    transform *= 0.35f;
    FixedTransform2D localTransform;
    for (uint32_t i = 0; i < kCount; ++i)
    {
        SinTable::SinCos(phase - (float)i * 0.02f, s, c);
        //sincosf(phase - (float)i * 0.02f, &s, &c);
        localTransform.setAsRotation(s, c, FixedTransform2D::Vector2Type(0.f, 0.f));
        localTransform *= 0.35f;
        float scale = 1.f - ((float) i / kCount);
        localTransform *= scale;
        localTransform.m[0][0] *= 0.75f;
        localTransform.m[1][0] *= 0.75f;
        localTransform.setTranslation(FixedTransform2D::Vector2Type(0.5f, 0.5f));
        intensity = 1.f - ((float) i / kCount);
        intensity = (intensity * intensity * 0.75f) + 0.25f;
        if(i == brightIdx)
        {
            //intensity = 4.f;
        }
        PushShapeToDisplayList(displayList, points, kNumPoints, intensity, true /*closed*/, localTransform);
    }
}
