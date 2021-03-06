#ifndef DEFOLD_TSS_RENDERER
#define DEFOLD_TSS_RENDERER

namespace rive
{
    class DefoldTessellationRenderer : public DefoldRenderer
    {
    private:
        static const int STACK_ENTRY_MAX_CLIP_PATHS = 16;

        struct StackEntry
        {
            Mat2D          m_Transform;
            PathDescriptor m_ClipPaths[STACK_ENTRY_MAX_CLIP_PATHS];
            uint8_t        m_ClipPathsCount;
        };

        dmArray<StackEntry>     m_ClipPathStack;
        dmArray<PathDescriptor> m_ClipPaths;
        dmArray<PathDescriptor> m_AppliedClips;
        Mat2D                   m_Transform;
        uint32_t                m_DrawIndex;

        void applyClipping();
        void printStack(const char* label);
    public:
        void save()                                         override;
        void restore()                                      override;
        void transform(const Mat2D& transform)              override;
        void drawPath(RenderPath* path, RenderPaint* paint) override;
        void clipPath(RenderPath* path)                     override;
        void startFrame()                                   override;
    };
}

#endif // DEFOLD_TSS_RENDERER
