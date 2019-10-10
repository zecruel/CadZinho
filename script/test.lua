function getNumber()
   local function getNumberHelper()
      co = coroutine.create(function ()
      coroutine.yield(1)
      coroutine.yield(2)
      coroutine.yield(3)
      coroutine.yield(4)
      coroutine.yield(5)
      end)
      return co
   end
	
   if(numberHelper) then
      status, number = coroutine.resume(numberHelper);
		
      if coroutine.status(numberHelper) == "dead" then
         numberHelper = getNumberHelper()
         status, number = coroutine.resume(numberHelper);
      end
		
      return number
   else
      numberHelper = getNumberHelper()
      status, number = coroutine.resume(numberHelper);
      return number
   end
	
end

for index = 1, 10 do
   print(index, getNumber())
end

-- Test lua script
soma = 3+4
teste = "texto = teste"
cadzinho.db_print("testing send debug messages\n")
cadzinho.db_print("1+3=", 1+3)
cadzinho.db_print("soma")
cadzinho.db_print("teste")
cadzinho.db_print(1+3/2)

--teste2 = cadzinho+soma

cadzinho.set_timeout(1)

cadzinho.set_layer("0")
cadzinho.set_ltype("Continuous")
cadzinho.set_color(1)
pts = 32
step = 1/pts

x = step
y = math.sin(2*math.pi*x)

pline = cadzinho.new_pline(0,0,0,x,y,0)
--cadzinho.pline_append(pline, 20,0,-1)

for i = 2, 100 do 
	x = x + step
	y = math.sin(2*math.pi*x)
	cadzinho.pline_append(pline, x, y,0)
end

cadzinho.ent_append(pline)

cadzinho.set_color("by layer")
cx = 5
cy = 2
r = 2
n = 5

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

cadzinho.set_lw(14)

circle = cadzinho.new_circle(cx,cy,a)
cadzinho.ent_append(circle)


while(true)
do
end

for i = 1, 152 do print(i) end