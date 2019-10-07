local cx, cy, r, n=...

x = cx + r * math.cos(2*math.pi/n)
y = cy + r * math.sin(2*math.pi/n)

pline = cadzinho.new_pline(cx + r, cy,0, x,y,0)

for i = 2, n - 1 do
	x = cx + r * math.cos(2*math.pi*i/n)
	y = cy + r * math.sin(2*math.pi*i/n)
	cadzinho.pline_append(pline, x,y,0)
end

cadzinho.pline_close(pline, true)
cadzinho.ent_append(pline)

circle = cadzinho.new_circle(cx,cy,r)
cadzinho.ent_append(circle)

a = r * math.cos(math.pi/n)

circle = cadzinho.new_circle(cx,cy,a)
cadzinho.ent_append(circle)
