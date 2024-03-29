cz_debug_breakpoints = {}
cz_debug_basedir = ""
cz_debug_stack = {}

cz_debug_serpent = (function() ---- include Serpent module for serialization
local n, v = "serpent", "0.302" -- (C) 2012-18 Paul Kulchenko; MIT License
local c, d = "Paul Kulchenko", "Lua serializer and pretty printer"
local snum = {[tostring(1/0)]='1/0 --[[math.huge]]',[tostring(-1/0)]='-1/0 --[[-math.huge]]',[tostring(0/0)]='0/0'}
local badtype = {thread = true, userdata = true, cdata = true}
local getmetatable = debug and debug.getmetatable or getmetatable
local pairs = function(t) return next, t end -- avoid using __pairs in Lua 5.2+
local keyword, globals, G = {}, {}, (_G or _ENV)
for _,k in ipairs({'and', 'break', 'do', 'else', 'elseif', 'end', 'false',
  'for', 'function', 'goto', 'if', 'in', 'local', 'nil', 'not', 'or', 'repeat',
  'return', 'then', 'true', 'until', 'while'}) do keyword[k] = true end
for k,v in pairs(G) do globals[v] = k end -- build func to name mapping
for _,g in ipairs({'coroutine', 'debug', 'io', 'math', 'string', 'table', 'os'}) do
  for k,v in pairs(type(G[g]) == 'table' and G[g] or {}) do globals[v] = g..'.'..k end end

local function s(t, opts)
  local name, indent, fatal, maxnum = opts.name, opts.indent, opts.fatal, opts.maxnum
  local sparse, custom, huge = opts.sparse, opts.custom, not opts.nohuge
  local space, maxl = (opts.compact and '' or ' '), (opts.maxlevel or math.huge)
  local maxlen, metatostring = tonumber(opts.maxlength), opts.metatostring
  local iname, comm = '_'..(name or ''), opts.comment and (tonumber(opts.comment) or math.huge)
  local numformat = opts.numformat or "%.17g"
  local seen, sref, syms, symn = {}, {'local '..iname..'={}'}, {}, 0
  local function gensym(val) return '_'..(tostring(tostring(val)):gsub("[^%w]",""):gsub("(%d%w+)",
    -- tostring(val) is needed because __tostring may return a non-string value
    function(s) if not syms[s] then symn = symn+1; syms[s] = symn end return tostring(syms[s]) end)) end
  local function safestr(s) return type(s) == "number" and tostring(huge and snum[tostring(s)] or numformat:format(s))
    or type(s) ~= "string" and tostring(s) -- escape NEWLINE/010 and EOF/026
    or ("%q"):format(s):gsub("\010","n"):gsub("\026","\\026") end
  local function comment(s,l) return comm and (l or 0) < comm and ' --[['..select(2, pcall(tostring, s))..']]' or '' end
  local function globerr(s,l) return globals[s] and globals[s]..comment(s,l) or not fatal
    and safestr(select(2, pcall(tostring, s))) or error("Can't serialize "..tostring(s)) end
  local function safename(path, name) -- generates foo.bar, foo[3], or foo['b a r']
    local n = name == nil and '' or name
    local plain = type(n) == "string" and n:match("^[%l%u_][%w_]*$") and not keyword[n]
    local safe = plain and n or '['..safestr(n)..']'
    return (path or '')..(plain and path and '.' or '')..safe, safe end
  local alphanumsort = type(opts.sortkeys) == 'function' and opts.sortkeys or function(k, o, n) -- k=keys, o=originaltable, n=padding
    local maxn, to = tonumber(n) or 12, {number = 'a', string = 'b'}
    local function padnum(d) return ("%0"..tostring(maxn).."d"):format(tonumber(d)) end
    table.sort(k, function(a,b)
      -- sort numeric keys first: k[key] is not nil for numerical keys
      return (k[a] ~= nil and 0 or to[type(a)] or 'z')..(tostring(a):gsub("%d+",padnum))
           < (k[b] ~= nil and 0 or to[type(b)] or 'z')..(tostring(b):gsub("%d+",padnum)) end) end
  local function val2str(t, name, indent, insref, path, plainindex, level)
    local ttype, level, mt = type(t), (level or 0), getmetatable(t)
    local spath, sname = safename(path, name)
    local tag = plainindex and
      ((type(name) == "number") and '' or name..space..'='..space) or
      (name ~= nil and sname..space..'='..space or '')
    if seen[t] then -- already seen this element
      sref[#sref+1] = spath..space..'='..space..seen[t]
      return tag..'nil'..comment('ref', level) end
    -- protect from those cases where __tostring may fail
    if type(mt) == 'table' and metatostring ~= false then
      local to, tr = pcall(function() return mt.__tostring(t) end)
      local so, sr = pcall(function() return mt.__serialize(t) end)
      if (to or so) then -- knows how to serialize itself
        seen[t] = insref or spath
        t = so and sr or tr
        ttype = type(t)
      end -- new value falls through to be serialized
    end
    if ttype == "table" then
      if level >= maxl then return tag..'{}'..comment('maxlvl', level) end
      seen[t] = insref or spath
      if next(t) == nil then return tag..'{}'..comment(t, level) end -- table empty
      if maxlen and maxlen < 0 then return tag..'{}'..comment('maxlen', level) end
      local maxn, o, out = math.min(#t, maxnum or #t), {}, {}
      for key = 1, maxn do o[key] = key end
      if not maxnum or #o < maxnum then
        local n = #o -- n = n + 1; o[n] is much faster than o[#o+1] on large tables
        for key in pairs(t) do if o[key] ~= key then n = n + 1; o[n] = key end end end
      if maxnum and #o > maxnum then o[maxnum+1] = nil end
      if opts.sortkeys and #o > maxn then alphanumsort(o, t, opts.sortkeys) end
      local sparse = sparse and #o > maxn -- disable sparsness if only numeric keys (shorter output)
      for n, key in ipairs(o) do
        local value, ktype, plainindex = t[key], type(key), n <= maxn and not sparse
        if opts.valignore and opts.valignore[value] -- skip ignored values; do nothing
        or opts.keyallow and not opts.keyallow[key]
        or opts.keyignore and opts.keyignore[key]
        or opts.valtypeignore and opts.valtypeignore[type(value)] -- skipping ignored value types
        or sparse and value == nil then -- skipping nils; do nothing
        elseif ktype == 'table' or ktype == 'function' or badtype[ktype] then
          if not seen[key] and not globals[key] then
            sref[#sref+1] = 'placeholder'
            local sname = safename(iname, gensym(key)) -- iname is table for local variables
            sref[#sref] = val2str(key,sname,indent,sname,iname,true) end
          sref[#sref+1] = 'placeholder'
          local path = seen[t]..'['..tostring(seen[key] or globals[key] or gensym(key))..']'
          sref[#sref] = path..space..'='..space..tostring(seen[value] or val2str(value,nil,indent,path))
        else
          out[#out+1] = val2str(value,key,indent,nil,seen[t],plainindex,level+1)
          if maxlen then
            maxlen = maxlen - #out[#out]
            if maxlen < 0 then break end
          end
        end
      end
      local prefix = string.rep(indent or '', level)
      local head = indent and '{\n'..prefix..indent or '{'
      local body = table.concat(out, ','..(indent and '\n'..prefix..indent or space))
      local tail = indent and "\n"..prefix..'}' or '}'
      return (custom and custom(tag,head,body,tail,level) or tag..head..body..tail)..comment(t, level)
    elseif badtype[ttype] then
      seen[t] = insref or spath
      return tag..globerr(t, level)
    elseif ttype == 'function' then
      seen[t] = insref or spath
      if opts.nocode then return tag.."function() --[[..skipped..]] end"..comment(t, level) end
      local ok, res = pcall(string.dump, t)
      local func = ok and "((loadstring or load)("..safestr(res)..",'@serialized'))"..comment(t, level)
      return tag..(func or globerr(t, level))
    else return tag..safestr(t) end -- handle all other types
  end
  local sepr = indent and "\n" or ";"..space
  local body = val2str(t, name, indent) -- this call also populates sref
  local tail = #sref>1 and table.concat(sref, sepr)..sepr or ''
  local warn = opts.comment and #sref>1 and space.."--[[incomplete output with shared/self-references skipped]]" or ''
  return not name and body..warn or "do local "..body..sepr..tail.."return "..name..sepr.."end"
end

local function deserialize(data, opts)
  local env = (opts and opts.safe == false) and G
    or setmetatable({}, {
        __index = function(t,k) return t end,
        __call = function(t,...) error("cannot call functions") end
      })
  local f, res = (loadstring or load)('return '..data, nil, nil, env)
  if not f then f, res = (loadstring or load)(data, nil, nil, env) end
  if not f then return f, res end
  if setfenv then setfenv(f, env) end
  return pcall(f)
end

local function merge(a, b) if b then for k,v in pairs(b) do a[k] = v end end; return a; end
return { _NAME = n, _COPYRIGHT = c, _DESCRIPTION = d, _VERSION = v, serialize = s,
  load = deserialize,
  dump = function(a, opts) return s(a, merge({name = '_', compact = true, sparse = true}, opts)) end,
  line = function(a, opts) return s(a, merge({sortkeys = true, comment = true}, opts)) end,
  block = function(a, opts) return s(a, merge({indent = '  ', sortkeys = true, comment = true}, opts)) end }
end)() ---- end of Serpent module


function cz_debug_stringify_results(params, status, ...)
  if not status then return status, ... end -- on error report as it

  params = params or {}
  if params.nocode == nil then params.nocode = true end
  if params.comment == nil then params.comment = 1 end

  local t = {}
  for i = 1, select('#', ...) do -- stringify each of the returned values
    local ok, res = pcall(cz_debug_serpent.line, select(i, ...), params)
    t[i] = ok and res or ("%q"):format(res):gsub("\010","n"):gsub("\026","\\026")
  end
  -- stringify table with all returned values
  -- this is done to allow each returned value to be used (serialized or not)
  -- intependently and to preserve "original" comments
  return pcall(cz_debug_serpent.dump, t, {sparse = false})
end

local function cz_debug_removebasedir(path, basedir)
  return string.gsub(path, '^'.. string.gsub(basedir, '([%(%)%.%%%+%-%*%?%[%^%$%]])','%%%1'), '')
end

function cz_debug_get_stack()
  local function vars(f)
    local func = debug.getinfo(f, "f").func
    local i = 1
    local locals = {}
    -- get locals
    while true do
      local name, value = debug.getlocal(f, i)
      if not name then break end
      if string.sub(name, 1, 1) ~= '(' then
        locals[name] = {value, select(2,pcall(tostring,value))}
      end
      i = i + 1
    end
    -- get varargs (these use negative indices)
    i = 1
    while true do
      local name, value = debug.getlocal(f, -i)
      if not name then break end
      locals[name:gsub("%)$"," "..i..")")] = {value, select(2,pcall(tostring,value))}
      i = i + 1
    end
    -- get upvalues
    i = 1
    local ups = {}
    while func and debug.getupvalue do -- check for func as it may be nil for tail calls
      local name, value = debug.getupvalue(func, i)
      if not name then break end
      if string.sub(name, 1, 4) ~= '_ENV' then
        ups[name] = {value, select(2,pcall(tostring,value))}
      end
      i = i + 1
    end
    return locals, ups
  end

  cz_debug_stack = {}
  for i = 2, 100 do
    local source = debug.getinfo(i, "Snl")
    if not source then break end

    local src = source.source
    --if src:find("@") == 1 then
      src = src:sub(2):gsub("\\", "/")
      if src:find("%./") == 1 then src = src:sub(3) end
    --end

    table.insert(cz_debug_stack, { -- remove basedir from source
      {source.name, cz_debug_removebasedir(src, cz_debug_basedir),
       source.linedefined,
       source.currentline,
       source.what, source.namewhat, source.source},
      vars(i+1)})
  end
end


function cz_debug_stack_send (cz_debug_line)
  local cz_debug_response = "400 Bad Request\n"
  -- extract any optional parameters
  local cz_debug_params = string.match(cz_debug_line, "--%s*(%b{})%s*$")
  
  local cz_debug_pfunc = cz_debug_params and load("return ".. cz_debug_params) -- use internal function
  cz_debug_params = cz_debug_pfunc and cz_debug_pfunc()
  cz_debug_params = (type(cz_debug_params) == "table" and cz_debug_params or {})
  if cz_debug_params.nocode == nil then cz_debug_params.nocode = true end
  if cz_debug_params.sparse == nil then cz_debug_params.sparse = false end
  
  cz_debug_params.maxlevel = cz_debug_params.maxlevel + 1
  
  local cz_debug_status, cz_debug_res = pcall(cz_debug_serpent.dump, cz_debug_stack, cz_debug_params)
  
  cz_debug_response = "200 OK " .. tostring(cz_debug_res) .. "\n"
  
  return cz_debug_response
end

function cz_debug_exec (cz_debug_line)
  local cz_debug_response = "400 Bad Request\n"
  local SAFEWS = "\012" -- "safe" whitespace value
  -- extract any optional parameters
  local cz_debug_params = string.match(cz_debug_line, "--%s*(%b{})%s*$")
  _, _, cz_debug_chunk = string.find(cz_debug_line, "^[A-Z]+%s+(.+)$")
  if cz_debug_chunk then
    cz_debug_chunk = cz_debug_chunk:gsub("\r?".. SAFEWS, "\n") -- convert safe whitespace back to new line
    local cz_debug_pfunc = cz_debug_params and load("return ".. cz_debug_params) -- use internal function
    cz_debug_params = cz_debug_pfunc and cz_debug_pfunc()
    
    cz_debug_pfunc = load(cz_debug_chunk) -- use internal function
    if type(cz_debug_pfunc) == 'function' then
      local cz_debug_status, cz_debug_res = cz_debug_stringify_results(cz_debug_params, true, cz_debug_pfunc())
      cz_debug_response = "200 OK " .. tostring(#cz_debug_res) .. "\n" .. cz_debug_res
    end
  end
  
  return cz_debug_response
end

function cz_debug_command (cz_debug_line)
  local _, _, command = string.find(cz_debug_line, "^([A-Z]+)")
  if command == "SETB" then
    local _, _, _, file, line = string.find(cz_debug_line, "^([A-Z]+)%s+(.-)%s+(%d+)%s*$")
    if file and line and type(cz_debug_breakpoints) == 'table' then
      line = tonumber(line)
      if not cz_debug_breakpoints[line] then cz_debug_breakpoints[line] = {} end
      cz_debug_breakpoints[line][file] = true
    end
  elseif command == "DELB" then
    local _, _, _, file, line = string.find(cz_debug_line, "^([A-Z]+)%s+(.-)%s+(%d+)%s*$")
    if file and line and type(cz_debug_breakpoints) == 'table' then
      line = tonumber(line)
      if file == '*' and line == 0 then cz_debug_breakpoints = {} end
      if cz_debug_breakpoints[line] then cz_debug_breakpoints[line][file] = nil end
    end
  end
end

function cz_debug_hasb (file, line)
  if type(cz_debug_breakpoints) ~= 'table' then return nil end
  return cz_debug_breakpoints[line] and cz_debug_breakpoints[line][file]
end