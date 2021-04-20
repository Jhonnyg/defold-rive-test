#include <renderer.hpp>
#include <artboard.hpp>
#include <math/mat2d.hpp>

#include <dmsdk/sdk.h>
#include "defold_rive_private.h"
#include "defold_renderer.h"

namespace rive
{
    void DefoldRenderer::save()
    {
        // dmLogInfo("Save");
    }

    void DefoldRenderer::restore()
    {
        // dmLogInfo("Restore");
    }

    void DefoldRenderer::transform(const Mat2D& transform)
    {
        Mat2D.multiply(m_Transform, m_Transform, transform);
    }

    void DefoldRenderer::drawPath(RenderPath* path, RenderPaint* paint)
    {
        const RiveListenerData data = {
            .m_Action     = ACTION_DRAW_PATH,
            .m_RenderPath = path
        };

        InvokeRiveListener(data);
    }

    void DefoldRenderer::clipPath(RenderPath* path)
    {
        if (m_ClipPaths.Size() == m_ClipPaths.Capacity())
        {
            m_ClipPaths.SetCapacity(m_ClipPaths.Size() + 1);
        }
        m_ClipPaths.Push({.m_Path = path, .m_Transform = m_Transform});
        m_IsDirtyClipping = true;
    }

    void DefoldRenderer::startFrame()
    {
        m_AppliedClips.SetSize(0);
        m_IsDirtyClipping = false;

        InvokeRiveListener({.m_Action = ACTION_START_FRAME});
    }
}
