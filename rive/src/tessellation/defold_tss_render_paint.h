#ifndef DEFOLD_TSS_RENDER_PAINT
#define DEFOLD_TSS_RENDER_PAINT

namespace rive
{
    enum FillType
    {
        FILL_TYPE_NONE   = 0,
        FILL_TYPE_SOLID  = 1,
        FILL_TYPE_LINEAR = 2,
        FILL_TYPE_RADIAL = 3,
    };

    struct GradientStop
    {
        unsigned int m_Color;
        float        m_Stop;
    };

    struct DefoldTessellationRenderPaintBuilder
    {
    public:
        dmArray<GradientStop> m_Stops;
        unsigned int          m_Color;
        FillType              m_GradientType;
        float                 m_StartX;
        float                 m_StartY;
        float                 m_EndX;
        float                 m_EndY;
    };

    struct DefoldTessellationRenderPaintData
    {
    public:
        static const int MAX_STOPS = 16;
        FillType     m_FillType;
        unsigned int m_StopCount;
        float        m_Stops[MAX_STOPS];
        float        m_Colors[MAX_STOPS * 4];
        float        m_GradientLimits[4];
    };

    class DefoldTessellationRenderPaint : public RenderPaint
    {
    private:
        DefoldTessellationRenderPaintBuilder* m_Builder;
        DefoldTessellationRenderPaintData     m_Data;

    public:
        DefoldTessellationRenderPaint();
        void color(unsigned int value)                              override;
        void style(RenderPaintStyle value)                          override {}
        void thickness(float value)                                 override {}
        void join(StrokeJoin value)                                 override {}
        void cap(StrokeCap value)                                   override {}
        void blendMode(BlendMode value)                             override {}
        void linearGradient(float sx, float sy, float ex, float ey) override;
        void radialGradient(float sx, float sy, float ex, float ey) override;
        void addStop(unsigned int color, float stop)                override;
        void completeGradient()                                     override;
        inline DefoldTessellationRenderPaintData getData() { return m_Data; }
    };
}

#endif // DEFOLD_TSS_RENDER_PAINT
