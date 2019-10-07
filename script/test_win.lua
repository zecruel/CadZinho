count = 0
local edit = {value = 'Edit text'}

function test_script_win()
	cadzinho.nk_layout(20, 2)
	cadzinho.nk_label("Click count:")
	if cadzinho.nk_button("Increment") then
		count = count + 1
		cadzinho.db_print(tonumber(edit.value))
		--assert(loadfile(".\\lua\\reg_poly.lua"))(10,20,30,6)
	end
	cadzinho.nk_label(count)
	cadzinho.nk_edit(edit)
end

cadzinho.win_show("test_script_win", "script test", 200,200,200,200)