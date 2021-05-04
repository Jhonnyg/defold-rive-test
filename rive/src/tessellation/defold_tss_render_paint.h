#ifndef DEFOLD_TSS_RENDER_PAINT
#define DEFOLD_TSS_RENDER_PAINT

namespace rive
{
    class DefoldTessellationRenderPaint : public RenderPaint
    {
    private:
        uint32_t m_Color;
    public:
        void color(unsigned int value) override { m_Color = value; }
        void style(RenderPaintStyle value) override {}
        void thickness(float value) override {}
        void join(StrokeJoin value) override {}
        void cap(StrokeCap value) override {}
        void blendMode(BlendMode value) override {}

        void linearGradient(float sx, float sy, float ex, float ey) override {}
        void radialGradient(float sx, float sy, float ex, float ey) override {}
        void addStop(unsigned int color, float stop) override {}
        void completeGradient() override {}

        void getColorArray(float* rgba);
    };
}

#endif // DEFOLD_TSS_RENDER_PAINT
