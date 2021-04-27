
local function rive_render_nop()
    -- dummy fn, remove at some point
end

local function rive_render_start_frame()
    render.disable_state(render.STATE_DEPTH_TEST)
    render.disable_state(render.STATE_CULL_FACE)
    render.enable_state(render.STATE_STENCIL_TEST)
    render.set_viewport(0, 0, render.get_window_width(), render.get_window_height())
    render.clear({[render.BUFFER_COLOR_BIT] = vmath.vector4(0, 0, 0, 0), [render.BUFFER_STENCIL_BIT] = 1})
end

local function rive_render_apply_clipping()
    render.set_stencil_mask(0xff)
    render.set_stencil_func(render.COMPARE_FUNC_ALWAYS, 0x0, 0xff)
    render.set_stencil_op(render.STENCIL_OP_ZERO, render.STENCIL_OP_ZERO, render.STENCIL_OP_ZERO)
    render.set_color_mask(false, false, false, false)

    -- todo:
    -- gl.useProgram(clearScreenProgramInfo.program);
    -- twgl.setBuffersAndAttributes(gl, clearScreenProgramInfo, screenBlitBuffer);
    -- twgl.drawBufferInfo(gl, screenBlitBuffer);
end

local function rive_render_apply_clip_path(action)
    --[[
    -- todo:
    -- gl.useProgram(programInfo.program);

    if action.is_clipping then
        -- When clipping we want to write only to the last/lower 7 bits as our high 8th bit is used to mark clipping inclusion.
        render.set_stencil_mask(0x7f)
        -- Pass only if that 8th bit is set. This allows us to write our new winding into the lower 7 bits.
        render.set_stencil_func(render.COMPARE_FUNC_EQUAL, 0x80, 0x80)
    else
        render.set_stencil_mask(0xff)
        render.set_stencil_func(render.COMPARE_FUNC_ALWAYS, 0x0, 0xff)
    end

    render.set_color_mask(false, false, false, false)

    -- todo
    -- gl.stencilOpSeparate(gl.FRONT, gl.KEEP, gl.KEEP, gl.INCR_WRAP);
    -- gl.stencilOpSeparate(gl.BACK, gl.KEEP, gl.KEEP, gl.DECR_WRAP);

    if not action.is_even_odd then
        -- gl.frontFace(gl.CCW);
    end

    -- todo:
    -- path.stencil(this, transform, 0, isEvenOdd);
    -- rive.stencil(action.render_path) <- infinte loop?

    render.set_color_mask(false, false, false, false)

    -- Fail when not equal to 0 and replace with 0x80 (mark high bit as included in clip).
    -- Require stencil mask (write mask) of 0xFF and stencil func mask of 0x7F such that the comparison looks for 0 but write 0x80.
    render.set_stencil_mask(0xff)
    render.set_stencil_func(render.COMPARE_FUNC_NOTEQUAL, 0x80, 0x7f)
    render.set_stencil_op(render.STENCIL_OP_ZERO, render.STENCIL_OP_ZERO, render.STENCIL_OP_REPLACE)

    if action.is_clipping then
        -- gl.useProgram(clearScreenProgramInfo.program);
        -- twgl.setBuffersAndAttributes(
        --     gl,
        --     clearScreenProgramInfo,
        --     screenBlitBuffer
        -- );
        -- twgl.drawBufferInfo(gl, screenBlitBuffer);
    else
        -- todo: path.cover(this, transform, programInfo);
        rive.cover(action.render_path)
    end
    --]]
end

local rive_render_fn_tbl                 = {}
rive_render_fn_tbl[rive.CMD_NONE]        = rive_render_nop
rive_render_fn_tbl[rive.CMD_START_FRAME] = rive_render_start_frame

local M = {}

M.execute = function(rive_commands)
    if rive_commands == nil then
        return
    end

    for i = 1, #rive_commands do
        local cmd_data = rive_commands[i]
        rive_render_fn_tbl[cmd_data.cmd](cmd_data)
    end
end

return M
