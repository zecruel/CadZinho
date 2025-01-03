sphere = cadzinho.new_mesh("manifold[1] = sphere(1);manifold[1]:translate(3,3,3)",{color = 4})
sphere:write()

sphere = cadzinho.new_mesh("manifold[1] = sphere(2);manifold[1]:translate(3,-3,3)",{color = 5})
sphere:write()

sphere = cadzinho.new_mesh(
  "manifold[1] = cylinder(10, 2, 2)\n"..
  "manifold[1]:rotate(0,90,0)\n" ..
  "manifold[1]:translate(10,0,0)\n",{color = 2})
sphere:write()


slab = cadzinho.new_mesh("manifold[1] = slab(2.5,1.0,0.5)\n" ..
  "manifold[1]:rotate(5,5,5)\n" ..
  "manifold[1]:translate(-5,-5,-5)\n" ..
  "manifold[1]:scale(10,2,1)\n",{color = 1})
slab:write()

sphere = cadzinho.new_mesh(
  "manifold[1] = cylinder(4, 4, 2)\n"..
  "manifold[1]:rotate(90,0,0)\n" ..
  "manifold[1]:translate(-2,0,0)\n",{color = 2})
sphere:write()

--sphere = cadzinho.new_mesh("manifold[1] = union(sphere(), cylinder(10))",{color = 1})
sphere = cadzinho.new_mesh("manifold[1] = sphere()\n"..
  "manifold[2] = cylinder(4, 0.7, 0.7)\n"..
  "manifold[2]:rotate(0,90,0)\n" ..
  "manifold[2]:translate(-2,0,0)\n" ..
  "manifold[3] = difference(manifold[1], manifold[2])\n" ..
  "manifold[3]:translate(2,2,0)\n",{color = 3})
--sphere = cadzinho.new_mesh("manifold[1] = intersection(sphere(), cylinder(10))",{color = 1})
sphere:write()