count = 0
n=5
r = 10

function test_script_dyn(event)
	--cadzinho.db_print(event.type)
	if event.type == 'motion' then
		--cadzinho.db_print('xy='..event.x..','..event.y)
		cx = event.x
		cy = event.y
		
		x = cx + r * math.cos(2*math.pi/n)
		y = cy + r * math.sin(2*math.pi/n)

		pline = cadzinho.new_pline(cx + r, cy,0, x,y,0)

		for i = 2, n - 1 do
			x = cx + r * math.cos(2*math.pi*i/n)
			y = cy + r * math.sin(2*math.pi*i/n)
			cadzinho.pline_append(pline, x,y,0)
		end

		cadzinho.pline_close(pline, true)
		cadzinho.ent_draw(pline)
		
	elseif event.type == 'enter' then
		count = count + 1
		cadzinho.db_print('count='..count)
		cadzinho.db_print('xy='..event.x..','..event.y)
	elseif event.type == 'cancel' then
		cadzinho.stop_dynamic()
	end
end

cadzinho.db_print('testing dynamic')
cadzinho.start_dynamic("test_script_dyn")