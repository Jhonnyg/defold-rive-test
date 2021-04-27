#ifndef DEFOLD_TSS_RENDER_PATH
#define DEFOLD_TSS_RENDER_PATH

namespace rive
{
    class DefoldTessellationRenderPath : public RenderPath
    {
    private:
        dmArray<PathCommand>    m_PathCommands;
        dmArray<PathDescriptor> m_Paths;
        bool                    m_IsDirty;

    public:
        void reset()                                                           override;
        void addRenderPath(RenderPath* path, const Mat2D& transform)           override;
        void fillRule(FillRule value)                                          override;
        void moveTo(float x, float y)                                          override;
        void lineTo(float x, float y)                                          override;
        void cubicTo(float ox, float oy, float ix, float iy, float x, float y) override;
        virtual void close()                                                   override;
    };
}

#endif // DEFOLD_TSS_RENDER_PATH
