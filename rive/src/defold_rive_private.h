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

    struct RiveContext
    {
        Artboard*                  m_Artboard;
        dmScript::LuaCallbackInfo* m_Listener;
    };

    enum RiveListenerAction
    {
        ACTION_NONE        = 0,
        ACTION_DRAW_PATH   = 1,
        ACTION_START_FRAME = 2,
    };

    struct RiveListenerData
    {
        RiveListenerAction m_Action;
        RenderPath*        m_RenderPath;
    };

    void InvokeRiveListener(const RiveListenerData data);
}

#endif
