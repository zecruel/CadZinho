

-- Get first element  in table that match the id key
local function get_elem(t, id)
	for _, elem in ipairs(t) do -- sweep elements in table (only numbered keys)
		if elem.id == id then return elem end -- check if match and return
	end
end

-- Get all elements  in table that match the id key. Return an array.
local function get_elems(t, id)
	local elems = {} -- array with match elements
	for _, elem in ipairs(t) do  --sweep elements in table (only numbered keys)
		if elem.id == id then  --check if match
			elems[#elems + 1] = elem --append to array
		end
	end
	return elems
end

--Auxiliary function to convert letters reference (as in xlsx columns) in numeric
local function letter2num(str)
	local sum = 0
	local idx = #str -- current character index in string
	local n = #str -- string length
	-- get each character in string, in reverse order
	while idx > 0 do
		local val = string.byte(str:upper(), idx) - 64 -- subtract 64 to make 'A' -> 1
		-- multiply character value to relative positional value and add to result
		sum = sum + val * math.floor(26^(n-idx)) -- base 26 -> alphabet letters
		idx = idx -1 -- update index in reverse order
	end
	return sum
end

--Auxiliary function to convert numeric reference in letters (as in xlsx columns)
local function num2letter(num)
	local t = {}
	
	-- get each 'algarism' by sucessive division
	local div = num
	repeat
		-- base 26 -> alphabet letters
		local rem = div % 26
		div = div // 26
		-- store each 'algarism' in a array
		t[#t + 1] = rem -- 'algarism' is remainder
	until div <= 0
	
	local str = "" -- string to return
	
	-- convert array of 'algarisms' in a string (reverse order)
	local idx = #t
	while idx > 0 do
		str = str .. string.char(t[idx] + 64) -- add 64 to make 1 -> 'A'
		idx = idx -1 -- update index in reverse order
	end
	
	return str
end

--Auxiliary function to convert range representation (like "A1:C4") in numeric form
local function range2num(str)
	-- parse expression like "A1:C5" to get rows and columns range
	local c_start, r_start, c_end, r_end = str:match('(%a+)(%d+):(%a+)(%d+)')
	
	-- Convert to numbers. Columns are identified by letters.
	local col_start = letter2num(c_start)
	local col_end = letter2num(c_end)
	local row_start = tonumber(r_start)
	local row_end = tonumber(r_end)
	
	return row_start, row_end, col_start, col_end
end

-- Get XLSX sheet data. Arguments:
--    - file = xlsx file buffer, to read zipped data
--    - parser = xml parser object
--    - ss = table with shared strings
--    - idx = sheet index
local function get_sheet (file, parser, ss, idx)
	-- get sheet index (in wb.sheets[name].id) and open relative file to buffer
	local s_str = file:read(("xl/worksheets/sheet%d.xml"):format(idx))
	local s_xml = parser:read(s_str) -- parse xml file data
	
	-- get dimension and sheetdata from root element
	local dim = get_elem(s_xml, "dimension")
	local data = get_elem(s_xml, "sheetData")
	-- get rows from sheetdata element
	local rows = get_elems(data, "row")
	
	--get merged cells information
	local mc_t = get_elem(s_xml, "mergeCells")
	local merged = {}
	if type (mc_t) == 'table' then
		local mc_els = get_elems(mc_t, "mergeCell")
		for _, elem in ipairs(mc_els) do  --sweep elements in table (only numbered keys)
			merged[#merged + 1] = elem.attr.ref
		end
	end
	
	local s_dim = {rows={}, cols={}} -- table with sheet dimmensions
	local sheet = {} -- table with sheet data
	
	-- get sheet matrix dimension
	if type(dim) == "table" then
		-- parse expression like "A1:C5" to get rows and columns range
		local col_start, row_start, col_end, row_end = dim.attr.ref:match('(%a+)(%d+):(%a+)(%d+)')
		
		-- assemble columns identification array. Columns are identified by letters.
		local columns = {}
		for col = letter2num(col_start), letter2num(col_end) do -- use auxiliary functions to convert letters to numbers
			columns[#columns + 1] = num2letter(col) -- and vice-versa
		end
		
		-- assemble rows identification array (numeric identification)
		local row_s = {}
		for row = tonumber(row_start), tonumber(row_end) do
			row_s[#row_s + 1] = row
		end
		
		-- store dimension information
		s_dim.rows = row_s
		s_dim.cols = columns
	end
	
	-- get sheet data -> iterate over rows
	for i, r in ipairs(rows) do
		local r_idx = tonumber(r.attr.r) -- current row id (numeric)
		local row = {} -- table to store current row data
		local cells = get_elems(r, "c") -- look for 'c' elements in row, for cells data
		for j, c in ipairs(cells) do
			local c_ref = c.attr.r:match('%a+') -- current column id (letter)
			local value = get_elem(c, "v") -- look for 'v' element in cell, for cell's value
			if type(value) == "table" then 
				if c.attr.t == "s" then -- verify if value is a shared string
					-- value content is the index to look shared string in workbook
					row[c_ref] = ss[tonumber(value.cont)+1]
				else
					-- regular value in content
					row[c_ref] = value.cont
				end
			end
		end
		-- store row in main table
		sheet[r_idx] = row
	end
	
	--sheet.merged = merged
	
	-- return main table
	return {idx = idx, dim = s_dim, data = sheet, merged = merged}
end

local function open_xlsx(path)
	-- try to open file (xlsx is a ziped archive)
	local zip = miniz.open(path)
	if zip == nil then return nil end --fail to open file

	-- read main xml structures to strings
	local wb = zip:read("xl/workbook.xml")
	local ss = zip:read("xl/sharedStrings.xml")
	local sty = zip:read("xl/styles.xml")
	
	local parser = yxml.new()
	
	-- main workbook object
	local workbook = {}
	workbook.sheets = {}
	
	local shar_str = {}
	
	-- get shared strings
	local ss_t = parser:read(ss) -- convert xml string to Lua table
	local ssi = get_elems(ss_t, "si") -- look for 'si' elements
	for i, s in ipairs(ssi) do
		local str = get_elem(s, "t") -- in each element, look for 't' id
		if type(str) == "table" then 
			shar_str[i] = str.cont -- store content in workbook
		end
	end
	
	-- get sheets information
	local wb_t = parser:read(wb) -- convert xml string to Lua table
	local sheets = get_elem(wb_t, "sheets") -- look for 'sheets' section
	local sheets_t = get_elems(sheets, "sheet") -- look for 'sheet' elements in section
	for _, sheet in ipairs(sheets_t) do
		-- in each element, look for sheet name and id (index) in its attributes
		if type(sheet.attr) == "table" then
			-- store in workbook
			workbook.sheets[sheet.attr.name] = {} -- sheet's name as key
			local idx = sheet.attr["r:id"]
			idx = tonumber(idx:match('rId(%d+)')) -- get id index, by matching string
			-- get sheet data
			workbook.sheets[sheet.attr.name] = get_sheet (zip, parser, shar_str, idx)
		end
	end
	
	zip:close()
	parser:close()
	
	return workbook

end

function expand_merge (sheet)
	for _, merge in ipairs(sheet.merged) do
		-- parse expression like "A1:C5" to get rows and columns range
		local col_start, row_start, col_end, row_end = merge:match('(%a+)(%d+):(%a+)(%d+)')
		-- first cell in range ha the value to copy to entire merged region 
		local value = sheet.data[tonumber(row_start)][col_start]
		for row = tonumber(row_start), tonumber(row_end) do
			for col = letter2num(col_start), letter2num(col_end) do -- use auxiliary functions to convert letters to numbers
				sheet.data[row][num2letter(col)] = value
				--cadzinho.db_print (("%s%d"):format(row, col), value)
			end
		end
		
		
	end
end

function get_merge (sheet)
  local merged = {}
	for id, merge in ipairs(sheet.merged) do
		-- parse expression like "A1:C5" to get rows and columns range
		local col_start, row_start, col_end, row_end = merge:match('(%a+)(%d+):(%a+)(%d+)')
		-- first cell in range ha the value to copy to entire merged region 
		local value = sheet.data[tonumber(row_start)][col_start]
		for row = tonumber(row_start), tonumber(row_end) do
      merged[row] = merged[row] or {}
			for col = letter2num(col_start), letter2num(col_end) do -- use auxiliary functions to convert letters to numbers
				merged[row][num2letter(col)] = {id=id, value=value} 
				--cadzinho.db_print (("%s%d"):format(row, col), value)
			end
		end
		
		
	end
  return merged
end

return {open = open_xlsx, n2l = num2letter, l2n = letter2num, range = range2num, expand_merge = expand_merge, get_merge = get_merge}