function test_script_win()
	if not cadzinho.nk_begin("script test", 100,100,100,100) then
		cadzinho.win_close()
	end
	cadzinho.nk_end()
end

cadzinho.win_show("test_script_win")