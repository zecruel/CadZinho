cadzinho.set_layer("0")
cadzinho.set_ltype("Continuous")
cadzinho.set_style("Standard")


for i = 1, 9 do
	cadzinho.set_color(7)
	text = cadzinho.new_text(i * 15 + 5, 92, i, 2.5, "center", "bottom")
	cadzinho.ent_append(text)

	cadzinho.set_color(i)
	bounds = {
		{x = i*15, y = 80},
		{x = i*15, y = 80+10},
		{x = i*15+10, y = 80+10},
		{x = i*15+10, y = 80}
	}

	hatch = cadzinho.new_hatch(bounds, "solid")
	cadzinho.ent_append(hatch)
end

cadzinho.set_color(7)
for i = 10, 240, 10 do
	text = cadzinho.new_text(i + 5, 62, i, 2.5, "center", "bottom")
	cadzinho.ent_append(text)
end

for i = 0, 8, 2 do
	text = cadzinho.new_text(5, -5*i+5, i, 2.5, "right", "middle")
	cadzinho.ent_append(text)
end

for i = 1, 9, 2 do
	text = cadzinho.new_text(5, 5*i+10, i, 2.5, "right", "middle")
	cadzinho.ent_append(text)
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
	cadzinho.set_color(7)
	text = cadzinho.new_text((i-249)*15 + 5, -58, i, 2.5, "center", "bottom")
	cadzinho.ent_append(text)

	cadzinho.set_color(i)
	bounds = {
		{x = (i-249)*15, y = -70},
		{x = (i-249)*15, y = -70+10},
		{x = (i-249)*15+10, y = -70+10},
		{x = (i-249)*15+10, y = -70}
	}

	hatch = cadzinho.new_hatch(bounds, "solid")
	cadzinho.ent_append(hatch)
end

cadzinho.set_color("by layer")