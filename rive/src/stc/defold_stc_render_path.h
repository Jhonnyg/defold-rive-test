#ifndef DEFOLD_RENDER_PATH
#define DEFOLD_RENDER_PATH

namespace rive
{
    class DefoldStCRenderPath : public RenderPath
    {
    private:
        dmArray<PathCommand>    m_PathCommands;
        dmArray<PathDescriptor> m_Paths;
        dmBuffer::HBuffer       m_BufferCover;
        FillRule                m_FillRule;
        bool                    m_IsDirty;

    public:
        DefoldStCRenderPath();
        void reset()                                                           override;
        void addRenderPath(RenderPath* path, const Mat2D& transform)           override;
        void fillRule(FillRule value)                                          override;
        void moveTo(float x, float y)                                          override;
        void lineTo(float x, float y)                                          override;
        void cubicTo(float ox, float oy, float ix, float iy, float x, float y) override;
        virtual void close()                                                   override;

        uintptr_t getUserData();
        void computeContour();
        void stencil();
        void cover(const Mat2D& transform);

        inline const FillRule getFillRule() { return m_FillRule; }
        inline const dmBuffer::HBuffer getBufferCover() { return m_BufferCover; }
    };
}

#endif // DEFOLD_RENDER_PATH
