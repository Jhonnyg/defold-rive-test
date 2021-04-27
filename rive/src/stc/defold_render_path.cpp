#include <artboard.hpp>
#include <renderer.hpp>

#include <dmsdk/sdk.h>
#include "../defold_rive_private.h"
#include "defold_render_path.h"

namespace rive
{
    static const dmhash_t VERTEX_STREAM_NAME_POSITION = dmHashString64("position");

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

    DefoldRenderPath::DefoldRenderPath()
    {
        createDMBuffer(&m_BufferCover, 6, "Contour");
    }

    void DefoldRenderPath::reset()
    {
        m_Paths.SetCapacity(0);
        m_PathCommands.SetCapacity(0);
        m_IsDirty = true;
    }

    void DefoldRenderPath::addRenderPath(RenderPath* path, const Mat2D& transform)
    {
        PathDescriptor desc = {path, transform};

        if (m_Paths.Size() == m_Paths.Capacity())
        {
            m_Paths.SetCapacity(m_Paths.Capacity() + 1);
        }

        m_Paths.Push(desc);
    }

    void DefoldRenderPath::fillRule(FillRule value)
    {
        m_FillRule = value;
    }

    void DefoldRenderPath::moveTo(float x, float y)
    {
        if (m_PathCommands.Size() == m_PathCommands.Capacity())
        {
            m_PathCommands.SetCapacity(m_PathCommands.Capacity() + 1);
        }

        m_PathCommands.Push({
            .m_Command = TYPE_MOVE,
            .m_X       = x,
            .m_Y       = y
        });
    }

    void DefoldRenderPath::lineTo(float x, float y)
    {
        if (m_PathCommands.Size() == m_PathCommands.Capacity())
        {
            m_PathCommands.SetCapacity(m_PathCommands.Capacity() + 1);
        }

        m_PathCommands.Push({
            .m_Command = TYPE_LINE,
            .m_X       = x,
            .m_Y       = y
        });
    }

    void DefoldRenderPath::cubicTo(float ox, float oy, float ix, float iy, float x, float y)
    {
        if (m_PathCommands.Size() == m_PathCommands.Capacity())
        {
            m_PathCommands.SetCapacity(m_PathCommands.Capacity() + 1);
        }

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

    void DefoldRenderPath::close()
    {
        if (m_PathCommands.Size() == m_PathCommands.Capacity())
        {
            m_PathCommands.SetCapacity(m_PathCommands.Capacity() + 1);
        }

        m_PathCommands.Push({
            .m_Command = TYPE_CLOSE
        });
    }

    void DefoldRenderPath::computeContour()
    {
        m_IsDirty = false;

        float penX = 0.0f;
        float penY = 0.0f;

        for (int i=0; i < m_PathCommands.Size(); i++)
        {
            const PathCommand pc = m_PathCommands[i];
            switch(pc.m_Command)
            {
                case TYPE_MOVE:
                    penX = pc.m_X;
                    penY = pc.m_Y;
                    break;
                case TYPE_LINE:
                    break;
                case TYPE_CUBIC:
                    break;
                case TYPE_CLOSE:
                    break;
            }
        }

        float* coverPositions;
        getDMBufferPositionStream(m_BufferCover, coverPositions);

        const float p0[] = {-0.5f, -0.5f};
        const float p1[] = { 0.5f, -0.5f};
        const float p2[] = { 0.5f,  0.5f};
        const float p3[] = {-0.5f,  0.5f};

        #define SET_BUFFER_POSITION_V2(b,ix,p) \
            b[ix * 2    ] = p[0]; \
            b[ix * 2 + 1] = p[1];

        SET_BUFFER_POSITION_V2(coverPositions, 0, p0);
        SET_BUFFER_POSITION_V2(coverPositions, 1, p1);
        SET_BUFFER_POSITION_V2(coverPositions, 2, p2);
        SET_BUFFER_POSITION_V2(coverPositions, 3, p2);
        SET_BUFFER_POSITION_V2(coverPositions, 4, p3);
        SET_BUFFER_POSITION_V2(coverPositions, 5, p0);

        #undef SET_BUFFER_POSITION_V2
    }

    void DefoldRenderPath::stencil()
    {
        if (m_Paths.Size() > 0)
        {
            for (int i=0; i < m_Paths.Size(); i++)
            {
                DefoldRenderPath* asDefoldPath = (DefoldRenderPath*) m_Paths[i].m_Path;
                asDefoldPath->stencil();
            }
        }

        if (m_IsDirty)
        {
            computeContour();
        }
    }

    void DefoldRenderPath::cover(const Mat2D& transform)
    {
        if (m_Paths.Size() > 0)
        {
            for (int i=0; i < m_Paths.Size(); i++)
            {
                DefoldRenderPath* asDefoldPath = (DefoldRenderPath*) m_Paths[i].m_Path;

                Mat2D path_transform;
                Mat2D::multiply(path_transform, transform, m_Paths[i].m_Transform);

                asDefoldPath->cover(path_transform);
            }
        }

        if (m_IsDirty)
        {
            computeContour();
        }
    }
}
