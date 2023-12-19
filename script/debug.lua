local serpent = (function() ---- include Serpent module for serialization
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



local function stringify_results(params, status, ...)
  if not status then return status, ... end -- on error report as it

  params = params or {}
  if params.nocode == nil then params.nocode = true end
  if params.comment == nil then params.comment = 1 end

  local t = {}
  for i = 1, select('#', ...) do -- stringify each of the returned values
    local ok, res = pcall(serpent.line, select(i, ...), params)
    t[i] = ok and res or ("%q"):format(res):gsub("\010","n"):gsub("\026","\\026")
  end
  -- stringify table with all returned values
  -- this is done to allow each returned value to be used (serialized or not)
  -- intependently and to preserve "original" comments
  return pcall(serpent.dump, t, {sparse = false})
end

--print("DEBUG thread running")
--server:send("204 Output " .. stream .. " " .. tostring(#file) .. "\n" .. file)


line = nil
response = nil
chunk = nil
--local events = { BREAK = 1, WATCH = 2, RESTART = 3, STACK = 4 }
local SAFEWS = "\012" -- "safe" whitespace value

if received then
  --print(#received)
  
  if buffer then 
    buffer = buffer .. received
  else
    buffer = received
  end
  
  if wait_recv then
    if wait_recv <= string.len(buffer) then
      chunk = string.sub (buffer, 1, wait_recv)
      buffer = string.sub (buffer, wait_recv+1)
      wait_recv = nil
      response = "200 OK 0\n"
    end
  else
    s, e = string.find (buffer, "\n")
    if s then
      line = string.sub (buffer, 1, s-1)
      buffer = string.sub (buffer, e+1)
    end
  end
end

--for i, line in ipairs (lines) do
if line then
  print(line)
  
  status = 0
  _, _, command = string.find(line, "^([A-Z]+)")
  if command == "SETB" then
    local _, _, _, file, line = string.find(line, "^([A-Z]+)%s+(.-)%s+(%d+)%s*$")
    if file and line then
      --[[set_breakpoint(file, tonumber(line))]]
      status = 1
      response = "200 OK\n"
    else
      response = "400 Bad Request\n"
    end
  elseif command == "DELB" then
    local _, _, _, file, line = string.find(line, "^([A-Z]+)%s+(.-)%s+(%d+)%s*$")
    if file and line then
      --[[remove_breakpoint(file, tonumber(line))]]
      status = 2
      response = "200 OK\n"
    else
      response = "400 Bad Request\n"
    end
  elseif command == "EXEC" then
    -- extract any optional parameters
    local params = string.match(line, "--%s*(%b{})%s*$")
    _, _, chunk = string.find(line, "^[A-Z]+%s+(.+)$")
    if chunk then
      chunk = chunk:gsub("\r?".. SAFEWS, "\n") -- convert safe whitespace back to new line
      --print (chunk)
      
      
      local pfunc = params and load("return ".. params) -- use internal function
      params = pfunc and pfunc()
      --print (params)
      
    --[[
      -- \r is optional, as it may be stripped by some luasocket versions, like the one in LOVE2d
      chunk = chunk:gsub("\r?"..SAFEWS, "\n") -- convert safe whitespace back to new line
      local func, res = mobdebug.loadstring(chunk)
      local status
      if func then
        local pfunc = params and loadstring("return "..params) -- use internal function
        params = pfunc and pfunc()
        params = (type(params) == "table" and params or {})
        local stack = tonumber(params.stack)
        -- if the requested stack frame is not the current one, then use a new capture
        -- with a specific stack frame: `capture_vars(0, coro_debugee)`
        local env = stack and coro_debugee and capture_vars(stack-1, coro_debugee) or eval_env
        setfenv(func, env)
        status, res = stringify_results(params, pcall(func, unpack(rawget(env,'...') or {})))
      end
      if status then
        if mobdebug.onscratch then mobdebug.onscratch(res) end
        response = "200 OK " .. tostring(#res) .. "\n" .. res
      else
        -- fix error if not set (for example, when loadstring is not present)
        if not res then res = "Unknown error" end
        response = "401 Error in Expression " .. tostring(#res) .. "\n" .. res
      end
      ]]
      status = 3
      local status, res = stringify_results(params, true, "testing output")
      --print (res)
      response = "200 OK " .. tostring(#res) .. "\n" .. res
    else
      response = "400 Bad Request\n"
    end
  elseif command == "LOAD" then
    local _, _, size, name = string.find(line, "^[A-Z]+%s+(%d+)%s+(%S.-)%s*$")
    size = tonumber(size)
    
    if size and name then
      response = nil
    else
      response = "400 Bad Request\n"
    end
    
    status = 4
    
    if size <= string.len(buffer) then
      chunk = string.sub (buffer, 1, size)
      buffer = string.sub (buffer, size+1)
      wait_recv = nil
      response = "200 OK 0\n"
    else
      wait_recv = size
    end
    
    --[[

    if abort == nil then -- no LOAD/RELOAD allowed inside start()
      --if size > 0 then server:receive(size) end
      if sfile and sline then
        response = "201 Started " .. sfile .. " " .. tostring(sline) .. "\n"
      else
        response = "200 OK 0\n"
      end
    else
      -- reset environment to allow required modules to load again
      -- remove those packages that weren't loaded when debugger started
      for k in pairs(package.loaded) do
        if not loaded[k] then package.loaded[k] = nil end
      end

      if size == 0 and name == '-' then -- RELOAD the current script being debugged
        response = "200 OK 0\n"
        --coroyield("load")
      else
        -- receiving 0 bytes blocks (at least in luasocket 2.0.2), so skip reading
        --local chunk = size == 0 and "" or server:receive(size)
        if chunk then -- LOAD a new script for debugging
          --local func, res = mobdebug.loadstring(chunk, "@"..name)
          if func then
            response = "200 OK 0\n"
            debugee = func
            --coroyield("load")
          else
            response = "401 Error in Expression " .. tostring(#res) .. "\n" .. res
          end
        else
          response = "400 Bad Request\n"
        end
      end
    end ]]--
  elseif command == "SETW" then
    local _, _, exp = string.find(line, "^[A-Z]+%s+(.+)%s*$")
    if exp then
      --local func, res = mobdebug.loadstring("return(" .. exp .. ")")
      if func then
        --watchescnt = watchescnt + 1
        --local newidx = #watches + 1
        --watches[newidx] = func
        status = 5
        response = "200 OK " .. tostring(newidx) .. "\n"
      else
        response = "401 Error in Expression " .. tostring(#res) .. "\n" .. res
      end
    else
      response = "400 Bad Request\n"
    end
  elseif command == "DELW" then
    local _, _, index = string.find(line, "^[A-Z]+%s+(%d+)%s*$")
    index = tonumber(index)
    if index > 0 and index <= #watches then
      --watchescnt = watchescnt - (watches[index] ~= emptyWatch and 1 or 0)
      --watches[index] = emptyWatch
      status = 6
      response = "200 OK\n"
    else
      response = "400 Bad Request\n"
    end
  elseif command == "RUN" then
    status = 7
    response = "200 OK\n"
    
    --[[
    local tbl = {123, "testando", nil, {1, 2, 3}}
    for n = 1, #tbl do
      --print (tbl[n])
      tbl[n] = select(2, pcall(serpent.line, tbl[n], {nocode = true, comment = false}))
      --print (tbl[n])
    end
    local file = table.concat(tbl, "\t").."\n"
    --print (file)
    response = "200 OK\n204 Output stdout " .. tostring(#file) .. "\n" .. file
    ]]--
    
    --[[
    local ev, vars, file, line, idx_watch = coroyield()
    eval_env = vars
    if ev == events.BREAK then
      response = "202 Paused " .. file .. " " .. tostring(line) .. "\n"
    elseif ev == events.WATCH then
      response = "203 Paused " .. file .. " " .. tostring(line) .. " " .. tostring(idx_watch) .. "\n"
    elseif ev == events.RESTART then
      -- nothing to do
    else
      response = "401 Error in Execution " .. tostring(#file) .. "\n" .. file
    end
    ]]--
    --server:send("204 Output " .. stream .. " " .. tostring(#file) .. "\n" .. file)
  elseif command == "STEP" then
    status = 8
    response = "200 OK\n"
    step_into = true

    local ev, vars, file, line, idx_watch = coroyield()
    eval_env = vars
    if ev == events.BREAK then
      response = "202 Paused " .. file .. " " .. tostring(line) .. "\n"
    elseif ev == events.WATCH then
      response = "203 Paused " .. file .. " " .. tostring(line) .. " " .. tostring(idx_watch) .. "\n"
    elseif ev == events.RESTART then
      -- nothing to do
    else
      response = "401 Error in Execution " .. tostring(#file) .. "\n" .. file
    end
  elseif command == "OVER" or command == "OUT" then
    status = 9
    response = "200 OK\n"
    step_over = true

    -- OVER and OUT are very similar except for
    -- the stack level value at which to stop
    if command == "OUT" then step_level = stack_level - 1
    else step_level = stack_level end

    local ev, vars, file, line, idx_watch = coroyield()
    eval_env = vars
    if ev == events.BREAK then
      response = "202 Paused " .. file .. " " .. tostring(line) .. "\n"
    elseif ev == events.WATCH then
      response = "203 Paused " .. file .. " " .. tostring(line) .. " " .. tostring(idx_watch) .. "\n"
    elseif ev == events.RESTART then
      -- nothing to do
    else
      response = "401 Error in Execution " .. tostring(#file) .. "\n".. file
    end
  elseif command == "BASEDIR" then
    local _, _, dir = string.find(line, "^[A-Z]+%s+(.+)%s*$")
    if dir then
      basedir = iscasepreserving and string.lower(dir) or dir
      -- reset cached source as it may change with basedir
      lastsource = nil
      status = 10
      response = "200 OK\n"
    else
      response = "400 Bad Request\n"
    end
  elseif command == "SUSPEND" then
    -- do nothing; it already fulfilled its role
    status = 11
  elseif command == "DONE" then
    status = 12
    --coroyield("done")
    --return -- done with all the debugging
  elseif command == "STACK" then
    -- first check if we can execute the stack command
    -- as it requires yielding back to debug_hook it cannot be executed
    -- if we have not seen the hook yet as happens after start().
    -- in this case we simply return an empty result
    local vars, ev = {}
    if seen_hook then
      ev, vars = coroyield("stack")
    end
    if ev and ev ~= events.STACK then
      response = "401 Error in Execution " .. tostring(#vars) .. "\n" .. vars
    else
      local params = string.match(line, "--%s*(%b{})%s*$")
      local pfunc = params and loadstring("return "..params) -- use internal function
      params = pfunc and pfunc()
      params = (type(params) == "table" and params or {})
      if params.nocode == nil then params.nocode = true end
      if params.sparse == nil then params.sparse = false end
      -- take into account additional levels for the stack frames and data management
      if tonumber(params.maxlevel) then params.maxlevel = tonumber(params.maxlevel)+4 end

      local ok, res = pcall(mobdebug.dump, vars, params)
      if ok then
        status = 13
        response = "200 OK " .. tostring(res) .. "\n"
      else
        response = "401 Error in Execution " .. tostring(#res) .. "\n" ..  res
      end
    end
  elseif command == "OUTPUT" then
    local _, _, stream, mode = string.find(line, "^[A-Z]+%s+(%w+)%s+([dcr])%s*$")
    if stream and mode and stream == "stdout" then
      --[[
      -- assign "print" in the global environment
      local default = mode == 'd'
      genv.print = default and iobase.print or corowrap(function()
        -- wrapping into coroutine.wrap protects this function from
        -- being stepped through in the debugger.
        -- don't use vararg (...) as it adds a reference for its values,
        -- which may affect how they are garbage collected
        while true do
          local tbl = {coroutine.yield()}
          if mode == 'c' then iobase.print(unpack(tbl)) end
          for n = 1, #tbl do
            tbl[n] = select(2, pcall(mobdebug.line, tbl[n], {nocode = true, comment = false})) end
          local file = table.concat(tbl, "\t").."\n"
          response = "204 Output " .. stream .. " " .. tostring(#file) .. "\n" .. file
        end
      end)
      if not default then genv.print() end -- "fake" print to start printing loop
      ]]--
      status = 14
      response = "200 OK\n"
    else
      response = "400 Bad Request\n"
    end
  elseif command == "EXIT" then
    status = 15
    response = "200 OK\n"
    --coroyield("exit")
  else
    response = "400 Bad Request\n"
  end
end