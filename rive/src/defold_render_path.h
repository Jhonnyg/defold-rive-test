#ifndef DEFOLD_RENDER_PATH
#define DEFOLD_RENDER_PATH

namespace rive
{
    class DefoldRenderPath : public RenderPath
    {
    private:
        struct PathDescriptor
        {
        public:
            RenderPath* m_Path;
            Mat2D       m_Transform;
        };

        enum PathCommandType
        {
            TYPE_MOVE  = 0,
            TYPE_LINE  = 1,
            TYPE_CUBIC = 2,
            TYPE_CLOSE = 3,
        };

        struct PathCommand
        {
        public:
            PathCommandType m_Command;
            float           m_X;
            float           m_Y;
            float           m_OX;
            float           m_OY;
            float           m_IX;
            float           m_IY;
        };

        dmArray<PathCommand>    m_PathCommands;
        dmArray<PathDescriptor> m_Paths;
        dmBuffer::HBuffer       m_BufferCover;
        FillRule                m_FillRule;
        bool                    m_IsDirty;

    public:
        DefoldRenderPath();
        void reset()                                                           override;
        void addRenderPath(RenderPath* path, const Mat2D& transform)           override;
        void fillRule(FillRule value)                                          override;
        void moveTo(float x, float y)                                          override;
        void lineTo(float x, float y)                                          override;
        void cubicTo(float ox, float oy, float ix, float iy, float x, float y) override;
        virtual void close()                                                   override;

        void computeContour();
        void stencil();
    };
}

#endif // DEFOLD_RENDER_PATH
