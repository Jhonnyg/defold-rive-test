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
}

#endif
