#ifndef DEFOLD_TSS_RENDER_PATH
#define DEFOLD_TSS_RENDER_PATH

namespace rive
{
    class DefoldTessellationRenderPath : public RenderPath
    {
    private:
        static const uint32_t COUNTOUR_BUFFER_ELEMENT_COUNT = 512;

        float                   m_VertexData[COUNTOUR_BUFFER_ELEMENT_COUNT * 2];
        dmBuffer::HBuffer       m_BufferContour;
        dmArray<PathCommand>    m_PathCommands;
        dmArray<PathDescriptor> m_Paths;
        RenderPath*             m_Parent;
        Mat2D                   m_Transform;
        FillRule                m_FillRule;
        uint32_t                m_VertexCount;
        uint32_t                m_DrawIndex;
        bool                    m_IsDirty;
        bool                    m_IsShapeDirty;

        void updateTesselation();
        void updateContour(float contourError);
        void computeContour(float contourError);
        void addContours(TESStesselator* tess, const Mat2D& m);
        bool isShapeDirty();
    public:
        DefoldTessellationRenderPath();
        ~DefoldTessellationRenderPath();
        void         reset()                                                           override;
        void         addRenderPath(RenderPath* path, const Mat2D& transform)           override;
        void         fillRule(FillRule value)                                          override;
        void         moveTo(float x, float y)                                          override;
        void         lineTo(float x, float y)                                          override;
        void         cubicTo(float ox, float oy, float ix, float iy, float x, float y) override;
        virtual void close()                                                   override;
        void         drawMesh(const Mat2D& transform);
        uintptr_t    getUserData();
        void         setDrawIndex(uint32_t* drawIndex);

        inline const uint32_t          getDrawIndex()           { return m_DrawIndex; }
        inline const Mat2D             getTransform()           { return m_Transform; }
        inline const dmBuffer::HBuffer getContourBuffer()       { return m_BufferContour; }
        inline RenderPath*             getParent()              { return m_Parent; }
        inline void                    setParent(RenderPath* p) { m_Parent = p; }
    };
}

#endif // DEFOLD_TSS_RENDER_PATH
