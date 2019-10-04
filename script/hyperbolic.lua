-----------------------------------------------------------------------
-- Pure Lua implementation for the hyperbolic trigonometric functions
-- Freely adapted from P.J.Plauger, "The Standard C Library"
-- author: Roberto Ierusalimschy
-----------------------------------------------------------------------

local exp = math.exp

local function cosh (x)
  if x == 0.0 then return 1.0 end
  if x < 0.0 then x = -x end
  x = exp(x)
  x = x / 2.0 + 0.5 / x
  return x
end


local function sinh (x)
  if x == 0 then return 0.0 end
  local neg = false
  if x < 0 then x = -x; neg = true end
  if x < 1.0 then
    local y = x * x
    x = x + x * y *
        (((-0.78966127417357099479e0  * y +
           -0.16375798202630751372e3) * y +
           -0.11563521196851768270e5) * y +
           -0.35181283430177117881e6) /
        ((( 0.10000000000000000000e1  * y +
           -0.27773523119650701667e3) * y +
            0.36162723109421836460e5) * y +
           -0.21108770058106271242e7)
  else
    x =  exp(x)
    x = x / 2.0 - 0.5 / x
  end
  if neg then x = -x end
  return x
end


local function tanh (x)
  if x == 0 then return 0.0 end
  local neg = false
  if x < 0 then x = -x; neg = true end
  if x < 0.54930614433405 then
    local y = x * x
    x = x + x * y *
        ((-0.96437492777225469787e0  * y +
          -0.99225929672236083313e2) * y +
          -0.16134119023996228053e4) /
        (((0.10000000000000000000e1  * y +
           0.11274474380534949335e3) * y +
           0.22337720718962312926e4) * y +
           0.48402357071988688686e4)
  else
    x = exp(x)
    x = 1.0 - 2.0 / (x * x + 1.0)
  end
  if neg then x = -x end
  return x
end

-- invhyp.lua: inverse hyperbolic trig functions
-- Adapted from glibc
local abs, log, sqrt = math.abs, math.log, math.sqrt
local log2 = log(2)

-- good for IEEE754, double precision
local function islarge (x) return x > 2 ^ 28 end
local function issmall (x) return x < 2 ^ (-28) end

local INF = math.huge
local function isinfornan (x)
  return x ~= x or x == INF or x == -INF
end

local function log1p (x) -- not very precise, but works well
  local u = 1 + x
  if u == 1 then return x end -- x < eps?
  return log(u) * x / (u - 1)
end

local function acosh (x)
  if x < 1 then return (x - x) / (x - x) end -- nan
  if islarge(x) then
    if isinfornan(x) then return x + x end
    return log2 + log(x)
  end
  if x + 1 == 1 then return 0 end -- acosh(1) == 0
  if x > 2 then
    local x2 = x * x
    return log(2 * x - 1 / (x + sqrt(x2 - 1)))
  end
  -- 1 < x < 2:
  local t = x - 1
  return log1p(t + sqrt(2 * t + t * t))
end

local function asinh (x)
  local y = abs(x)
  if issmall(y) then return x end
  local a
  if islarge(y) then -- very large?
    if isinfornan(x) then return x + x end
    a = log2 + log(y)
  else
    if y > 2 then
      a = log(2 * y + 1 / (y + sqrt(1 + y * y)))
    else
      local y2 = y * y
      a = log1p(y + y2 / (1 + sqrt(1 + y2)))
    end
  end
  return x < 0 and -a or a -- transfer sign
end

local function atanh (x)
  local y = abs(x)
  local a
  if y < .5 then
    if issmall(y) then return x end
    a = 2 * y
    a = .5 * log1p(a + a * y / (1 - y))
  else
    if y < 1 then
      a = .5 * log1p(2 * y / (1 - y))
    elseif y > 1 then
      return (x - x) / (x - x) -- nan
    else -- y == 1
      return x / 0 -- inf with sign
    end
  end
  return x < 0 and -a or a -- transfer sign
end

return {sinh = sinh, cosh = cosh, tanh = tanh, log1p = log1p, asinh = asinh, acosh = acosh, atanh = atanh}