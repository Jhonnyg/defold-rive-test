#include <renderer.hpp>
#include <artboard.hpp>
#include <command_path.hpp>
#include <math/mat2d.hpp>

#include <dmsdk/sdk.h>
#include "defold_rive_private.h"
#include "defold_renderer.h"
#include "defold_render_path.h"

namespace rive
{
    void DefoldRenderer::applyClipPath(RenderPath* renderPath, Mat2D transform)
    {
        /*
        InvokeRiveListener({
            .m_Action     = ACTION_APPLY_CLIP_PATH,
            .m_RenderPath = renderPath,
            .m_IsClipping = m_IsClipping,
            .m_IsEvenOdd  = ((DefoldRenderPath*) renderPath)->getFillRule() == FillRule::evenOdd,
        });
        */

        m_IsClipping = true;
    }

    void DefoldRenderer::applyClipping()
    {
        bool same = true;
        m_IsDirtyClipping = false;

        if (m_ClipPaths.Size() == m_AppliedClips.Size())
        {
            for (int i = 0; i < m_ClipPaths.Size(); ++i)
            {
                const PathDescriptor& pdA = m_ClipPaths[i];
                const PathDescriptor& pdB = m_AppliedClips[i];

                if (pdA.m_Path != pdB.m_Path || (pdA.m_Transform == pdB.m_Transform))
                {
                    same = false;
                    break;
                }
            }
        }
        else
        {
            same = false;
        }

        if (same)
        {
            return;
        }

        // InvokeRiveListener({.m_Action = ACTION_APPLY_CLIPPING});

        if (m_ClipPaths.Size() > 0)
        {
            for (int i = 0; i < m_ClipPaths.Size(); ++i)
            {
                const PathDescriptor& pd = m_ClipPaths[i];
                applyClipPath(pd.m_Path, pd.m_Transform);
            }
        }

        m_IsClipping = false;

        // todo: better way of copying this array?
        m_AppliedClips.SetCapacity(m_ClipPaths.Capacity());
        m_AppliedClips.SetSize(0);
        m_AppliedClips.PushArray(m_ClipPaths.Begin(), m_ClipPaths.Size());
    }

    void DefoldRenderer::save()
    {
    }

    void DefoldRenderer::restore()
    {
    }

    void DefoldRenderer::transform(const Mat2D& transform)
    {
        Mat2D::multiply(m_Transform, m_Transform, transform);
    }

    void DefoldRenderer::drawPath(RenderPath* path, RenderPaint* paint)
    {
        if (m_IsDirtyClipping)
        {
            applyClipping();
        }

        /*
        InvokeRiveListener({
            .m_Action     = ACTION_DRAW_PATH,
            .m_RenderPath = path
        });
        */
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

        AddCmd({.m_Cmd = CMD_START_FRAME});
    }
}
