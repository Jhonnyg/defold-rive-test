#ifndef DEFOLD_TSS_RENDERER
#define DEFOLD_TSS_RENDERER

namespace rive
{
    class DefoldTessellationRenderer : public DefoldRenderer
    {
    public:
        void save()                                         override;
        void restore()                                      override;
        void transform(const Mat2D& transform)              override;
        void drawPath(RenderPath* path, RenderPaint* paint) override;
        void clipPath(RenderPath* path)                     override;
        void startFrame();
    };
}

#endif // DEFOLD_TSS_RENDERER
