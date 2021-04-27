#include <artboard.hpp>
#include <renderer.hpp>

#include <dmsdk/sdk.h>
#include "../defold_rive_private.h"
#include "defold_tss_render_path.h"

namespace rive
{
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

        m_PathCommands.Push({
            .m_Command = TYPE_CLOSE
        });
    }
}
