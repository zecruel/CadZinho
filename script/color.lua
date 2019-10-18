cadzinho.set_layer("0")
cadzinho.set_ltype("Continuous")


for i = 1, 9 do
	cadzinho.set_color(i)
	bounds = {
		{x = i*15, y = 90},
		{x = i*15, y = 90+10},
		{x = i*15+10, y = 90+10},
		{x = i*15+10, y = 90}
	}

	hatch = cadzinho.new_hatch(bounds, "solid")
	cadzinho.ent_append(hatch)
end


for j = 0, 8, 2 do
	for i = 10, 240, 10 do
		cadzinho.set_color(i+j)
		bounds = {
			{x = i, y = -5*j},
			{x = i, y = -5*j+10},
			{x = i+10, y = -5*j+10},
			{x = i+10, y = -5*j}
		}

		hatch = cadzinho.new_hatch(bounds, "solid")
		cadzinho.ent_append(hatch)
	end
end

for j = 1, 9, 2 do
	for i = 10, 240, 10 do
		cadzinho.set_color(i+j)
		bounds = {
			{x = i, y = 5*j+5},
			{x = i, y = 5*j+15},
			{x = i+10, y = 5*j+15},
			{x = i+10, y = 5*j+5}
		}

		hatch = cadzinho.new_hatch(bounds, "solid")
		cadzinho.ent_append(hatch)
	end
end

for i = 250, 255 do
	cadzinho.set_color(i)
	bounds = {
		{x = (i-249)*15, y = -90},
		{x = (i-249)*15, y = -90+10},
		{x = (i-249)*15+10, y = -90+10},
		{x = (i-249)*15+10, y = -90}
	}

	hatch = cadzinho.new_hatch(bounds, "solid")
	cadzinho.ent_append(hatch)
end

cadzinho.set_color("by layer")