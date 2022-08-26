-- CadZinho script file - Plugin: Catenary Curves
-- This file is writen in Lua language

-- Global variables
count = 0  -- points entered
a_param = {value = '5'} -- catenary parameter, for GUI
length = {value = '100'} -- chain length, for GUI
option = {value = 1, "by Tension param", "by Length"}    -- option of type, for GUI
x_start = 0.0
y_start = 0.0
x_end = 0.0
y_end = 0.0
hyp = require "hyperbolic" -- hyperbolic functions

-- catenary equation
-- y = a*cosh(x/a) + C
-- where:
--    x origin is vertice of curve
--    a = catenary parameter (proportional to tension in chain)
--    cosh() = hyperbolic cosine function

-- Function to sample a catenary curve
function catenary(a,h,v)
    -- input data
    --a = catenary parameter
    --h = horizontal distance between anchor points
    --v = vertical distance between anchor points
    -- return table with sampled catenary coordinates points
    
    --determine s = length of chain
    s = math.sqrt((2*a*hyp.sinh(h/(2*a)))^2 + v^2)

    -- calcule horizontal shift of lowest point
    x0 = -hyp.atanh(v/s)*a

    -- get catenary curve points
    step = h/20 -- 20 points sample
    umin = -h/2
    umax = h/2

    u = umin
    y0 = a * hyp.cosh ((u-x0) / a) -- vertical shift
    
    cat_curve = {}
    for i = 1, math.floor((umax - umin) / step) do
        cat_curve[i] = {}
        cat_curve[i].x = u - umin
        cat_curve[i].y = a * hyp.cosh ((u-x0) / a) - y0
        u = u + step
    end
    -- last point of catenary
    i = #cat_curve + 1
    cat_curve[i] = {}
    cat_curve[i].x = umax - umin
    cat_curve[i].y = a * hyp.cosh ((umax-x0) / a) - y0
    
    return cat_curve
end

-- Function to compute the catenary parameter of a fixed chain lenght
function get_cat_a(s,h,v)
    -- input data
    --s = total chain lenght
    --h = horizontal distance between anchor points
    --v = vertical distance between anchor points

    -- transcendental equation of catenary for determining "a" (quadratic form is more stable in numeric evaluation)
    --((2*a*sinh(h/(2*a)) - sqrt(s^2 - v^2))^2) = 0  where
    -- sinh() = hyperbolic sine function

    -- its derivative (https://www.wolframalpha.com/input/?i=d%28%282a*sinh%28h%2F%282a%29%29-sqrt%28s%5E2-v%5E2%29%29%5E2%29%2Fda)
    --2*(2*sinh(h/(2*a)) - (h*cosh(h/(2*a)))/a)*(2*a*sinh(h/(2*a)) - sqrt(s^2 - v^2))

    --d/(da)(2 a sinh(h/(2 a)) - sqrt(s^2 - v^2)) = 2 sinh(h/(2 a)) - (h cosh(h/(2 a)))/a

    if s^2 > h^2 + v^2 then -- verify if exists a valid result

        -- initial value for "a" parameter
        a = h/((math.log(s/((h^2+v^2)^0.5)))^0.5)/5

        prev_a = 0 -- save "a" previous value to evaluate error
        n= 0 -- iterations 
        
        -- Newton method to evaluate "a" value
        for i = 1, 100 do -- max 100 iterations
            prev_a = a
            n = i
            a = a - ((2*a*hyp.sinh(h/(2*a)) - (s^2 - v^2)^0.5)^2) / (2*(2*hyp.sinh(h/(2*a)) - (h*hyp.cosh(h/(2*a)))/a)*(2*a*hyp.sinh(h/(2*a)) - (s^2 - v^2)^0.5))
            --a = a - (2*a*hyp.sinh(h/(2*a)) - math.sqrt(s^2 - v^2)) / (2*hyp.sinh(h/(2*a)) - (h*hyp.cosh(h/(2*a)))/a)
            a = math.abs(a)
            -- stop criteria (error < 0.01%)
            if (math.abs(prev_a - a)/a < 0.0001) then 
                return a -- return success
            end
        end
    end
    
    return nil -- return fail
end

function catenary_dyn(event)
    cadzinho.nk_layout(20, 1)
    cadzinho.nk_label("Catenary Curve")-- Simple information: script title

    cadzinho.nk_option(option)   -- type of calc, by parameter or by length
    
    if option.value == 1 then
        cadzinho.nk_propertyd("Tension", a_param) -- catenary parameter value
    else
        cadzinho.nk_propertyd("Length", length) -- chain length value
    end
    
    cadzinho.nk_layout(20, 1)
    if count == 0 then  -- first point
        cadzinho.nk_label('Enter start point')  -- information to the user
        if event.type == 'enter' then
            count = count + 1 -- go to next point
            -- store first point
            x_start = event.x
            y_start = event.y
        elseif event.type == 'cancel' then
            cadzinho.stop_dynamic() -- canceled by user: quit script
        end
    elseif count == 1 then -- second point
        cadzinho.nk_label('Enter end point')

        h = math.abs(event.x - x_start)
        v = event.y - y_start
        s = math.abs(tonumber(length.value))
        a = math.abs(tonumber(a_param.value))
        
        dir =1.0
        if (event.x - x_start) < 0.0 then
            dir = -1.0
        end
        if option.value > 1 then -- if catenary is by chain length
            a = get_cat_a(s,h,v) -- get a parameter
        else
            if h == 0 then a = nil end
        end
        if (a) then
            cat_curve = catenary(a,h,v)
            -- generate curve in drawing with a polyline
            x1 = cat_curve[1].x * dir + x_start
            y1 = cat_curve[1].y + y_start
            x2 = cat_curve[2].x * dir + x_start
            y2 = cat_curve[2].y + y_start

            pline = cadzinho.new_pline(x1, y1, 0, x2, y2, 0)
            for i = 3, #cat_curve do
                x2 = cat_curve[i].x * dir + x_start
                y2 = cat_curve[i].y + y_start
                cadzinho.pline_append(pline, x2, y2, 0)
            end
            cadzinho.ent_draw(pline)
        else
            pline = nil
        end
        
        
        if event.type == 'enter' then
            -- store second point
            x_end = event.x
            y_end = event.y
            
            if option.value == 1 then -- if catenary is by parameter
                count = count + 1 -- go to next point (confirmation
            elseif pline then
                pline:write()
                count = 0 -- restart
            end
        elseif event.type == 'cancel' then -- user cancel
            count = 0  -- restart
        end
    else
        cadzinho.nk_label('Confirm')
        
        h = math.abs(x_end - x_start)
        v = y_end - y_start
        a = math.abs(tonumber(a_param.value))
        
        dir =1.0
        if (x_end - x_start) < 0.0 then
            dir = -1.0
        end

        cat_curve = catenary(a,h,v)
        
        -- generate curve in drawing
        x1 = cat_curve[1].x * dir + x_start
        y1 = cat_curve[1].y + y_start
        x2 = cat_curve[2].x * dir + x_start
        y2 = cat_curve[2].y + y_start

        pline = cadzinho.new_pline(x1, y1, 0, x2, y2, 0)
        for i = 3, #cat_curve do
            x2 = cat_curve[i].x * dir + x_start
            y2 = cat_curve[i].y + y_start
            cadzinho.pline_append(pline, x2, y2, 0)
        end
        cadzinho.ent_draw(pline)
        
        if event.type == 'enter' then
            if (pline) then
                pline:write()
            end
            
            count = 0 -- restart
        elseif event.type == 'cancel' then -- user cancel
            count = 0 -- restart
        end
    end
end

-- Starts the dynamic mode in main loop
cadzinho.start_dynamic("catenary_dyn")