
local Action = {
	NONE        = 0,
	RENDER_PATH = 1,
	START_FRAME = 2,
}

local function rive_render_start_frame()
	render.disable_state(render.STATE_DEPTH_TEST)
	render.disable_state(render.STATE_CULL_FACE)
	render.enable_state(render.STATE_STENCIL_TEST)
	render.set_viewport(0, 0, render.get_window_width(), render.get_window_height())
	render.clear({[render.BUFFER_COLOR_BIT] = vmath.vector4(0, 0, 0, 0), [render.BUFFER_STENCIL_BIT] = 1})
end

local function rive_render_listener(self, data)
	if data.action == Action.RENDER_PATH then
		-- print("rive_listener: RENDER_PATH")
	elseif data.action == Action.START_FRAME then
		rive_render_start_frame()
	end
end


local M = {}

M.init = function()
	rive.set_render_listener(rive_render_listener)
end

M.draw = function(dt)
	rive.draw_frame(dt)
end

return M
