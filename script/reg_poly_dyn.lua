count = 0
sides = {value = '4'}

function reg_poly(cx, cy, n, r, ang)
	local x0 = cx + r * math.cos(ang)
	local y0 = cy + r * math.sin(ang)
	local x = cx + r * math.cos(2*math.pi/n + ang)
	local y = cy + r * math.sin(2*math.pi/n + ang)

	local pline = cadzinho.new_pline(x0, y0, 0, x, y, 0)

	for i = 2, n - 1 do
		x = cx + r * math.cos(2*math.pi*i/n + ang)
		y = cy + r * math.sin(2*math.pi*i/n + ang)
		cadzinho.pline_append(pline, x,y,0)
	end

	cadzinho.pline_close(pline, true)
	
	return pline
end

function test_script_dyn(event)
	if count == 0 then
		if event.type == 'enter' then
			count = count + 1
			cx = event.x
			cy = event.y
			
			cadzinho.db_print('enter radius')
		elseif event.type == 'cancel' then
			cadzinho.stop_dynamic()
		end
	else
		if event.type == 'motion' then
			r = ((event.x - cx)^2 + (event.y - cy)^2) ^ 0.5
			ang = math.atan((event.y - cy), (event.x - cx))
			n = tonumber(sides.value)

			pline = reg_poly(cx, cy, n, r, ang)
			cadzinho.ent_draw(pline)
			
			circle = cadzinho.new_circle(cx,cy,r)
			cadzinho.ent_draw(circle)
			
		elseif event.type == 'enter' then
			r = ((event.x - cx)^2 + (event.y - cy)^2) ^ 0.5
			ang = math.atan((event.y - cy), (event.x - cx))
			n = tonumber(sides.value)
			
			pline = reg_poly(cx, cy, n, r, ang)
			
			cadzinho.ent_append(pline)
		
			count = 0
			cadzinho.db_print('enter center')
		elseif event.type == 'cancel' then
			count = 0
			cadzinho.db_print('enter center')
		end
	end
end

function polyg_win()
	cadzinho.nk_layout(20, 1)
	
	cadzinho.nk_label("Sides:")
	cadzinho.nk_edit(sides)
	
end

cadzinho.win_show("polyg_win", "script test", 200,200,200,200)

cadzinho.start_dynamic("test_script_dyn")