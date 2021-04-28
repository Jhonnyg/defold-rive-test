#ifndef DEFOLD_TSS_RENDER_PATH
#define DEFOLD_TSS_RENDER_PATH

namespace rive
{
    class DefoldTessellationRenderPath : public RenderPath
    {
    private:
        dmBuffer::HBuffer       m_BufferContour;
        dmArray<PathCommand>    m_PathCommands;
        dmArray<PathDescriptor> m_Paths;
        bool                    m_IsDirty;

        void updateTesselation();
        void updateContour(float contourError);
        void computeContour(float contourError);
    public:
        DefoldTessellationRenderPath();
        void reset()                                                           override;
        void addRenderPath(RenderPath* path, const Mat2D& transform)           override;
        void fillRule(FillRule value)                                          override;
        void moveTo(float x, float y)                                          override;
        void lineTo(float x, float y)                                          override;
        void cubicTo(float ox, float oy, float ix, float iy, float x, float y) override;
        virtual void close()                                                   override;

        void drawMesh(const Mat2D& transform);
    };
}

#endif // DEFOLD_TSS_RENDER_PATH
