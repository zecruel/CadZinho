count = 0
radius = {value = '4'}
pyr_top_scale = {value = '0'}
pyr_heigth = {value = '1'}
slab_heigth = {value = '1'}
extr_top_scalex = {value = '1'}
extr_top_scaley = {value = '1'}
extr_heigth = {value = '1'}
extr_slices = {value = '0'}
extr_twist = {value = '0'}
contour = "{{-0.5,-0.5},{0.5,-0.5},{0.5,-0.4},{-0.4,-0.4},{-0.4,0.5},{-0.5,0.5}}"
path = "{{0.0,0.0,0.0},{10,10,0.0},{15,10,0.0},\n{25,5,0.0},{25,0.0,0.0}}"

function inter3d_dyn(event)
  cadzinho.nk_layout(20, 1)
  cadzinho.nk_label("3D Object")-- Simple information: script title
	if count == 0 then
    cadzinho.nk_label("First point")
		if event.type == 'enter' then
			count = count + 1
			x0 = event.x
			y0 = event.y
			
		elseif event.type == 'cancel' then
			cadzinho.stop_dynamic()
		end
	else
    cadzinho.nk_label("Next point")
		x1 = event.x
		y1 = event.y
		if event.type == 'motion' then
			--r = tonumber(radius.value)
      --pline = cadzinho.new_pline(xx0, yy0+r, 0, xx0+r, yy0, 0.414214)
      --cadzinho.ent_draw(pline)
			
			
		elseif event.type == 'enter' then
			--r = tonumber(radius.value)
      --pline = cadzinho.new_pline(xx0, yy0+r, 0, xx0+r, yy0, 0.414214)
      --pline:write()
			
      -- back to first point
			count = 0
			
		elseif event.type == 'cancel' then
			-- back to first point
			count = 0
		end
	end
end

function sphere_dyn(event)
  cadzinho.nk_layout(20, 1)
  cadzinho.nk_label("New Sphere")-- Simple information: script title
	if count == 0 then
    cadzinho.nk_label("Center point")
		if event.type == 'enter' then
			count = count + 1
			x0 = event.x
			y0 = event.y
			
		elseif event.type == 'cancel' then
			cadzinho.stop_dynamic()
		end
	else
    cadzinho.nk_label("Radius")
		x1 = event.x
		y1 = event.y
    r = math.sqrt( (x1 - x0)^2 + (y1 - y0)^2 )
		
    sphere = cadzinho.new_mesh("manifold[1] = sphere(" .. r .. ")\n" ..
        "manifold[1]:translate(" .. x0 .. "," .. y0 .. ",0)")
    cadzinho.ent_draw(sphere)
			
		if event.type == 'enter' and r > 0.0 then
      sphere:write()
			
      -- back to first point
			count = 0
			
		elseif event.type == 'cancel' then
			-- back to first point
			count = 0
		end
	end
end

function pyramid_dyn(event)
  cadzinho.nk_layout(20, 1)
  cadzinho.nk_label("New pyramid")-- Simple information: script title
  cadzinho.nk_propertyd("Heigth", pyr_heigth)
  cadzinho.nk_propertyd("Scale", pyr_top_scale, 0)
	if count == 0 then
    cadzinho.nk_label("Center point")
		if event.type == 'enter' then
			count = count + 1
			x0 = event.x
			y0 = event.y
			
		elseif event.type == 'cancel' then
			cadzinho.stop_dynamic()
		end
	else
    cadzinho.nk_label("Base")
		x1 = event.x
		y1 = event.y
    z = 0.0
    w = x1 - x0
    h = y1 - y0
    p = tonumber(pyr_heigth.value)
    s = tonumber(pyr_top_scale.value)
    x = x0 + w/2.0
    y = y0 + h/2.0
    
    rot = ""
    if p < 0 then rot = "manifold[1]:rotate(180,0,0)\n" end
    
		
    pyramid = cadzinho.new_mesh("manifold[1] = pyramid(".. math.abs(w) ..",".. math.abs(h) ..",".. math.abs(p) ..",".. s ..")\n" ..
        rot .. "manifold[1]:translate(" .. x ..",".. y .. "," .. z ..")")
    cadzinho.ent_draw(pyramid)
			
		if event.type == 'enter' then
      pyramid:write()
			
      -- back to first point
			count = 0
			
		elseif event.type == 'cancel' then
			-- back to first point
			count = 0
		end
	end
end

function extrude_dyn(event)
  cadzinho.nk_layout(20, 1)
  cadzinho.nk_label("New extrude")-- Simple information: script title
  cadzinho.nk_propertyd("Heigth", extr_heigth)
  cadzinho.nk_propertyd("Sx", extr_top_scalex, 0)
  cadzinho.nk_propertyd("Sy", extr_top_scaley, 0)
  cadzinho.nk_propertyi("Slices", extr_slices, 0)
  cadzinho.nk_propertyd("Twist", extr_twist, 0)
	if count == 0 then
    cadzinho.nk_label("Center point")
		if event.type == 'enter' then
			count = count + 1
			x0 = event.x
			y0 = event.y
			
		elseif event.type == 'cancel' then
			cadzinho.stop_dynamic()
		end
	else
    cadzinho.nk_label("Confirm")
		x1 = event.x
		y1 = event.y
    z = 0.0
    p = tonumber(extr_heigth.value)
    sx = tonumber(extr_top_scalex.value)
    sy = tonumber(extr_top_scaley.value)
    slices = tonumber(extr_slices.value)
    twist = tonumber(extr_twist.value)
    
    rot = ""
    if p < 0 then
      twist = twist * -1
      rot = "manifold[1]:mirror(0,0,1)\n" 
    end
    
		
    extrude = cadzinho.new_mesh("manifold[1] = extrude("..
        contour ..",".. math.abs(p) ..",".. sx ..",".. sy ..",".. slices ..",".. twist ..")\n" ..
        rot .. "manifold[1]:translate(" .. x0 ..",".. y0 .. "," .. z ..")")
    cadzinho.ent_draw(extrude)
			
		if event.type == 'enter' then
      extrude:write()
			
      -- back to first point
			count = 0
			
		elseif event.type == 'cancel' then
			-- back to first point
			count = 0
		end
	end
end

function slab_dyn(event)
  cadzinho.nk_layout(20, 1)
  cadzinho.nk_label("New slab")-- Simple information: script title
  cadzinho.nk_propertyd("Heigth", slab_heigth)
	if count == 0 then
    cadzinho.nk_label("first corner")
		if event.type == 'enter' then
			count = count + 1
			x0 = event.x
			y0 = event.y
			
		elseif event.type == 'cancel' then
			cadzinho.stop_dynamic()
		end
	else
    cadzinho.nk_label("Base")
		x1 = event.x
		y1 = event.y
    x = x0
    y = y0
    z = 0.0
    w = x1 - x0
    h = y1 - y0
    p = tonumber(slab_heigth.value)
    if w < 0 then x = x - math.abs(w) end
    if h < 0 then y = y - math.abs(h) end
    if p < 0 then z = z - math.abs(p) end
		
    slab = cadzinho.new_mesh("manifold[1] = slab(".. math.abs(w) ..",".. math.abs(h) ..",".. math.abs(p) ..")\n" ..
        "manifold[1]:translate(" .. x ..",".. y .. "," .. z ..")")
    cadzinho.ent_draw(slab)
    
		if event.type == 'enter' then
      slab:write()
			
      -- back to first point
			count = 0
			
		elseif event.type == 'cancel' then
			-- back to first point
			count = 0
		end
	end
end

function rotate_dyn(event)
  cadzinho.nk_layout(20, 1)
  cadzinho.nk_label("rotate")-- Simple information: script title
  --cadzinho.nk_propertyd("Heigth", slab_heigth)
  
  
  local sel = cadzinho.get_sel()
  if #sel < 1 then
    cadzinho.nk_label("Select a element")
    count = 0
    cadzinho.enable_sel("-all+polyline")
  end
  if #sel > 0 then
    
    cadzinho.nk_label("Select pivot point")
		
	
	end
  if event.type == 'cancel' then
			cadzinho.stop_dynamic()
		end
end

function inter3d_win()
	cadzinho.nk_layout(20, 1)
  if cadzinho.nk_button("Sphere") then
    count = 0
    --cadzinho.start_dynamic("inter3d_dyn")
    cadzinho.start_dynamic("sphere_dyn")
  end
  if cadzinho.nk_button("pyramid") then
    count = 0
    cadzinho.start_dynamic("pyramid_dyn")
  end
  if cadzinho.nk_button("slab") then
    count = 0
    cadzinho.start_dynamic("slab_dyn")
  end
  if cadzinho.nk_button("Extrude") then
    count = 0
    cadzinho.start_dynamic("extrude_dyn")
  end
  if cadzinho.nk_button("Extrude_path") then
    extr = cadzinho.new_mesh("manifold[1] = extrude_path("..
        contour ..",\n".. path ..", 100)\nmanifold[1]:rotate(0,90)")
    extr:write()
  end
  if cadzinho.nk_button("rotate") then
    count = 0
    cadzinho.clear_sel()
    
    cadzinho.start_dynamic("rotate_dyn")
  end
	--cadzinho.nk_label("Radius:")
	--cadzinho.nk_edit(radius)
	
end

cadzinho.win_show("inter3d_win", "Toolbox 3d", 215,260,200,200)