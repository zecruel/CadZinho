-- CadZinho script file - Plugin: import SVG
-- This file is writen in Lua language

-- Global variables
file = {value = ''}  -- svg file entry, for GUI
valid = false -- check if is a valid file
msg = '' -- message for success or fail, for GUI

-- Function to get file name, without path and extension
function GetFileName(url)
  return url:match("^.+[\\/]([^\\/]+)$")
end

-- Function to get file extension
function GetFileExtension(url)
  return url:match("^.+%.([^%.]+)$")
end

-- convert RGB color to aproximate DXF color index
local function rgb_color_i (r, g, b) -- color components 0 - 255
  local max_val = math.max(r, g, b)
  local min_val = math.min(r, g, b)

  -- calculate Luminance
  l = max_val + min_val
  if l < 100 then return 250 -- black
  elseif l > 410 then return 255 end -- white
  
  local delta = max_val - min_val
  
  -- calculate Saturation
  local s = delta / (255 - math.abs(l - 255))
  if s < 0.2 then -- gray scale
    return 250 + math.floor(0.5 + (l - 100) / 62)
  end
  
  -- Calculate Hue
  local h
  if delta == 0 then
    h = 10
  elseif max_val == r then
    h = 40 * ((g - b) / delta)
    if h >= 0 then h = math.floor(h + 10.5)
    else h = math.floor(h + 250.5) end
  elseif max_val == g then
    h = math.floor(40 * ((b - r) / delta) + 80.5)
  else
    h = math.floor(40 * ((r - g) / delta) + 170.5)
  end
  
  --print ("RGB",r,g, b,"HSL",h,s,l)
  
  if s > 0.8 and l <= 280 then
    return h + (math.floor((280 - l) / 43) * 2)
  else
    return h + (math.floor((410 - l) / 58) * 2) + 1
  end
  
end

local function hex2rgba(hex)
  if type(hex) ~= 'string' then return nil end
  local a = tonumber(hex:sub(1, 2), 16)
  local b = tonumber(hex:sub(3, 4), 16)
  local g = tonumber(hex:sub(5, 6), 16)
  local r = tonumber(hex:sub(7, 8), 16)
  --print ("HEX", hex)
  
  return { r = r, g = g, b = b, a = a }
end
  
local function cubicbezier(pline, p1, p2, p3, p4, nseg)
  nseg = nseg or 10
  local prevx, prevy, currx, curry
  for i = 0, nseg do
    local t = i / nseg
    local a, b, c, d = (1-t)^3, 3*t*(1-t)^2, 3*t^2*(1-t), t^3
    prevx, prevy = currx, curry
    currx = a * p1.x + b * p2.x + c * p3.x + d * p4.x
    curry = a * p1.y + b * p2.y + c * p3.y + d * p4.y
    if i == 1 then
      if type(pline) == 'userdata' then
        cadzinho.pline_append(pline, currx, curry, 0)
      else
        local param
        if type(pline) == 'number' then param = {color = pline} end
        pline = cadzinho.new_pline(prevx, prevy, 0, currx, curry, 0, param)
      end
    elseif i > 1 then
      cadzinho.pline_append(pline, currx, curry, 0)
    else
    end
  end
  return pline
end
  
local function import (url)

  cadzinho.set_timeout(180)

  local f = io.open(url)
  if not f then return false end
  curves = cadzinho.svg_curves(f:read('a'))
  f:close()
  if not curves then return false end

  for i, shape in ipairs(curves) do
    local color = hex2rgba(shape.stroke) or hex2rgba(shape.fill)
    if color then
      color = rgb_color_i (color.r, color.g, color.b)
    end
    
    for j, path in ipairs(shape) do
      local pline = color
      for k = 1, #path-3, 3 do
        pline = cubicbezier(pline, path[k], path[k+1], path[k+2], path[k+3])
      end
      
      if pline then
        --cadzinho.pline_close(pline, path.closed)
        pline:write()
      end
      
    end
  end
  
  return true
end

-- Import GUI
function svg_imp_win()

  cadzinho.nk_layout(20, 1)
  cadzinho.nk_label("File:")
  if cadzinho.nk_edit(file) then
    valid = false
    msg = ''
    local ext = GetFileExtension(file.value)
    if type(ext) == "string" then
      if ext:upper() == "SVG" then
        valid = true
      else
        msg = 'Not a valid file name'
      end
    end
  end
  
  if valid then
    if cadzinho.nk_button("Import") then
      if import (file.value) then
        msg = 'Success!'
      else
        msg = 'Fail to import'
      end
    end
  end
  
  cadzinho.nk_label(msg)
end

-- Starts the window
cadzinho.win_show("svg_imp_win", "Import SVG", 500,100,300,150)