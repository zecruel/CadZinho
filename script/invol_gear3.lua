function gear(gear_mod, press_ang, teeth, shift)
	local elements = {}
	
	cadzinho.set_color("by block")
	cadzinho.set_lw("by block")

	-- reference diameter (primitive circle)
	local ref_diam = gear_mod * teeth

	-- base circle to generate involute curve
	local base_diam = ref_diam * math.cos( press_ang * math.pi / 180 )
	local base_r = base_diam / 2

	local tip_diam = ref_diam + 2 * gear_mod * (1 + shift)
	local tip_r = tip_diam / 2
	local tip_press_ang = math.acos(base_diam / tip_diam) * 180 / math.pi
	local inv_alpha = math.tan(press_ang * math.pi / 180) - press_ang * math.pi / 180
	local inv_alpha_a = math.tan(tip_press_ang * math.pi / 180) - tip_press_ang * math.pi / 180
	local top_thick = math.pi / (2 * teeth) + (2 * shift * math.tan(press_ang * math.pi / 180)) / teeth + inv_alpha - inv_alpha_a
	local top_thick_deg = top_thick * 180 / math.pi
	local crest_w = tip_diam * top_thick

	local valey_r = ref_diam/2 - (7/6 * gear_mod)

	local tooth_thick = 2 * ((90 / teeth) + ( 360 * shift * math.tan(press_ang * math.pi / 180) ) / math.pi / teeth)

	-- parameter u to generate involute curve
	local umax = math.sqrt( tip_r * tip_r / base_r / base_r - 1) --sqrt( R^2/r^2 - 1)
	local umin = 0

	if valey_r > base_r then
		umin = math.sqrt( valey_r * valey_r / base_r / base_r - 1) --sqrt( R^2/r^2 - 1)
	end

	--  calculate angular tooth width along the base diameter.
	local x1 = base_r;
	local y1 = 0;
	local x2 = base_r * ( math.cos( umax ) + umax * math.sin( umax ) );
	local y2 = base_r * ( math.sin( umax ) - umax * math.cos( umax ) );

	-- distance between beginning and end of the involute curve
	local d = math.sqrt( ( x1 - x2 ) * ( x1 - x2 ) + ( y1 - y2 ) * ( y1 - y2 ) )
	local cosx = (base_r * base_r + tip_r * tip_r - d * d) / 2 / base_r / tip_r

	local base_tooth_thick = 2 * top_thick_deg + 2 * math.acos(cosx) * 180 / math.pi;
--[[
	cadzinho.db_print("Reference diameter = "..ref_diam)
	cadzinho.db_print("Base diameter = "..base_diam)
	cadzinho.db_print("Tip diameter = "..tip_diam)
	cadzinho.db_print("Tooth thickness at base (°) = "..base_tooth_thick)
	cadzinho.db_print("Tooth top thickness (°) = "..top_thick_deg)
	cadzinho.db_print("Crest width = "..crest_w)
	cadzinho.db_print("u max = "..umax)
]]
	cadzinho.set_ltype("Dot")
	
	local circle = cadzinho.new_circle(0, 0, ref_diam/2)
	table.insert(elements, circle)

	--circle = cadzinho.new_circle(0, 0, tip_r)
	--table.insert(elements, circle)

	--circle = cadzinho.new_circle(0, 0, base_r)
	--table.insert(elements, circle)

	--circle = cadzinho.new_circle(0, 0, valey_r)
	--table.insert(elements, circle)

	cadzinho.set_ltype("Continuous")

	-- draw tooth reflection line
	local x = tip_r * (math.cos(base_tooth_thick/2 * math.pi/180))
	local y = tip_r * (math.sin(base_tooth_thick/2 * math.pi/180))
	local pline = cadzinho.new_pline(0, 0, 0, x,y,0)
	table.insert(elements, pline)


	--[[ ************************************************************
	************   generate tooth involute profile *********************
	*************************************************************** ]]

	-- involute curve parametric equations (u is pararameter)
	--x = base_r * (cos(u) + u * sin(u))
	--y = base_r * (sin(u) - u * cos(u))

	local step = 0.05
	local u = umin
	local tooth_curve = {}
	local i
	for i = 1, math.floor((umax-umin)/step) do
		tooth_curve[i] = {}
		tooth_curve[i].x = base_r * (math.cos(u) + u*math.sin(u))
		tooth_curve[i].y = base_r * (math.sin(u) - u*math.cos(u))
		tooth_curve[i].b = 0
		u = u + step
	end
	-- last point of involute
	i = #tooth_curve + 1
	tooth_curve[i] = {}
	tooth_curve[i].x = base_r * (math.cos(umax) + umax * math.sin(umax))
	tooth_curve[i].y = base_r * (math.sin(umax) - umax * math.cos(umax))
	tooth_curve[i].b = 0

	-- **********************************************************

	--[[ ************************************************************
	*****  reflect profile to generate complete tooth drawing ***********
	************************************************************** ]]

	-- tooth reflection parameters
	local rx = tip_r * (math.cos(base_tooth_thick/2 * math.pi/180))
	local ry = tip_r * (math.sin(base_tooth_thick/2 * math.pi/180))
	local rm = (rx^2 + ry^2)^0.5

	local num_pts = #tooth_curve
	local dist

	for i = 1, num_pts do
		-- get original list of points in reverse order
		-- calcule distance between point and reflection line
		dist = (-ry*tooth_curve[num_pts - i + 1].x + rx*tooth_curve[num_pts - i + 1].y + ry*rx - rx*ry)/rm;
		
		-- new reflected points
		tooth_curve[num_pts + i] = {}
		tooth_curve[num_pts + i].x = tooth_curve[num_pts - i + 1].x + 2 * dist * (ry/rm);
		tooth_curve[num_pts + i].y = tooth_curve[num_pts - i + 1].y + 2 * dist * (-rx/rm);
		tooth_curve[num_pts + i].b = 0;
	end

	-- **********************************************************



	-- draw all tooths, rotating along center (0,0)
	local j
	local cosine
	local sine
	local bulge
	for j = 1, teeth do
		cosine = math.cos((j-1) * 2 * math.pi / teeth)
		sine = math.sin((j-1) * 2 * math.pi / teeth)
		
		x1 = tooth_curve[1].x * cosine - tooth_curve[1].y * sine;
		y1 = tooth_curve[1].x * sine + tooth_curve[1].y * cosine;
		x2 = tooth_curve[2].x * cosine - tooth_curve[2].y * sine;
		y2 = tooth_curve[2].x * sine + tooth_curve[2].y * cosine;
		
		pline = cadzinho.new_pline(x1, y1, tooth_curve[1].b, x2, y2, tooth_curve[2].b)
		for i = 3, #tooth_curve do
			x2 = tooth_curve[i].x * cosine - tooth_curve[i].y * sine;
			y2 = tooth_curve[i].x * sine + tooth_curve[i].y * cosine;
			cadzinho.pline_append(pline, x2, y2, tooth_curve[i].b)
		end
		table.insert(elements, pline)
		
		-- draw tooth jointer
		cosine = math.cos((j) * 2 * math.pi / teeth)
		sine = math.sin((j) * 2 * math.pi / teeth)
		
		x1 = tooth_curve[1].x * cosine - tooth_curve[1].y * sine;
		y1 = tooth_curve[1].x * sine + tooth_curve[1].y * cosine;
		
		bulge = 0
		if valey_r < base_r then
			bulge = -1.3
		end
		pline = cadzinho.new_pline(x2, y2, 0, x1, y1, bulge)
		table.insert(elements, pline)
	end
	return elements
end

cadzinho.set_layer("0")
cadzinho.set_ltype("Continuous")
cadzinho.set_style("Standard")

new_gear = gear(2.5, 20, 56, 0)
text = cadzinho.new_text(0, 2, "#id", 2.5, "center", "middle")
table.insert (new_gear, text)
text = cadzinho.new_text(0, -2, "#teeth$56", 2.5, "center", "middle")
table.insert (new_gear, text)
text = cadzinho.new_text(0, 5, "#*module$2.5", 2.5, "center", "middle")
table.insert (new_gear, text)
text = cadzinho.new_text(0, -5, "#*press_ang$20", 2.5, "center", "middle")
table.insert (new_gear, text)
cadzinho.new_block(new_gear, "gear-56", "Gear 56 teeth", true)

new_gear = gear(2.5, 20, 12, 0)
text = cadzinho.new_text(0, 2, "%id", 2.5, "center", "middle")
table.insert (new_gear, text)
text = cadzinho.new_text(0, -2, "%teeth*12", 2.5, "center", "middle")
table.insert (new_gear, text)
text = cadzinho.new_text(0, 5, "%#module*2.5", 2.5, "center", "middle")
table.insert (new_gear, text)
text = cadzinho.new_text(0, -5, "%#press_ang*20", 2.5, "center", "middle")
table.insert (new_gear, text)
cadzinho.new_block(new_gear, "gear-12", "Gear 12 teeth", true, "%", "#", "*", "??")

new_gear = gear(2.5, 20, 30, 0)
text = cadzinho.new_text(0, 2, "%id", 2.5, "center", "middle")
table.insert (new_gear, text)
text = cadzinho.new_text(0, -2, "%teeth*30", 2.5, "center", "middle")
table.insert (new_gear, text)
text = cadzinho.new_text(0, 5, "%#module*2.5", 2.5, "center", "middle")
table.insert (new_gear, text)
text = cadzinho.new_text(0, -5, "%#press_ang*20", 2.5, "center", "middle")
table.insert (new_gear, text)
cadzinho.new_block(new_gear, "gear-30", "Gear 30 teeth", true, "%", "#", "*", "??", 0, 0, 0)

cadzinho.new_block(gear(2.5, 20, 44, 0), "gear-44", "Gear 44 teeth")

cadzinho.set_color("by layer")
cadzinho.set_lw("by layer")