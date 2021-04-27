local stc = require("rive/scripts/rendering_stc")
local tss = require("rive/scripts/rendering_tessellation")

local M = {}
local fn_tbl = nil

M.init = function()
    if rive.render_mode() == rive.MODE_TESSELLATION then
        fn_tbl = tss
    else
        fn_tbl = stc
    end

    pprint(fn_tbl)
end

M.execute = function(rive_commands)
    if rive_commands == nil then
        return
    end

    for i = 1, #rive_commands do
        local cmd_data = rive_commands[i]
        fn_tbl[cmd_data.cmd](cmd_data)
    end
end

return M
