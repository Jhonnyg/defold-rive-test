#include <float.h>

#include <artboard.hpp>
#include <renderer.hpp>
#include <animation/linear_animation_instance.hpp>

#include <tesselator.h>

#include <dmsdk/sdk.h>
#include "../defold_rive_private.h"
#include "defold_tss_render_path.h"
#include "defold_tss_render_paint.h"

namespace rive
{
    static const dmhash_t VERTEX_STREAM_NAME_POSITION = dmHashString64("position");
    static const dmhash_t VERTEX_STREAM_NAME_NORMAL   = dmHashString64("normal");
    static const dmhash_t VERTEX_STREAM_NAME_UV0      = dmHashString64("uv");

    static void createDMBuffer(dmBuffer::HBuffer* buffer, uint32_t elementCount, const char* bufferName)
    {
        dmBuffer::StreamDeclaration streams[] = {
            {VERTEX_STREAM_NAME_POSITION, dmBuffer::VALUE_TYPE_FLOAT32, 2},
            {VERTEX_STREAM_NAME_NORMAL, dmBuffer::VALUE_TYPE_FLOAT32, 3},
            {VERTEX_STREAM_NAME_UV0, dmBuffer::VALUE_TYPE_FLOAT32, 2},
        };

        dmBuffer::Result r = dmBuffer::Create(elementCount, streams, DM_ARRAY_SIZE(streams), buffer);
        if (r != dmBuffer::RESULT_OK)
        {
            dmLogError("Failed to create buffer '%s': %s (%d)",
                bufferName, dmBuffer::GetResultString(r), r);
        }
    }

    static void getDMBufferStream(dmBuffer::HBuffer buffer, dmhash_t streamHash, float*& stream, uint32_t& count, uint32_t& stride)
    {
        uint32_t components;
        dmBuffer::Result r = dmBuffer::GetStream(buffer, streamHash,
            (void**)&stream, &count, &components, &stride);
        if (r != dmBuffer::RESULT_OK)
        {
            dmLogError("Failed to get stream '%s': %s (%d)",
                dmHashReverseSafe64(streamHash),
                dmBuffer::GetResultString(r), r);
            return;
        }
    }

    static bool tooFar(const Vec2D& a, const Vec2D& b, float distTooFar)
    {
        return fmax(fabs(a[0] - b[0]), fabs(a[1] - b[1])) > distTooFar;
    }

    static void computeHull(const Vec2D& from,
                            const Vec2D& fromOut,
                            const Vec2D& toIn,
                            const Vec2D& to,
                            float t,
                            Vec2D* hull)
    {
        Vec2D::lerp(hull[0], from, fromOut, t);
        Vec2D::lerp(hull[1], fromOut, toIn, t);
        Vec2D::lerp(hull[2], toIn, to, t);

        Vec2D::lerp(hull[3], hull[0], hull[1], t);
        Vec2D::lerp(hull[4], hull[1], hull[2], t);

        Vec2D::lerp(hull[5], hull[3], hull[4], t);
    }

    static bool shouldSplitCubic(const Vec2D& from,
                                 const Vec2D& fromOut,
                                 const Vec2D& toIn,
                                 const Vec2D& to,
                                 float distTooFar)
    {
        Vec2D oneThird, twoThird;
        Vec2D::lerp(oneThird, from, to, 1.0f / 3.0f);
        Vec2D::lerp(twoThird, from, to, 2.0f / 3.0f);
        return tooFar(fromOut, oneThird, distTooFar) || tooFar(toIn, twoThird, distTooFar);
    }

    static float cubicAt(float t, float a, float b, float c, float d)
    {
        float ti = 1.0f - t;
        float value =
            ti * ti * ti * a +
            3.0f * ti * ti * t * b +
            3.0f * ti * t * t * c +
            t * t * t * d;
        return value;
    }

    static void segmentCubic(const Vec2D& from,
                              const Vec2D& fromOut,
                              const Vec2D& toIn,
                              const Vec2D& to,
                              float t1,
                              float t2,
                              float minSegmentLength,
                              float distTooFar,
                              float* vertices,
                              uint32_t& verticesCount)
    {

        if (shouldSplitCubic(from, fromOut, toIn, to, distTooFar))
        {
            float halfT = (t1 + t2) / 2.0f;

            Vec2D hull[6];
            computeHull(from, fromOut, toIn, to, 0.5f, hull);

            segmentCubic(from,
                         hull[0],
                         hull[3],
                         hull[5],
                         t1,
                         halfT,
                         minSegmentLength,
                         distTooFar,
                         vertices,
                         verticesCount);
            segmentCubic(hull[5],
                         hull[4],
                         hull[2],
                         to,
                         halfT,
                         t2,
                         minSegmentLength,
                         distTooFar,
                         vertices,
                         verticesCount);
        }
        else
        {
            float length = Vec2D::distance(from, to);
            if (length > minSegmentLength)
            {
                float x              = cubicAt(t2, from[0], fromOut[0], toIn[0], to[0]);
                float y              = cubicAt(t2, from[1], fromOut[1], toIn[1], to[1]);
                vertices[verticesCount * 2    ] = x;
                vertices[verticesCount * 2 + 1] = y;
                verticesCount++;
            }
        }
    }

    DefoldTessellationRenderPath::DefoldTessellationRenderPath()
    : m_Parent(0)
    , m_IsDirty(true)
    , m_IsShapeDirty(true)
    , m_DrawIndex(0xffffffff)
    {
        createDMBuffer(&m_BufferContour, 6, "Contour");
    }

    DefoldTessellationRenderPath::~DefoldTessellationRenderPath()
    {
        if (m_BufferContour)
        {
            dmBuffer::Destroy(m_BufferContour);
        }
    }

    uintptr_t DefoldTessellationRenderPath::getUserData()
    {
        return (uintptr_t) MODE_TESSELLATION;
    }

    void DefoldTessellationRenderPath::reset()
    {
        m_Paths.SetCapacity(0);
        m_Paths.SetSize(0);
        m_PathCommands.SetCapacity(0);
        m_PathCommands.SetSize(0);
        m_IsDirty = true;
        m_IsShapeDirty = true;
    }

    void DefoldTessellationRenderPath::addRenderPath(RenderPath* path, const Mat2D& transform)
    {
        PathDescriptor desc = {path, transform};

        if (m_Paths.Size() == m_Paths.Capacity())
        {
            m_Paths.OffsetCapacity(1);
        }

        // dmLogInfo("ADD_RENDER_PATH %p to %p", (uintptr_t) path, (uintptr_t) this);

        DefoldTessellationRenderPath* child = (DefoldTessellationRenderPath*) path;
        child->setParent(this);

        m_Paths.Push(desc);
    }

    void DefoldTessellationRenderPath::setDrawIndex(uint32_t* drawIndex)
    {
        m_DrawIndex = *drawIndex;
        *drawIndex += 1;

        for (int i = 0; i < m_Paths.Size(); ++i)
        {
            DefoldTessellationRenderPath* child = (DefoldTessellationRenderPath*) m_Paths[i].m_Path;
            child->setDrawIndex(drawIndex);
        }
    }

    void DefoldTessellationRenderPath::fillRule(FillRule value)
    {
        m_FillRule = value;
    }

    void DefoldTessellationRenderPath::moveTo(float x, float y)
    {
        if (m_PathCommands.Size() == m_PathCommands.Capacity())
        {
            m_PathCommands.OffsetCapacity(1);
        }

        // dmLogInfo("TYPE_MOVE %p", (uintptr_t) this);

        m_PathCommands.Push({
            .m_Command = TYPE_MOVE,
            .m_X       = x,
            .m_Y       = y
        });
    }

    void DefoldTessellationRenderPath::lineTo(float x, float y)
    {
        if (m_PathCommands.Size() == m_PathCommands.Capacity())
        {
            m_PathCommands.SetCapacity(m_PathCommands.Capacity() + 1);
        }

        // dmLogInfo("TYPE_LINE %p", (uintptr_t) this);

        m_PathCommands.Push({
            .m_Command = TYPE_LINE,
            .m_X       = x,
            .m_Y       = y
        });
    }

    void DefoldTessellationRenderPath::cubicTo(float ox, float oy, float ix, float iy, float x, float y)
    {
        if (m_PathCommands.Size() == m_PathCommands.Capacity())
        {
            m_PathCommands.SetCapacity(m_PathCommands.Capacity() + 1);
        }

        // dmLogInfo("TYPE_CUBIC %p", (uintptr_t) this);

        m_PathCommands.Push({
            .m_Command = TYPE_CUBIC,
            .m_X       = x,
            .m_Y       = y,
            .m_OX      = ox,
            .m_OY      = oy,
            .m_IX      = ix,
            .m_IY      = iy,
        });
    }

    void DefoldTessellationRenderPath::close()
    {
        if (m_PathCommands.Size() == m_PathCommands.Capacity())
        {
            m_PathCommands.SetCapacity(m_PathCommands.Capacity() + 1);
        }

        // dmLogInfo("TYPE_CLOSE %p", (uintptr_t) this);

        m_PathCommands.Push({
            .m_Command = TYPE_CLOSE
        });
    }

    void DefoldTessellationRenderPath::computeContour(float contourError)
    {
        const float minSegmentLength = contourError * contourError;
        const float distTooFar       = contourError;

        m_IsDirty  = false;

        float minX = FLT_MAX;
        float minY = FLT_MAX;
        float maxX = -FLT_MAX;
        float maxY = -FLT_MAX;

        float penX      = 0.0f;
        float penY      = 0.0f;
        float penDownX  = 0.0f;
        float penDownY  = 0.0f;
        float isPenDown = false;

        m_VertexCount = 0;

        #define ADD_VERTEX(x,y) \
            { \
                m_VertexData[m_VertexCount * 2]     = x; \
                m_VertexData[m_VertexCount * 2 + 1] = y; \
                minX = fmin(minX, x); \
                minY = fmin(minY, y); \
                maxX = fmax(maxX, x); \
                maxY = fmax(maxY, y); \
                m_VertexCount++; \
            }

        #define PEN_DOWN() \
            if (!isPenDown) \
            { \
                isPenDown = true; \
                penDownX = penX; \
                penDownY = penY; \
                ADD_VERTEX(penX, penY); \
            }

        // dmLogInfo("Commands (%p):", (uintptr_t) this);
        for (int i=0; i < m_PathCommands.Size(); i++)
        {
            const PathCommand& pc = m_PathCommands[i];
            switch(pc.m_Command)
            {
                case TYPE_MOVE:
                    penX = pc.m_X;
                    penY = pc.m_Y;
                    break;
                case TYPE_LINE:
                    PEN_DOWN()
                    ADD_VERTEX(pc.m_X, pc.m_Y);
                    break;
                case TYPE_CUBIC:
                    PEN_DOWN()
                    segmentCubic(
                        Vec2D(penX, penY),
                        Vec2D(pc.m_OX, pc.m_OY),
                        Vec2D(pc.m_IX, pc.m_IY),
                        Vec2D(pc.m_X, pc.m_Y),
                        0.0f,
                        1.0f,
                        minSegmentLength,
                        distTooFar,
                        m_VertexData,
                        m_VertexCount);
                    penX = pc.m_X;
                    penY = pc.m_Y;
                    break;
                case TYPE_CLOSE:
                    if (isPenDown)
                    {
                        penX      = penDownX;
                        penY      = penDownY;
                        isPenDown = false;
                    }
                    break;
            }
        }

        #undef ADD_VERTEX
        #undef PEN_DOWN
    }

    void DefoldTessellationRenderPath::updateContour(float contourError)
    {
        if (m_Paths.Size() > 0)
        {
            for (int i=0; i < m_Paths.Size(); i++)
            {
                DefoldTessellationRenderPath* asDefoldPath = (DefoldTessellationRenderPath*) m_Paths[i].m_Path;
                asDefoldPath->updateContour(contourError);
            }
        }

        if (m_IsDirty)
        {
            computeContour(contourError);
        }
    }

    void DefoldTessellationRenderPath::addContours(TESStesselator* tess, const Mat2D& m)
    {
        m_IsShapeDirty = false;

        if (m_Paths.Size() > 0)
        {
            for (int i = 0; i < m_Paths.Size(); ++i)
            {
                DefoldTessellationRenderPath* asDefoldPath = (DefoldTessellationRenderPath*) m_Paths[i].m_Path;
                asDefoldPath->addContours(tess, m_Paths[i].m_Transform);
            }
            return;
        }

        const int numComponents = 2;
        const int stride        = sizeof(float) * numComponents;

        Mat2D identity;
        if (identity == m)
        {
            tessAddContour(tess, numComponents, m_VertexData, stride, m_VertexCount);
        }
        else
        {
            const float m0 = m[0];
            const float m1 = m[1];
            const float m2 = m[2];
            const float m3 = m[3];
            const float m4 = m[4];
            const float m5 = m[5];

            float transformBuffer[COUNTOUR_BUFFER_ELEMENT_COUNT * numComponents];

            for (int i = 0; i < m_VertexCount * numComponents; i += numComponents)
            {
                float x                = m_VertexData[i];
                float y                = m_VertexData[i + 1];
                transformBuffer[i    ] = m0 * x + m2 * y + m4;
                transformBuffer[i + 1] = m1 * x + m3 * y + m5;
            }

            tessAddContour(tess, numComponents, transformBuffer, stride, m_VertexCount);
        }
    }

    bool DefoldTessellationRenderPath::isShapeDirty()
    {
        bool dirty = m_IsShapeDirty;
        if (dirty)
        {
            return true;
        }

        if (m_Paths.Size() > 0)
        {
            for (int i = 0; i < m_Paths.Size(); ++i)
            {
                DefoldTessellationRenderPath* asDefoldPath = (DefoldTessellationRenderPath*) m_Paths[i].m_Path;
                if (asDefoldPath->isShapeDirty())
                {
                    dirty = true;
                    break;
                }
            }
        }

        return dirty;
    }

    void DefoldTessellationRenderPath::updateTesselation()
    {
        const float contourQuality  = 0.8888888888888889f;
        const float maxContourError = 5.0f;
        const float minContourError = 0.5f;
        const float contourError    = minContourError * contourQuality + maxContourError * (1.0f - contourQuality);

        updateContour(contourError);

        if (!isShapeDirty())
        {
            return;
        }

        // TODO: custom memory management here + move globally or something else
        TESStesselator* tess = tessNewTess(0);

        Mat2D identity;
        addContours(tess, identity);

        const int windingRule = m_FillRule == FillRule::nonZero ? TESS_WINDING_NONZERO : TESS_WINDING_ODD;
        const int elementType = TESS_POLYGONS;
        const int polySize    = 3;
        const int vertexSize  = 2;

        if (tessTesselate(tess, windingRule, elementType, polySize, vertexSize, 0))
        {
            int              tessVerticesCount = tessGetVertexCount(tess);
            int              tessElementsCount = tessGetElementCount(tess);
            const TESSreal*  tessVertices      = tessGetVertices(tess);
            const TESSindex* tessElements      = tessGetElements(tess);
            const uint32_t   bufferItemCount   = polySize * tessElementsCount;

            float*   dmPositions      = 0;
            uint32_t dmPositionStride = 0;
            uint32_t dmPositionsCount = 0;
            getDMBufferStream(m_BufferContour, VERTEX_STREAM_NAME_POSITION, dmPositions, dmPositionsCount, dmPositionStride);

            if (dmPositionsCount != bufferItemCount)
            {
                dmBuffer::Destroy(m_BufferContour);
                createDMBuffer(&m_BufferContour, bufferItemCount , "Contour");
                getDMBufferStream(m_BufferContour, VERTEX_STREAM_NAME_POSITION, dmPositions, dmPositionsCount, dmPositionStride);
            }

            float*   dmUv0       = 0;
            uint32_t dmUv0Stride = 0;
            uint32_t dmUv0Count  = 0;
            getDMBufferStream(m_BufferContour, VERTEX_STREAM_NAME_UV0, dmUv0, dmUv0Count, dmUv0Stride);

            int dmVx = 0;
            int dmUvx = 0;
            int dmCount = 0;
            for (int i = 0; i < tessElementsCount; ++i)
            {
                const TESSindex* poly = &tessElements[i * polySize];

                for (int j = 0; j < polySize; ++j)
                {
                    if (poly[j] == TESS_UNDEF) break;

                    const TESSreal* vx    = &tessVertices[poly[j]*vertexSize];
                    dmPositions[dmVx    ] = vx[0];
                    dmPositions[dmVx + 1] = vx[1];
                    dmUv0[dmUvx    ]      = vx[0];
                    dmUv0[dmUvx + 1]      = vx[1];
                    dmUvx                += dmUv0Stride;
                    dmVx                 += dmPositionStride;
                    dmCount              += 2;
                }
            }

            AddCmd({.m_Cmd = CMD_UPDATE_TESSELATION, .m_RenderPath = this});
        }
        else
        {
            dmLogError("Unable to tesselate path ‰p", this);
        }

        tessDeleteTess(tess);
    }

    void DefoldTessellationRenderPath::drawMesh(const Mat2D& transform)
    {
        updateTesselation();
        m_Transform = transform;
    }
}
