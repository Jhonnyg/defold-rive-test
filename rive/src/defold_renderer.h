#ifndef DEFOLD_RENDERER
#define DEFOLD_RENDERER

namespace rive
{
    class DefoldRenderer : public Renderer
    {
    private:
        bool                    m_IsDirtyClipping;
        Mat2D                   m_Transform;
        dmArray<PathDescriptor> m_ClipPaths;
        dmArray<PathDescriptor> m_AppliedClips;
    public:
        void save()                                         override;
        void restore()                                      override;
        void transform(const Mat2D& transform)              override;
        void drawPath(RenderPath* path, RenderPaint* paint) override;
        void clipPath(RenderPath* path)                     override;
        void startFrame();
    };
}

#endif // DEFOLD_RENDERER
