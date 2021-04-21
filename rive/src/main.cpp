#define LIB_NAME "DefoldRiveTest"
#define MODULE_NAME "rive"

// Rive
#include <artboard.hpp>
#include <shapes/rectangle.hpp>
#include <shapes/shape.hpp>
#include <file.hpp>
#include <renderer.hpp>

// todo: remove
#include "no_op_renderer.hpp"

// Defold
#include <dmsdk/sdk.h>
#include "defold_rive_private.h"
#include "defold_render_path.h"
#include "defold_renderer.h"

namespace rive
{
    RenderPaint* makeRenderPaint() { return new NoOpRenderPaint();  }
    RenderPath* makeRenderPath()   { return new DefoldRenderPath(); }

    static DefoldRenderer* g_Renderer = 0;
    static RiveContext*    g_Context = 0;

    void InvokeRiveListener(const RiveListenerData data)
    {
        rive::RiveContext* ctx = rive::g_Context;
        if (!dmScript::IsCallbackValid(ctx->m_Listener))
        {
            dmLogWarning("No callback function set!");
            return;
        }

        if (!dmScript::SetupCallback(ctx->m_Listener))
        {
            return;
        }
        lua_State* L = dmScript::GetCallbackLuaContext(ctx->m_Listener);
        lua_newtable(L);

        lua_pushinteger(L, data.m_Action);
        lua_setfield(L, -2, "action");

        lua_pushlightuserdata(L, (void*) data.m_RenderPath);
        lua_setfield(L, -2, "render_path");

        lua_pushboolean(L, data.m_IsClipping);
        lua_setfield(L, -2, "is_clipping");

        lua_pushboolean(L, data.m_IsEvenOdd);
        lua_setfield(L, -2, "is_even_odd");

        dmScript::PCall(L, 2, 0);
        dmScript::TeardownCallback(ctx->m_Listener);
    }
}

static rive::Artboard* LoadArtBoardFromFile(const char* path)
{
    FILE* fp = fopen(path, "r");
    if (fp == 0)
    {
        dmLogError("Failed to open file from '%s'", path);
        return 0;
    }

    fseek(fp, 0, SEEK_END);
    size_t fileBytesLength = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    uint8_t* fileBytes = new uint8_t[fileBytesLength];
    if (fread(fileBytes, 1, fileBytesLength, fp) != fileBytesLength)
    {
        delete[] fileBytes;
        dmLogError("Failed to read file from '%s'", path);
        return 0;
    }

    rive::File* file          = 0;
    rive::BinaryReader reader = rive::BinaryReader(fileBytes, fileBytesLength);
    rive::ImportResult result = rive::File::import(reader, &file);
    if (result != rive::ImportResult::success)
    {
        delete[] fileBytes;
        dmLogError("Failed to import file from '%s'", path);
        return 0;
    }

    delete[] fileBytes;

    return file->artboard();
}

static int Init(lua_State* L)
{
    // Init and load artboard
    rive::Artboard* artboard = LoadArtBoardFromFile(lua_tostring(L, 1));

    if (artboard != 0)
    {
        rive::g_Context->m_Artboard = artboard;
        rive::g_Context->m_Artboard->advance(0.0f);
    }

    return 0;
}

static int SetRenderListener(lua_State* L)
{
    if (rive::g_Context->m_Listener)
    {
        dmScript::DestroyCallback(rive::g_Context->m_Listener);
    }
    rive::g_Context->m_Listener = dmScript::CreateCallback(L, 1);
    return 0;
}

static int DrawFrame(lua_State* L)
{
    // Todo: pass these in?
    float width  = 960.0f;
    float height = 640.0f;
    float dt     = lua_tonumber(L, 1);

    if (rive::g_Context->m_Artboard)
    {
        rive::g_Renderer->save();
        rive::g_Renderer->startFrame();
        rive::g_Renderer->align(rive::Fit::contain,
           rive::Alignment::center,
           rive::AABB(0, 0, width, height),
           rive::g_Context->m_Artboard->bounds());
        rive::g_Context->m_Artboard->advance(dt);
        rive::g_Context->m_Artboard->draw(rive::g_Renderer);
        rive::g_Renderer->restore();
    }

    return 0;
}

static int PathStencil(lua_State* L)
{
    rive::DefoldRenderPath* rp = (rive::DefoldRenderPath*) lua_touserdata(L, 1);
    if (rp)
    {
        rp->stencil();
    }

    return 0;
}

static int PathCover(lua_State* L)
{
    rive::DefoldRenderPath* rp = (rive::DefoldRenderPath*) lua_touserdata(L, 1);
    if (rp)
    {
        // todo: use transform from lua
        rive::Mat2D transform;
        rp->cover(transform);
    }

    return 0;
}

// Functions exposed to Lua
static const luaL_reg Module_methods[] =
{
    {"init",                Init},
    {"draw_frame",          DrawFrame},
    {"set_render_listener", SetRenderListener},
    {"stencil",             PathStencil},
    {"cover",               PathCover},
    {0, 0}
};

static void LuaInit(lua_State* L)
{
    int top = lua_gettop(L);

    // Register lua names
    luaL_register(L, MODULE_NAME, Module_methods);

    lua_pop(L, 1);
    assert(top == lua_gettop(L));
}

dmExtension::Result AppInitializeMyExtension(dmExtension::AppParams* params)
{
    return dmExtension::RESULT_OK;
}

dmExtension::Result InitializeMyExtension(dmExtension::Params* params)
{
    rive::g_Context  = new rive::RiveContext;
    rive::g_Renderer = new rive::DefoldRenderer;

    memset(rive::g_Context,  0, sizeof(*rive::g_Context));

    // Init Lua
    LuaInit(params->m_L);
    return dmExtension::RESULT_OK;
}

dmExtension::Result AppFinalizeMyExtension(dmExtension::AppParams* params)
{
    return dmExtension::RESULT_OK;
}

dmExtension::Result FinalizeMyExtension(dmExtension::Params* params)
{
    delete rive::g_Context;
    rive::g_Context = 0;

    delete rive::g_Renderer;
    rive::g_Renderer = 0;

    return dmExtension::RESULT_OK;
}

dmExtension::Result OnUpdateMyExtension(dmExtension::Params* params)
{
    return dmExtension::RESULT_OK;
}

void OnEventMyExtension(dmExtension::Params* params, const dmExtension::Event* event)
{
    switch(event->m_Event)
    {
        case dmExtension::EVENT_ID_ACTIVATEAPP:
            dmLogInfo("OnEventMyExtension - EVENT_ID_ACTIVATEAPP\n");
            break;
        case dmExtension::EVENT_ID_DEACTIVATEAPP:
            dmLogInfo("OnEventMyExtension - EVENT_ID_DEACTIVATEAPP\n");
            break;
        case dmExtension::EVENT_ID_ICONIFYAPP:
            dmLogInfo("OnEventMyExtension - EVENT_ID_ICONIFYAPP\n");
            break;
        case dmExtension::EVENT_ID_DEICONIFYAPP:
            dmLogInfo("OnEventMyExtension - EVENT_ID_DEICONIFYAPP\n");
            break;
        default:
            dmLogWarning("OnEventMyExtension - Unknown event id\n");
            break;
    }
}

DM_DECLARE_EXTENSION(DefoldRiveTest, LIB_NAME, AppInitializeMyExtension, AppFinalizeMyExtension, InitializeMyExtension, OnUpdateMyExtension, OnEventMyExtension, FinalizeMyExtension)