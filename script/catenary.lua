-- catenary equation
-- y = a*cosh(x/a) + C
-- where x origin is vertice of curve

-- transcendental equation of catenary for determining "a" (quadratic form is more stable in numeric evaluation)
--((2*a*sinh(h/(2*a)) - sqrt(s^2 - v^2))^2) = 0  where
-- s = length of chain
-- h = horizontal distance
-- v = vertical distance
-- sinh() = hyperbolic sine function

-- its derivative (https://www.wolframalpha.com/input/?i=d%28%282a*sinh%28h%2F%282a%29%29-sqrt%28s%5E2-v%5E2%29%29%5E2%29%2Fda)
--2*(2*sinh(h/(2*a)) - (h*cosh(h/(2*a)))/a)*(2*a*sinh(h/(2*a)) - sqrt(s^2 - v^2))

--d/(da)(2 a sinh(h/(2 a)) - sqrt(s^2 - v^2)) = 2 sinh(h/(2 a)) - (h cosh(h/(2 a)))/a

package.path = package.path .. ";D:\\documentos\\cadzinho\\script\\?.lua"
local hyp = require "hyperbolic"

-- input data
s = 2400 -- total chain lenght
h = 2000 -- horizontal distance between anchor points
v = 1000 -- vertical distance between anchor points

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
			break
		end
	end

	cadzinho.db_print("a="..a, "e="..math.abs(prev_a - a), "iterations="..n)
	
	-- calcule horizontal shift of lowest point
	x0 = -hyp.atanh(v/s)*a
	cadzinho.db_print("x0"..x0)

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
		cat_curve[i].b = 0
		u = u + step
	end
	-- last point of catenary
	i = #cat_curve + 1
	cat_curve[i] = {}
	cat_curve[i].x = umax - umin
	cat_curve[i].y = a * hyp.cosh ((umax-x0) / a) - y0
	cat_curve[i].b = 0
	
	-- generate curve in drawing
	x1 = cat_curve[1].x
	y1 = cat_curve[1].y
	x2 = cat_curve[2].x
	y2 = cat_curve[2].y

	pline = cadzinho.new_pline(x1, y1, cat_curve[1].b, x2, y2, cat_curve[2].b)
	for i = 3, #cat_curve do
		x2 = cat_curve[i].x
		y2 = cat_curve[i].y
		cadzinho.pline_append(pline, x2, y2, cat_curve[i].b)
	end
	cadzinho.ent_append(pline)
end