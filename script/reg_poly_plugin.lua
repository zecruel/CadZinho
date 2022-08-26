-- CadZinho script file - Plugin: Create a Regular Polygon
-- This file is writen in Lua language

-- Parameters: 
--    - number of sides: 3 to 20
--    - option: Inscribed or Tangential to circle
--    - center: script will require interactively in drawing (enters a point by mouse click)
--    - circle radius: script will require interactively in drawing (enters a point by mouse click)
--    - rotation: calculated by points entered interactively

-- Global variables
count = 0    -- points entered
sides = {value = '5'}    -- number of sides, for GUI
option = {value = 1, "Inscribed", "Tangential"}    -- option of type, for GUI

-- Function to calculate the polygon
function do_reg_poly(cx, cy, n, r, ang)
    -- parameters passed:
    -- center coordinates,
    -- number of sides
    -- circle radius, where polygon is inscribed
    -- angle of first vertex
    
    -- calcule the two first vertices
    local x0 = cx + r * math.cos(ang)
    local y0 = cy + r * math.sin(ang)
    local x = cx + r * math.cos(2*math.pi/n + ang)
    local y = cy + r * math.sin(2*math.pi/n + ang)
    
    -- create a polyline entity
    local pline = cadzinho.new_pline(x0, y0, 0, x, y, 0)
    
    -- calcule next vertices and append to polyline entity
    for i = 2, n - 1 do
        x = cx + r * math.cos(2*math.pi*i/n + ang)
        y = cy + r * math.sin(2*math.pi*i/n + ang)
        cadzinho.pline_append(pline, x,y,0)
    end

    cadzinho.pline_close(pline, true)    -- close the polyline
    
    return pline -- return the polyline
end

-- Function to operate the interactive mode and GUI 
function reg_poly_dyn(event)
    cadzinho.nk_layout(20, 1)
    cadzinho.nk_label("Regular polygon") -- Simple information: script title
    
    cadzinho.nk_layout(20, 1)
    cadzinho.nk_option(option)   -- type of polygon, inscribed or tangential
    cadzinho.nk_propertyi("Sides", sides, 3, 20) -- number of sides
    
    cadzinho.nk_layout(20, 1)
    if count == 0 then -- first point: center
        cadzinho.nk_label('Enter center') -- information to the user
        if event.type == 'enter' then
            count = count + 1 -- go to next point
            cx = event.x
            cy = event.y
        elseif event.type == 'cancel' then
            cadzinho.stop_dynamic() -- canceled by user: quit script
        end
    else -- second point: radius and rotation
        cadzinho.nk_label('Enter radius') -- information to the user
        
        -- calculate the parameters
        r = ((event.x - cx)^2 + (event.y - cy)^2) ^ 0.5
        ang = math.atan((event.y - cy), (event.x - cx))
        n = tonumber(sides.value)
        
        -- draw a circle to guide the user, graphicaly
        circle = cadzinho.new_circle(cx,cy,r)
        cadzinho.ent_draw(circle)
        
        -- if tangetial polygon is choosen
        if option.value > 1 then
            -- adjust radius and angle to pass to funtion
            r = r / (math.cos(math.pi / n))
            ang = ang + math.pi / n
        end
        
        -- create and draw the result polyline, the polygon
        pline = do_reg_poly(cx, cy, n, r, ang)
        cadzinho.ent_draw(pline)
        
        if event.type == 'enter' then -- user confirms
            pline:write()  -- add polygon to drawing
            count = 0 -- restart
        elseif event.type == 'cancel' then -- user cancel
            count = 0 -- restart
        end
    end
end

-- Starts the dynamic mode in main loop
cadzinho.start_dynamic("reg_poly_dyn")