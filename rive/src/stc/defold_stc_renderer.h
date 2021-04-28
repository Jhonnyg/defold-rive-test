#ifndef DEFOLD_RENDERER
#define DEFOLD_RENDERER

namespace rive
{
    class DefoldStCRenderer : public DefoldRenderer
    {
    private:
        bool                    m_IsDirtyClipping;
        bool                    m_IsClipping;
        Mat2D                   m_Transform;
        dmArray<PathDescriptor> m_ClipPaths;
        dmArray<PathDescriptor> m_AppliedClips;

        void applyClipping();
        void applyClipPath(rive::RenderPath* renderPath, Mat2D transform);
    public:
        void save()                                         override;
        void restore()                                      override;
        void transform(const Mat2D& transform)              override;
        void drawPath(RenderPath* path, RenderPaint* paint) override;
        void clipPath(RenderPath* path)                     override;
        void startFrame()                                   override;
    };
}

#endif // DEFOLD_RENDERER
