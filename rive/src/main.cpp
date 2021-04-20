#define LIB_NAME "DefoldRiveTest"
#define MODULE_NAME "rive"

#include <dmsdk/sdk.h>

#define TESTING
#include <artboard.hpp>
#include <shapes/rectangle.hpp>
#include <shapes/shape.hpp>

#include "no_op_renderer.hpp"
#include "renderer.hpp"

#include <dmsdk/sdk.h>
#include "defold_render_path.h"
#include "defold_renderer.h"

namespace rive
{
    RenderPaint* makeRenderPaint() { return new NoOpRenderPaint();  }
    RenderPath* makeRenderPath()   { return new DefoldRenderPath(); }

    static DefoldRenderer* renderer  = new DefoldRenderer();
    static Artboard* active_artboard = 0;
}

static int Test(lua_State* L)
{
    rive::active_artboard = new rive::Artboard();
    rive::Shape* shape = new rive::Shape();
    rive::Rectangle* rectangle = new rive::Rectangle();

    rectangle->x(0.0f);
    rectangle->y(0.0f);
    rectangle->width(100.0f);
    rectangle->height(200.0f);
    rectangle->cornerRadius(20.0f);

    rive::active_artboard->addObject(rive::active_artboard);
    rive::active_artboard->addObject(shape);
    rive::active_artboard->addObject(rectangle);
    rectangle->parentId(1);

    rive::active_artboard->initialize();
    rive::active_artboard->advance(0.0f);
    return 0;
}

static int DrawFrame(lua_State* L)
{
    // Todo: pass these in?
    float width  = 960.0f;
    float height = 640.0f;

    if (rive::active_artboard)
    {
        dmLogInfo("DrawFrame");
        
        rive::renderer->save();
        rive::renderer->startFrame();
        rive::renderer->align(rive::Fit::contain,
                       rive::Alignment::center,
                       rive::AABB(0, 0, width, height),
                       rive::active_artboard->bounds());
        rive::active_artboard->draw(rive::renderer);
        rive::renderer->restore();
    }

    return 0;
}

// Functions exposed to Lua
static const luaL_reg Module_methods[] =
{
    {"test",       Test},
    {"draw_frame", DrawFrame},
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