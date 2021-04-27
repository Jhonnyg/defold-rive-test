#ifndef DEFOLD_RIVE_PRIVATE
#define DEFOLD_RIVE_PRIVATE

namespace rive
{
    struct PathDescriptor
    {
    public:
        RenderPath* m_Path;
        Mat2D       m_Transform;
    };

    enum RiveRenderMode
    {
        MODE_TESSELLATION     = 0,
        MODE_STENCIL_TO_COVER = 1,
    };

    enum RiveCmdType
    {
        CMD_NONE        = 0,
        CMD_START_FRAME = 1,
    };

    struct RiveCmd
    {
        RiveCmdType m_Cmd;
    };

    struct RiveContext
    {
        Artboard*                  m_Artboard;
        dmScript::LuaCallbackInfo* m_Listener;
        dmArray<RiveCmd>           m_Commands;
    };

    enum RiveListenerAction
    {
        ACTION_NONE            = 0,
        ACTION_DRAW_PATH       = 1,
        ACTION_START_FRAME     = 2,
        ACTION_APPLY_CLIPPING  = 3,
        ACTION_APPLY_CLIP_PATH = 4,
    };

    struct RiveListenerData
    {
        RiveListenerAction m_Action;
        RenderPath*        m_RenderPath;
        bool               m_IsClipping;
        bool               m_IsEvenOdd;
    };

    void InvokeRiveListener(const RiveListenerData data);
    void AddCmd(const RiveCmd cmd);

    class DefoldRenderer : public Renderer
    {
    public:
        virtual void startFrame() = 0;
    };

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

#endif
