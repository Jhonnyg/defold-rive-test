#ifndef DEFOLD_NO_OP_RENDER_PAINT
#define DEFOLD_NO_OP_RENDER_PAINT

namespace rive
{
	class DefoldNoOpRenderPaint : public RenderPaint
	{
	public:
		void color(unsigned int value) override {}
		void style(RenderPaintStyle value) override {}
		void thickness(float value) override {}
		void join(StrokeJoin value) override {}
		void cap(StrokeCap value) override {}
		void blendMode(BlendMode value) override {}

		void linearGradient(float sx, float sy, float ex, float ey) override {}
		void radialGradient(float sx, float sy, float ex, float ey) override {}
		void addStop(unsigned int color, float stop) override {}
		void completeGradient() override {}
	};
}

#endif // DEFOLD_NO_OP_RENDER_PAINT
