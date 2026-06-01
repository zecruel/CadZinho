qrencode = dofile("qrencode.lua")

local ok, res = qrencode.qrcode("https://docs.google.com/document/d/1A4256vdVvITOqPhYRV9Tkcv2rPM-ap83/edit?usp=drivesdk&ouid=106716562684859056625&rtpof=true&sd=true")
if not ok then
    cadzinho.db_print(res)
else

  cadzinho.set_color(255)
  bounds = {
    {x = -20, y = 30},
    {x = -20, y = -10 * #res - 30},
    {x = 10 * #res + 40, y = -10 * #res - 30},
    {x = 10 * #res + 40, y = 30}
  }
  hatch = cadzinho.new_hatch(bounds, "solid")
  hatch:write()
    
  cadzinho.db_print("ok", #res, #res[1])
  --str = ""
  for i = 1, #res do
    for j = 1, #res[i] do
      --str = str .. tostring(res[i][j]) .. " "
      
      if res[i][j] > 0 then
        cadzinho.set_color(250)
        bounds = {
          {x = 10*i, y = -10*j},
          {x = 10*i, y = -10*j+10},
          {x = 10*i+10, y = -10*j+10},
          {x = 10*i+10, y = -10*j}
        }
        hatch = cadzinho.new_hatch(bounds, "solid")
        hatch:write()
      end
      
    end
    --cadzinho.db_print(str)
    --str = ""
  end
end