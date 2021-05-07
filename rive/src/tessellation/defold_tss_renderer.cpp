#include <renderer.hpp>
#include <artboard.hpp>
#include <command_path.hpp>
#include <math/mat2d.hpp>
#include <animation/linear_animation_instance.hpp>

#include <tesselator.h>

#include <dmsdk/sdk.h>
#include "../defold_rive_private.h"
#include "defold_tss_renderer.h"
#include "defold_tss_render_path.h"
#include "defold_tss_render_paint.h"

namespace rive
{
    void DefoldTessellationRenderer::printStack(const char* label)
    {
        dmLogInfo("printStack (%s)", label);
        dmLogInfo("================");

        for (int i = 0; i < m_ClipPathStack.Size(); ++i)
        {
            const StackEntry entry = m_ClipPathStack[i];

            dmLogInfo("Entry %d", i);

            for (int j = 0; j < entry.m_ClipPathsCount; ++j)
            {
                dmLogInfo("  %d : %p", j, (uintptr_t) entry.m_ClipPaths[j].m_Path);
            }
        }
    }

    void DefoldTessellationRenderer::save()
    {
        StackEntry entry = {};
        entry.m_Transform = Mat2D(m_Transform);

        assert(m_ClipPaths.Size() < STACK_ENTRY_MAX_CLIP_PATHS);

        // todo: dmArray?
        entry.m_ClipPathsCount = m_ClipPaths.Size();
        memcpy(entry.m_ClipPaths, m_ClipPaths.Begin(), m_ClipPaths.Size() * sizeof(PathDescriptor));

        if (m_ClipPathStack.Size() == m_ClipPathStack.Capacity())
        {
            m_ClipPathStack.OffsetCapacity(1);
        }

        m_ClipPathStack.Push(entry);
    }

    void DefoldTessellationRenderer::restore()
    {
        const StackEntry last = m_ClipPathStack.Back();
        m_Transform = last.m_Transform;
        m_ClipPaths.SetSize(0);
        m_ClipPaths.SetCapacity(last.m_ClipPathsCount);
        m_ClipPaths.PushArray(last.m_ClipPaths, last.m_ClipPathsCount);
        m_ClipPathStack.Pop();
    }

    void DefoldTessellationRenderer::applyClipping()
    {
        bool same = true;
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

        if (m_ClipPaths.Size() > 0)
        {
            for (int i = 0; i < m_ClipPaths.Size(); ++i)
            {
                const PathDescriptor& pd = m_ClipPaths[i];
                const RiveRenderMode rm = (RiveRenderMode) pd.m_Path->getUserData();
                if (rm == MODE_TESSELLATION)
                {
                    DefoldTessellationRenderPath* p = (DefoldTessellationRenderPath*) pd.m_Path;
                    //p->drawMesh(pd.m_Transform);
                }
            }

            m_AppliedClips.SetCapacity(m_ClipPaths.Capacity());
            m_AppliedClips.SetSize(0);
            m_AppliedClips.PushArray(m_ClipPaths.Begin(), m_ClipPaths.Size());
        }
    }

    void DefoldTessellationRenderer::transform(const Mat2D& transform)
    {
        Mat2D::multiply(m_Transform, m_Transform, transform);
    }

    void DefoldTessellationRenderer::drawPath(RenderPath* path, RenderPaint* paint)
    {
        const RiveRenderMode rm = (RiveRenderMode) path->getUserData();
        if (rm == MODE_TESSELLATION)
        {
            DefoldTessellationRenderPath*   p = (DefoldTessellationRenderPath*) path;
            DefoldTessellationRenderPaint* rp = (DefoldTessellationRenderPaint*) paint;

            if (rp->getStyle() != RenderPaintStyle::fill || !rp->isVisible())
            {
                return;
            }

            p->setDrawIndex(&m_DrawIndex);
            applyClipping();

            AddCmd({.m_Cmd         = CMD_DRAW_PATH,
                    .m_RenderPath  = path,
                    .m_RenderPaint = paint});

            p->drawMesh(m_Transform);
        }
        else
        {
            applyClipping();
        }
    }

    void DefoldTessellationRenderer::clipPath(RenderPath* path)
    {
        if (m_ClipPaths.Size() == m_ClipPaths.Capacity())
        {
            m_ClipPaths.OffsetCapacity(1);
        }

        m_ClipPaths.Push({.m_Path = path, .m_Transform = m_Transform});
    }

    void DefoldTessellationRenderer::startFrame()
    {
        m_AppliedClips.SetSize(0);
        m_DrawIndex = 0;
    }
}
