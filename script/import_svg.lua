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
      if pline then
        cadzinho.pline_append(pline, currx, curry, 0)
      else
        pline = cadzinho.new_pline(prevx, prevy, 0, currx, curry, 0)
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
    for j, path in ipairs(shape) do
      local pline
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