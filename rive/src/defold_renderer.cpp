#include <renderer.hpp>
#include <artboard.hpp>

#include <dmsdk/sdk.h>
#include "defold_renderer.h"
#include "defold_rive_private.h"

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
        // dmLogInfo("transform");
    }

    void DefoldRenderer::drawPath(RenderPath* path, RenderPaint* paint)
    {
        // dmLogInfo("DrawPath");
        InvokeRiveListener();
    }

    void DefoldRenderer::clipPath(RenderPath* path)
    {
        // dmLogInfo("ClipPath");
    }

    void DefoldRenderer::startFrame()
    {
        // dmLogInfo("StartFrame");
    }
}
