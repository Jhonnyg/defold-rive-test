#define LIB_NAME "DefoldRiveTest"
#define MODULE_NAME "rive"

// Rive
#include <artboard.hpp>
#include <shapes/rectangle.hpp>
#include <shapes/shape.hpp>
#include <animation/linear_animation_instance.hpp>
#include <file.hpp>
#include <renderer.hpp>
#include <math/transform_components.hpp>

// Libtess
#include <tesselator.h>

// Defold
#include <dmsdk/sdk.h>
#include "defold_rive_private.h"

// stencil to cover
#include "stc/defold_stc_render_path.h"
#include "stc/defold_stc_renderer.h"

// tesselation
#include "tessellation/defold_tss_render_path.h"
#include "tessellation/defold_tss_render_paint.h"
#include "tessellation/defold_tss_renderer.h"

namespace rive
{
    struct PathIdTuple
    {
        uintptr_t m_PathAddr;
        int32_t   m_Id;
    };

    static const RiveRenderMode g_RenderMode    = MODE_TESSELLATION;
    static DefoldRenderer*      g_Renderer      = 0;
    static RiveContext*         g_Context       = 0;
    static int32_t              g_PathToIdSeq   = 0;
    static dmArray<PathIdTuple> g_PathToIdTable;

    RenderPaint* makeRenderPaint()
    {
        if (g_RenderMode == MODE_TESSELLATION)
        {
            return new DefoldTessellationRenderPaint();
        }
        else
        {
            return new DefoldNoOpRenderPaint();
        }
    }

    RenderPath* makeRenderPath()
    {
        if (g_RenderMode == MODE_TESSELLATION)
        {
            return new DefoldTessellationRenderPath;
        }
        else
        {
            return new DefoldStCRenderPath;
        }
    }

    static inline void ClearCommands()
    {
        rive::g_Context->m_Commands.SetSize(0);
    }

    void InvokeRiveListener(const RiveListenerData data)
    {
        rive::RiveContext* ctx = rive::g_Context;
        if (!dmScript::IsCallbackValid(ctx->m_Listener))
        {
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

    void AddCmd(const RiveCmd cmd)
    {
        rive::RiveContext* ctx = rive::g_Context;

        if (ctx->m_Commands.Size() == ctx->m_Commands.Capacity())
        {
            ctx->m_Commands.OffsetCapacity(1);
        }

        ctx->m_Commands.Push(cmd);
    }

    static int32_t GetOrPutPathId(RenderPath* path)
    {
        uintptr_t addr = (uintptr_t) path;

        for (int i = 0; i < g_PathToIdTable.Size(); ++i)
        {
            const PathIdTuple entry = g_PathToIdTable[i];
            if (entry.m_PathAddr == addr)
            {
                return entry.m_Id;
            }
        }

        if (g_PathToIdTable.Size() == g_PathToIdTable.Capacity())
        {
            g_PathToIdTable.OffsetCapacity(1);
        }

        g_PathToIdTable.Push({
            .m_PathAddr = addr,
            .m_Id       = g_PathToIdSeq,
        });

        return g_PathToIdSeq++;
    }

    static uintptr_t GetPathAddrFromId(int32_t id)
    {
        for (int i = 0; i < g_PathToIdTable.Size(); ++i)
        {
            if (g_PathToIdTable[i].m_Id == id)
            {
                return g_PathToIdTable[i].m_PathAddr;
            }
        }

        return 0;
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

    // dmLogInfo("Num artboards: %d", file->artboardCount());

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
        rive::g_Context->m_ArtboardAnimation = 0;

        if (rive::g_Context->m_Artboard->animationCount() > 0)
        {
            rive::LinearAnimation* a             = rive::g_Context->m_Artboard->firstAnimation();
            rive::g_Context->m_ArtboardAnimation = new rive::LinearAnimationInstance(a);
        }
    }

    return 0;
}

static int SetListener(lua_State* L)
{
    if (rive::g_Context->m_Listener)
    {
        dmScript::DestroyCallback(rive::g_Context->m_Listener);
    }
    rive::g_Context->m_Listener = dmScript::CreateCallback(L, 1);
    return 0;
}

static void PushRenderPath(lua_State* L, rive::RenderPath* p)
{
    int32_t id = rive::GetOrPutPathId(p);
    lua_pushstring(L, "id");
    lua_pushinteger(L, id);
    lua_settable(L, -3);
}

static void PushRenderPaint(lua_State* L, rive::RenderPaint* rp)
{
    float rgba[4];
    rive::DefoldTessellationRenderPaint* drp = (rive::DefoldTessellationRenderPaint*) rp;
    const rive::DefoldTessellationRenderPaintData data = drp->getData();

    lua_pushstring(L, "paint");
    lua_newtable(L);

    lua_pushstring(L, "type");
    lua_pushinteger(L, data.m_FillType);
    lua_settable(L, -3);

    if (data.m_FillType == rive::FILL_TYPE_SOLID)
    {
        lua_pushstring(L, "color");
        lua_newtable(L);

            lua_pushnumber(L, 1);
            lua_pushnumber(L, data.m_Colors[0]);
            lua_settable(L, -3);

            lua_pushnumber(L, 2);
            lua_pushnumber(L, data.m_Colors[1]);
            lua_settable(L, -3);

            lua_pushnumber(L, 3);
            lua_pushnumber(L, data.m_Colors[2]);
            lua_settable(L, -3);

            lua_pushnumber(L, 4);
            lua_pushnumber(L, data.m_Colors[3]);
            lua_settable(L, -3);

        lua_settable(L, -3);
    }
    else
    {
        lua_pushstring(L, "limits");
        lua_newtable(L);

            lua_pushnumber(L, 1);
            lua_pushnumber(L, data.m_GradientLimits[0]); // sx
            lua_settable(L, -3);

            lua_pushnumber(L, 2);
            lua_pushnumber(L, data.m_GradientLimits[1]); // ex
            lua_settable(L, -3);

            lua_pushnumber(L, 3);
            lua_pushnumber(L, data.m_GradientLimits[2]); // sy
            lua_settable(L, -3);

            lua_pushnumber(L, 4);
            lua_pushnumber(L, data.m_GradientLimits[3]); // ey
            lua_settable(L, -3);

        lua_settable(L, -3);

        lua_pushstring(L, "stops");
        lua_newtable(L);
        for (int i = 0; i < data.m_StopCount; ++i)
        {
            lua_pushnumber(L, i + 1);
            lua_pushnumber(L, data.m_Stops[i]);
            lua_settable(L, -3);
        }
        lua_settable(L, -3);

        lua_pushstring(L, "colors");
        lua_newtable(L);
        for (int i = 0; i < data.m_StopCount; ++i)
        {
            const float* color = &data.m_Colors[i * 4];

            lua_pushinteger(L, i + 1);
            lua_newtable(L);

                lua_pushnumber(L, 1);
                lua_pushnumber(L, color[0]);
                lua_settable(L, -3);

                lua_pushnumber(L, 2);
                lua_pushnumber(L, color[1]);
                lua_settable(L, -3);

                lua_pushnumber(L, 3);
                lua_pushnumber(L, color[2]);
                lua_settable(L, -3);

                lua_pushnumber(L, 4);
                lua_pushnumber(L, color[3]);
                lua_settable(L, -3);

            lua_settable(L, -3);
        }
        lua_settable(L, -3);
    }
    lua_settable(L, -3);
}

static int PushRiveCmdsToLua(lua_State* L)
{
    lua_newtable(L);

    for (int i = 0; i < rive::g_Context->m_Commands.Size(); ++i)
    {
        const rive::RiveCmd cmd = rive::g_Context->m_Commands[i];

        // push index
        lua_pushnumber(L, i + 1);

        // create sub-table and set cmd data
        lua_newtable(L);
        lua_pushstring(L, "cmd");
        lua_pushinteger(L, cmd.m_Cmd);
        lua_settable(L, -3);

        switch (cmd.m_Cmd)
        {
            case rive::CMD_UPDATE_TESSELATION:
            {
                assert(rive::g_RenderMode == rive::MODE_TESSELLATION);
                PushRenderPath(L, cmd.m_RenderPath);
            } break;
            case rive::CMD_DRAW_PATH:
            {
                assert(rive::g_RenderMode == rive::MODE_TESSELLATION);
                PushRenderPath(L, cmd.m_RenderPath);
                PushRenderPaint(L, cmd.m_RenderPaint);
            } break;
            case rive::CMD_UPDATE_DRAW_INDEX:
            {
                assert(rive::g_RenderMode == rive::MODE_TESSELLATION);
                PushRenderPath(L, cmd.m_RenderPath);
            }
            default:break;
        }

        // push new table to index
        lua_settable(L, -3);
    }

    return 1;
}

static int GetPath(lua_State* L)
{
    int32_t id         = lua_tointeger(L, 1);
    uintptr_t pathAddr = rive::GetPathAddrFromId(id);

    if (pathAddr == 0)
    {
        dmLogError("No path found with id %d", id);
    }

    rive::DefoldTessellationRenderPath* tesselationPath = (rive::DefoldTessellationRenderPath*) pathAddr;

    dmScript::LuaHBuffer luabuf = {
        tesselationPath->getContourBuffer(),
        dmScript::OWNER_C,
    };

    rive::TransformComponents decomposeResult;
    rive::Mat2D::decompose(decomposeResult, tesselationPath->getTransform());

    int parentId = -1;
    rive::RenderPath* parent = tesselationPath->getParent();
    if (parent)
    {
        parentId = rive::GetOrPutPathId(parent);
    }

    lua_newtable(L);

    lua_pushstring(L, "buffer");
    dmScript::PushBuffer(L, luabuf);
    lua_settable(L, -3);

    lua_pushstring(L, "parent");
    lua_pushinteger(L, parentId);
    lua_settable(L, -3);

    lua_pushstring(L, "position");
    lua_newtable(L);

        lua_pushnumber(L, 1);
        lua_pushnumber(L, decomposeResult.x());
        lua_settable(L, -3);

        lua_pushnumber(L, 2);
        lua_pushnumber(L, decomposeResult.y());
        lua_settable(L, -3);
    lua_settable(L, -3);

    lua_pushstring(L, "scale");
    lua_newtable(L);

        lua_pushnumber(L, 1);
        lua_pushnumber(L, decomposeResult.scaleX());
        lua_settable(L, -3);

        lua_pushnumber(L, 2);
        lua_pushnumber(L, decomposeResult.scaleY());
        lua_settable(L, -3);
    lua_settable(L, -3);

    lua_pushstring(L, "rotation");
    lua_pushnumber(L, decomposeResult.rotation());
    lua_settable(L, -3);

    lua_pushstring(L, "draw_index");
    lua_pushinteger(L, tesselationPath->getDrawIndex());
    lua_settable(L, -3);

    return 1;
}

static int DrawFrame(lua_State* L)
{
    float dt     = lua_tonumber(L, 1);
    float width  = lua_tonumber(L, 2);
    float height = lua_tonumber(L, 3);

    if (rive::g_Context->m_Artboard)
    {
        rive::ClearCommands();
        rive::g_Renderer->save();
        rive::g_Renderer->startFrame();
        rive::g_Renderer->align(rive::Fit::contain,
           rive::Alignment::center,
           rive::AABB(0, 0, width, height),
           rive::g_Context->m_Artboard->bounds());

        if (rive::g_Context->m_ArtboardAnimation)
        {
            rive::g_Context->m_ArtboardAnimation->advance(dt);
            rive::g_Context->m_ArtboardAnimation->apply(rive::g_Context->m_Artboard, 1);
        }
        rive::g_Context->m_Artboard->advance(dt);
        rive::g_Context->m_Artboard->draw(rive::g_Renderer);
        rive::g_Renderer->restore();

        return PushRiveCmdsToLua(L);
    }

    return 0;
}

static int GetRenderMode(lua_State* L)
{
    lua_pushinteger(L, rive::g_RenderMode);
    return 1;
}

// Functions exposed to Lua
static const luaL_reg Module_methods[] =
{
    {"init",                Init},
    {"draw_frame",          DrawFrame},
    {"set_render_listener", SetListener},
    {"render_mode",         GetRenderMode},
    {"get_path",            GetPath},
    { 0, 0}
};

static void LuaInit(lua_State* L)
{
    int top = lua_gettop(L);

    // Register lua names
    luaL_register(L, MODULE_NAME, Module_methods);

    #define REGISTER_RIVE_ENUM(name) \
        lua_pushnumber(L, (lua_Number) rive::name); \
        lua_setfield(L, -2, #name);

    // Register commands
    REGISTER_RIVE_ENUM(CMD_NONE);
    REGISTER_RIVE_ENUM(CMD_START_FRAME);
    REGISTER_RIVE_ENUM(CMD_UPDATE_TESSELATION);
    REGISTER_RIVE_ENUM(CMD_DRAW_PATH);
    REGISTER_RIVE_ENUM(CMD_UPDATE_DRAW_INDEX);

    // Register render modes
    REGISTER_RIVE_ENUM(MODE_TESSELLATION);
    REGISTER_RIVE_ENUM(MODE_STENCIL_TO_COVER);

    // Register fill types
    REGISTER_RIVE_ENUM(FILL_TYPE_NONE);
    REGISTER_RIVE_ENUM(FILL_TYPE_SOLID);
    REGISTER_RIVE_ENUM(FILL_TYPE_LINEAR);
    REGISTER_RIVE_ENUM(FILL_TYPE_RADIAL);

    #undef REGISTER_RIVE_ENUM

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

    if (rive::g_RenderMode == rive::MODE_TESSELLATION)
    {
        rive::g_Renderer = (rive::DefoldRenderer*) new rive::DefoldTessellationRenderer;
    }
    else
    {
        rive::g_Renderer = (rive::DefoldRenderer*) new rive::DefoldStCRenderer;
    }

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
