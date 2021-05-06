#include <renderer.hpp>

#include <dmsdk/sdk.h>
#include "defold_tss_render_paint.h"

namespace rive
{
    DefoldTessellationRenderPaint::DefoldTessellationRenderPaint()
    : m_Builder(0)
    {
        m_Data = {};
    }

    static void getColorArrayFromUint(unsigned int colorIn, float* rgbaOut)
    {
        rgbaOut[0] = (float)((0x00ff0000 & colorIn) >> 16) / 255.0f;
        rgbaOut[1] = (float)((0x0000ff00 & colorIn) >> 8)  / 255.0f;
        rgbaOut[2] = (float)((0x000000ff & colorIn) >> 0)  / 255.0f;
        rgbaOut[3] = (float)((0xff000000 & colorIn) >> 24) / 255.0f;
    }

    void DefoldTessellationRenderPaint::color(unsigned int value)
    {
        m_Data = {
            .m_FillType = FILL_TYPE_SOLID,
            .m_StopCount = 1
        };

        getColorArrayFromUint(value, &m_Data.m_Colors[0]);
    }

    void DefoldTessellationRenderPaint::linearGradient(float sx, float sy, float ex, float ey)
    {
        m_Builder                 = new DefoldTessellationRenderPaintBuilder();
        m_Builder->m_GradientType = FILL_TYPE_LINEAR;
        m_Builder->m_StartX       = sx;
        m_Builder->m_StartY       = sy;
        m_Builder->m_EndX         = ex;
        m_Builder->m_EndY         = ey;
    }

    void DefoldTessellationRenderPaint::radialGradient(float sx, float sy, float ex, float ey)
    {
        m_Builder                 = new DefoldTessellationRenderPaintBuilder();
        m_Builder->m_GradientType = FILL_TYPE_RADIAL;
        m_Builder->m_StartX       = sx;
        m_Builder->m_StartY       = sy;
        m_Builder->m_EndX         = ex;
        m_Builder->m_EndY         = ey;
    }

    void DefoldTessellationRenderPaint::addStop(unsigned int color, float stop)
    {
        if (m_Builder->m_Stops.Size() == m_Builder->m_Stops.Capacity())
        {
            m_Builder->m_Stops.OffsetCapacity(1);
        }

        m_Builder->m_Stops.Push({
            .m_Color = color,
            .m_Stop  = stop,
        });
    }

    void DefoldTessellationRenderPaint::completeGradient()
    {
        m_Data             = {};
        m_Data.m_FillType  = m_Builder->m_GradientType;
        m_Data.m_StopCount = m_Builder->m_Stops.Size();

        m_Data.m_GradientLimits[0] = m_Builder->m_StartX;
        m_Data.m_GradientLimits[1] = m_Builder->m_EndX;
        m_Data.m_GradientLimits[2] = m_Builder->m_StartY;
        m_Data.m_GradientLimits[3] = m_Builder->m_EndY;

        assert(m_Data.m_StopCount < DefoldTessellationRenderPaintData::MAX_STOPS);
        for (int i = 0; i < m_Builder->m_Stops.Size(); ++i)
        {
            const GradientStop& stop = m_Builder->m_Stops[i];
            getColorArrayFromUint(stop.m_Color, &m_Data.m_Colors[i * 4]);
            m_Data.m_Stops[i] = stop.m_Stop;
        }

        m_Builder->m_Stops.SetSize(0);
        m_Builder->m_Stops.SetCapacity(0);
        delete m_Builder;
    }
}
