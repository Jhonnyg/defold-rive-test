local function render_nop() end

local M = {}
M[rive.CMD_NONE]        = render_nop
M[rive.CMD_START_FRAME] = render_nop
return M
