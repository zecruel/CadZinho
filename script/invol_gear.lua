gear_mod = 2.5 -- Module of gear
press_ang = 20 -- pressure angle in degrees
teeth = 12 -- theeth number
shift = 0

-- reference diameter (primitive circle)
ref_diam = gear_mod * teeth

-- base circle to generate involute curve
base_diam = ref_diam * math.cos( press_ang * math.pi / 180 )
base_r = base_diam / 2

tip_diam = ref_diam + 2 * gear_mod * (1 + shift)
tip_r = tip_diam / 2
tip_press_ang = math.acos(base_diam / tip_diam) * 180 / math.pi
inv_alpha = math.tan(press_ang * math.pi / 180) - press_ang * math.pi / 180
inv_alpha_a = math.tan(tip_press_ang * math.pi / 180) - tip_press_ang * math.pi / 180
top_thick = math.pi / (2 * teeth) + (2 * shift * math.tan(press_ang * math.pi / 180)) / teeth + inv_alpha - inv_alpha_a
top_thick_deg = top_thick * 180 / math.pi
crest_w = tip_diam * top_thick

tooth_thick = 2 * ((90 / teeth) + ( 360 * shift * math.tan(press_ang * math.pi / 180) ) / math.pi / teeth)

-- parameter u to generate involute curve
umax = math.sqrt( tip_r * tip_r / base_r / base_r - 1) --sqrt( R^2/r^2 - 1)

--  calculate angular tooth width along the base diameter.
x1 = base_r;
y1 = 0;
x2 = base_r * ( math.cos( umax ) + umax * math.sin( umax ) );
y2 = base_r * ( math.sin( umax ) - umax * math.cos( umax ) );

-- distance between beginning and end of the involute curve
d = math.sqrt( ( x1 - x2 ) * ( x1 - x2 ) + ( y1 - y2 ) * ( y1 - y2 ) )
cosx = (base_r * base_r + tip_r * tip_r - d * d) / 2 / base_r / tip_r

base_tooth_thick = 2 * top_thick_deg + 2 * math.acos(cosx) * 180 / math.pi;

cadzinho.db_print("Reference diameter = "..ref_diam)
cadzinho.db_print("Base diameter = "..base_diam)
cadzinho.db_print("Tip diameter = "..tip_diam)
cadzinho.db_print("Tooth thickness at base (°) = "..base_tooth_thick)
cadzinho.db_print("Tooth top thickness (°) = "..top_thick_deg)
cadzinho.db_print("Crest width = "..crest_w)
cadzinho.db_print("u max = "..umax)

circle = cadzinho.new_circle(0, 0, ref_diam/2)
cadzinho.ent_append(circle)

circle = cadzinho.new_circle(0, 0, tip_r)
cadzinho.ent_append(circle)

circle = cadzinho.new_circle(0, 0, base_r)
cadzinho.ent_append(circle)

-- involute curve parametric equations (u is pararameter)
--x = base_r * (cos(u) + u * sin(u))
--y = base_r * (sin(u) - u * cos(u))

step = 0.05
u = step
x = base_r * (math.cos(u) + u*math.sin(u))
y = base_r * (math.sin(u) - u*math.cos(u))

pline = cadzinho.new_pline(base_r, 0, 0, x,y,0)

for u = 2*step, umax, step do
	x = base_r * (math.cos(u) + u*math.sin(u))
	y = base_r * (math.sin(u) - u*math.cos(u))
	cadzinho.pline_append(pline, x,y,0)
end

-- last point of involute
x = base_r * (math.cos(umax) + umax * math.sin(umax))
y = base_r * (math.sin(umax) - umax * math.cos(umax))
cadzinho.pline_append(pline, x,y,0)

-- draw tooth top
x = tip_r * (math.cos(2 * top_thick + math.acos(cosx)))
y = tip_r * (math.sin(2 * top_thick + math.acos(cosx)))
cadzinho.pline_append(pline, x,y, top_thick/math.pi)
cadzinho.ent_append(pline)

-- draw tooth reflection line
x = tip_r * (math.cos(base_tooth_thick/2 * math.pi/180))
y = tip_r * (math.sin(base_tooth_thick/2 * math.pi/180))
pline = cadzinho.new_pline(0, 0, 0, x,y,0)
cadzinho.ent_append(pline)