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
	cadzinho.nk_layout(20, 1)
	cadzinho.nk_label("Regular polygon")
	
	cadzinho.nk_layout(20, 2)
	cadzinho.nk_label("Sides:")
	cadzinho.nk_edit(sides)
	
	
	cadzinho.nk_layout(20, 1)
	if count == 0 then
		cadzinho.nk_label('Enter center')
		if event.type == 'enter' then
			count = count + 1
			cx = event.x
			cy = event.y
			
			
		elseif event.type == 'cancel' then
			cadzinho.stop_dynamic()
		end
	else
		cadzinho.nk_label('Enter radius')
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
			
			pline:write()
		
			count = 0
		elseif event.type == 'cancel' then
			count = 0
			
		end
	end
end

cadzinho.start_dynamic("test_script_dyn")