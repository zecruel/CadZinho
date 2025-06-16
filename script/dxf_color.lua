

function rgb2hsv( r, g, b )
  local min, max, abs = math.min, math.max, math.abs
	local M, m = max( r, g, b ), min( r, g, b )
	local C = M - m
	local K = 60.0/C
	local h = 0.0
	if C ~= 0.0 then
		if M == r then     h = ((g - b) * K) % 360.0
		elseif M == g then h = (b - r) * K + 120
		else               h = (r - g) * K + 240
		end
	end
	return h, M == 0.0 and 0.0 or 255 * C / M, M
end


function rgb2dxf( r, g, b )
  local h, s, v = rgb2hsv( r, g, b )
  if s < 50 and v < 50 then --black
    return 7
  elseif s < 50 then --gray
    un = math.floor((255 - v) / 40 + 0.5)
    return 255 - un
  elseif s >= 50 and s < 170 then --odd
    un = math.min(math.ceil((255 - v) / 40.8), 4)
    d = 1 + h // 15
    return d * 10 + 2 * un + 1
  elseif s >= 170 then --even
    un = math.min(math.ceil((255 - v) / 40.8), 4)
    d = 1 + h // 15
    return d * 10 + 2 * un
  else
    return 7
    
  end
  
  
end

--[[cadzinho.db_print("250=", rgb2hsv( 0.2, 0.2, 0.2 ))
cadzinho.db_print("251=", rgb2hsv( 0.357, 0.357, 0.357 ))
cadzinho.db_print("252=", rgb2hsv( 0.518, 0.518, 0.518 ))
cadzinho.db_print("253=", rgb2hsv( 0.678, 0.678, 0.678 ))
cadzinho.db_print("254=", rgb2hsv( 0.839, 0.839, 0.839 ))
cadzinho.db_print("255=", rgb2hsv( 1, 1, 1 ))
cadzinho.db_print("8=", rgb2hsv( 0.255, 0.255, 0.255 ))
cadzinho.db_print("9=", rgb2hsv( 0.5, 0.5, 0.5 ))
cadzinho.db_print("11=", rgb2hsv( 1, 0.5, 0.5 ))
cadzinho.db_print("17=", rgb2hsv( 0.584, 0.294, 0.294 ))
cadzinho.db_print("19=", rgb2hsv( 0.447, 0.224, 0.224 ))
cadzinho.db_print("12=", rgb2hsv( 0.86667, 0, 0 ))
cadzinho.db_print("18=", rgb2hsv( 0.447, 0, 0 ))]]--

cadzinho.db_print("2=", rgb2hsv( 255, 255, 0 ))
cadzinho.db_print("250=", rgb2hsv( 51, 60, 51 ))
cadzinho.db_print("251=", rgb2hsv( 91, 91, 91 ))
cadzinho.db_print("252=", rgb2hsv( 132, 132, 132 ))
cadzinho.db_print("253=", rgb2hsv( 173, 173, 173))
cadzinho.db_print("254=", rgb2hsv( 214, 214, 214 ))
cadzinho.db_print("255=", rgb2hsv( 255, 255, 255 ))
cadzinho.db_print("8=", rgb2hsv( 128, 128, 128 ))
cadzinho.db_print("9=", rgb2hsv( 192, 192, 192 ))
cadzinho.db_print("11=", rgb2hsv( 255, 128, 128 ))
cadzinho.db_print("17=", rgb2hsv( 149, 75, 75 ))
cadzinho.db_print("19=", rgb2hsv( 114, 57, 57 ))
cadzinho.db_print("12=", rgb2hsv( 221, 0, 0 ))
cadzinho.db_print("18=", rgb2hsv( 114, 0, 0 ))

cadzinho.db_print("250=", rgb2dxf( 51, 60, 51 ))
cadzinho.db_print("251=", rgb2dxf( 91, 91, 91 ))
cadzinho.db_print("252=", rgb2dxf( 132, 132, 132 ))
cadzinho.db_print("253=", rgb2dxf( 173, 173, 173))
cadzinho.db_print("254=", rgb2dxf( 214, 214, 214 ))
cadzinho.db_print("255=", rgb2dxf( 255, 255, 255 ))
cadzinho.db_print("2=", rgb2dxf( 255, 255, 0 ))
cadzinho.db_print("11=", rgb2dxf( 255, 128, 128 ))
cadzinho.db_print("17=", rgb2dxf( 149, 75, 75 ))
cadzinho.db_print("19=", rgb2dxf( 114, 57, 57 ))
cadzinho.db_print("12=", rgb2dxf( 221, 0, 0 ))
cadzinho.db_print("18=", rgb2dxf( 114, 0, 0 ))
cadzinho.db_print("145=", rgb2dxf( 92, 161, 184 ))