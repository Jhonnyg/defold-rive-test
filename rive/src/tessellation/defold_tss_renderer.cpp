#include <renderer.hpp>
#include <artboard.hpp>
#include <command_path.hpp>
#include <math/mat2d.hpp>

#include <dmsdk/sdk.h>
#include "../defold_rive_private.h"
#include "defold_tss_renderer.h"
#include "defold_tss_render_path.h"

namespace rive
{
    void DefoldTessellationRenderer::save()
    {
        StackEntry entry;
        entry.m_Transform = Mat2D(m_Transform);

        // todo: dmArray?
        entry.m_ClipPathsCount = m_ClipPaths.Size();
        memcpy(entry.m_ClipPaths, m_ClipPaths.Begin(), m_ClipPaths.Size());

        if (m_ClipPathStack.Size() == m_ClipPathStack.Capacity())
        {
            m_ClipPathStack.SetCapacity(m_ClipPathStack.Size() + 1);
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
                DefoldTessellationRenderPath* p = (DefoldTessellationRenderPath*) pd.m_Path;
                p->drawMesh(pd.m_Transform);
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
        applyClipping();
        DefoldTessellationRenderPath* p = (DefoldTessellationRenderPath*) path;
        p->drawMesh(m_Transform);
    }

    void DefoldTessellationRenderer::clipPath(RenderPath* path)
    {
        if (m_ClipPaths.Size() == m_ClipPaths.Capacity())
        {
            m_ClipPaths.SetCapacity(m_ClipPaths.Size() + 1);
        }
        m_ClipPaths.Push({.m_Path = path, .m_Transform = m_Transform});

        // dmLogInfo("Pushing %p", (uintptr_t) path);
    }

    void DefoldTessellationRenderer::startFrame()
    {
        m_AppliedClips.SetSize(0);
    }
}
