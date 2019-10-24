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


cadzinho.set_layer("0")
cadzinho.set_ltype("Continuous")
cadzinho.set_style("Standard")

elements = { reg_poly(10, 20, 5, 15, 0),
		reg_poly(10, 20, 6, 20, math.pi/2)
		}
cadzinho.new_block(elements, "teste")