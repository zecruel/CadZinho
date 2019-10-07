count = 0

function test_script_win()
	cadzinho.nk_layout(20, 2)
	cadzinho.nk_label("Click count:")
	if cadzinho.nk_button("Increment") then
		count = count + 1
		-- cadzinho.db_print(count)
	end
	cadzinho.nk_label(count)
end

cadzinho.win_show("test_script_win", "script test", 200,200,200,200)