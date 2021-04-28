#include <float.h>

#include <artboard.hpp>
#include <renderer.hpp>

#include <tesselator.h>

#include <dmsdk/sdk.h>
#include "../defold_rive_private.h"
#include "defold_tss_render_path.h"

namespace rive
{
    static const dmhash_t VERTEX_STREAM_NAME_POSITION   = dmHashString64("position");
    static const uint32_t COUNTOUR_BUFFER_ELEMENT_COUNT = 512;

    static void createDMBuffer(dmBuffer::HBuffer* buffer, uint32_t elementCount, const char* bufferName)
    {
        dmBuffer::StreamDeclaration streams[] = {
            {VERTEX_STREAM_NAME_POSITION, dmBuffer::VALUE_TYPE_FLOAT32, 2}
        };

        dmBuffer::Result r = dmBuffer::Create(elementCount, streams, sizeof(streams)/sizeof(dmBuffer::StreamDeclaration), buffer);
        if (r != dmBuffer::RESULT_OK)
        {
            dmLogError("Failed to create buffer '%s': %s (%d)",
                bufferName, dmBuffer::GetResultString(r), r);
        }
    }

    static void getDMBufferPositionStream(dmBuffer::HBuffer buffer, float*& positions)
    {
        uint32_t positions_count;
        uint32_t positions_components;
        uint32_t positions_stride;
        dmBuffer::Result r = dmBuffer::GetStream(buffer, VERTEX_STREAM_NAME_POSITION,
            (void**)&positions, &positions_count, &positions_components, &positions_stride);
        if (r != dmBuffer::RESULT_OK)
        {
            dmLogError("Failed to get stream '%s': %s (%d)",
                dmHashReverseSafe64(VERTEX_STREAM_NAME_POSITION),
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
                              int& verticesCount)
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
                int ix               = verticesCount;
                float x              = cubicAt(t2, from[0], fromOut[0], toIn[0], to[0]);
                float y              = cubicAt(t2, from[1], fromOut[1], toIn[1], to[1]);
                vertices[ix * 2    ] = x;
                vertices[ix * 2 + 1] = y;
                verticesCount++;
            }
        }
    }

    DefoldTessellationRenderPath::DefoldTessellationRenderPath()
    {
        createDMBuffer(&m_BufferContour, COUNTOUR_BUFFER_ELEMENT_COUNT, "Contour");
    }

    void DefoldTessellationRenderPath::reset()
    {
        m_Paths.SetCapacity(0);
        m_PathCommands.SetCapacity(0);
        m_IsDirty = true;
    }

    void DefoldTessellationRenderPath::addRenderPath(RenderPath* path, const Mat2D& transform)
    {
        PathDescriptor desc = {path, transform};

        if (m_Paths.Size() == m_Paths.Capacity())
        {
            m_Paths.SetCapacity(m_Paths.Capacity() + 1);
        }

        dmLogInfo("ADD_RENDER_PATH %p to %p", (uintptr_t) path, (uintptr_t) this);

        m_Paths.Push(desc);
    }

    void DefoldTessellationRenderPath::fillRule(FillRule value)
    {
    }

    void DefoldTessellationRenderPath::moveTo(float x, float y)
    {
        if (m_PathCommands.Size() == m_PathCommands.Capacity())
        {
            m_PathCommands.SetCapacity(m_PathCommands.Capacity() + 1);
        }

        dmLogInfo("TYPE_MOVE %p", (uintptr_t) this);

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

        dmLogInfo("TYPE_LINE %p", (uintptr_t) this);

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

        dmLogInfo("TYPE_CUBIC %p", (uintptr_t) this);

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

        dmLogInfo("TYPE_CLOSE %p", (uintptr_t) this);

        m_PathCommands.Push({
            .m_Command = TYPE_CLOSE
        });
    }

    void DefoldTessellationRenderPath::computeContour(float contourError)
    {
        const float minSegmentLength = contourError * contourError;
        const float distTooFar       = contourError;

        float minX = FLT_MAX;
        float minY = FLT_MAX;
        float maxX = -FLT_MAX;
        float maxY = -FLT_MAX;

        float penX      = 0.0f;
        float penY      = 0.0f;
        float penDownX  = 0.0f;
        float penDownY  = 0.0f;
        float isPenDown = false;

        // TODO: something else here
        float vertices[COUNTOUR_BUFFER_ELEMENT_COUNT * 2];
        int verticesCount = 0;

        #define ADD_VERTEX(x,y) \
            { \
                vertices[verticesCount * 2]     = x; \
                vertices[verticesCount * 2 + 1] = y; \
                minX = fmin(minX, x); \
                minY = fmin(minY, y); \
                maxX = fmax(maxX, x); \
                maxY = fmax(maxY, y); \
                verticesCount++; \
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
                    // dmLogInfo("TYPE_MOVE");
                    penX = pc.m_X;
                    penY = pc.m_Y;
                    break;
                case TYPE_LINE:
                    // dmLogInfo("TYPE_LINE");
                    PEN_DOWN()
                    ADD_VERTEX(pc.m_X, pc.m_Y);
                    break;
                case TYPE_CUBIC:
                    // dmLogInfo("TYPE_CUBIC");
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
                        vertices,
                        verticesCount);
                    penX = pc.m_X;
                    penY = pc.m_Y;
                    break;
                case TYPE_CLOSE:
                    // dmLogInfo("TYPE_CLOSE");
                    if (isPenDown)
                    {
                        penX      = penDownX;
                        penY      = penDownY;
                        isPenDown = false;
                    }
                    break;
            }
        }

        // dmLogInfo("verticesCount: %d\n", verticesCount);

        #undef ADD_VERTEX
        #undef PEN_DOWN
    }

    void DefoldTessellationRenderPath::updateContour(float contourError)
    {
        // dmLogInfo("num paths in %p : %d", (uintptr_t) this, m_Paths.Size());

        if (m_Paths.Size() > 0)
        {
            for (int i=0; i < m_Paths.Size(); i++)
            {
                DefoldTessellationRenderPath* asDefoldPath = (DefoldTessellationRenderPath*) m_Paths[i].m_Path;
                asDefoldPath->updateContour(contourError);
            }
        }

        // todo: dirty state
        computeContour(contourError);
    }

    void DefoldTessellationRenderPath::updateTesselation()
    {
        const float contourQuality  = 0.8888888888888889f;
        const float maxContourError = 5.0f;
        const float minContourError = 0.5f;
        const float contourError    = minContourError * contourQuality + maxContourError * (1.0f - contourQuality);

        updateContour(contourError);

        // TODO: custom memory management here
        TESStesselator* tess = tessNewTess(0);
    }

    void DefoldTessellationRenderPath::drawMesh(const Mat2D& transform)
    {
        updateTesselation();
    }
}
