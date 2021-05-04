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

    enum PathCommandType
    {
        TYPE_MOVE  = 0,
        TYPE_LINE  = 1,
        TYPE_CUBIC = 2,
        TYPE_CLOSE = 3,
    };

    struct PathCommand
    {
    public:
        PathCommandType m_Command;
        float           m_X;
        float           m_Y;
        float           m_OX;
        float           m_OY;
        float           m_IX;
        float           m_IY;
    };

    enum RiveRenderMode
    {
        MODE_NONE             = 0,
        MODE_TESSELLATION     = 1,
        MODE_STENCIL_TO_COVER = 2,
    };

    enum RiveCmdType
    {
        CMD_NONE               = 0,
        CMD_START_FRAME        = 1,
        CMD_UPDATE_TESSELATION = 2,
        CMD_DRAW_PATH          = 3,
        CMD_UPDATE_DRAW_INDEX  = 4,
    };

    struct RiveCmd
    {
        RiveCmdType  m_Cmd;
        RenderPath*  m_RenderPath;
        RenderPaint* m_RenderPaint;
    };

    struct RiveContext
    {
        Artboard*                  m_Artboard;
        LinearAnimationInstance*   m_ArtboardAnimation;
        dmScript::LuaCallbackInfo* m_Listener;
        dmArray<RiveCmd>           m_Commands;
    };

    // We'll probably remove the listener
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
        virtual void              startFrame() = 0;
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
