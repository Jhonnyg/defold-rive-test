#ifndef DEFOLD_RIVE_PRIVATE
#define DEFOLD_RIVE_PRIVATE

namespace rive
{
    struct RiveContext
    {
        Artboard*                  m_Artboard;
        dmScript::LuaCallbackInfo* m_Listener;
    };

    void InvokeRiveListener();
}

#endif
