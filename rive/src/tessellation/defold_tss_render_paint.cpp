#include <renderer.hpp>

#include "defold_tss_render_paint.h"

namespace rive
{
	void DefoldTessellationRenderPaint::getColorArray(float* rgba)
	{
		rgba[0] = (float)((0x00ff0000 & m_Color) >> 16) / 255.0f;
		rgba[1] = (float)((0x0000ff00 & m_Color) >> 8)  / 255.0f;
		rgba[2] = (float)((0x000000ff & m_Color) >> 0)  / 255.0f;
		rgba[3] = (float)((0xff000000 & m_Color) >> 24) / 255.0f;
	}
}
