-- Get first element  in table that match the id key
function get_elem(t, id)
	for _, elem in ipairs(t) do -- sweep elements in table (only numbered keys)
		if elem.id == id then return elem end -- check if match and return
	end
end

-- Get all elements  in table that match the id key. Return an array.
function get_elems(t, id)
	local elems = {} -- array with match elements
	for _, elem in ipairs(t) do  --sweep elements in table (only numbered keys)
		if elem.id == id then  --check if match
			elems[#elems + 1] = elem --append to array
		end
	end
	return elems
end

--Auxiliary function to convert letters reference (as in xlsx columns) in numeric
function letter2num(str)
	local sum = 0
	local idx = #str -- current character index in string
	local n = #str -- string length
	-- get each character in string, in reverse order
	while idx > 0 do
		val = string.byte(str:upper(), idx) - 64 -- subtract 64 to make 'A' -> 1
		-- multiply character value to relative positional value and add to result
		sum = sum + val * math.floor(26^(n-idx)) -- base 26 -> alphabet letters
		idx = idx -1 -- update index in reverse order
	end
	return sum
end

--Auxiliary function to convert numeric reference in letters (as in xlsx columns)
function num2letter(num)
	local t = {}
	
	-- get each 'algarism' by sucessive division
	local div = num
	repeat
		-- base 26 -> alphabet letters
		rem = div % 26
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

-- Get XLSX sheet data. Arguments:
--    - file = xlsx file buffer, to read zipped data
--    - parser = xml parser object
--    - ss = table with shared strings
--    - idx = sheet index
function get_sheet (file, parser, ss, idx)
	-- get sheet index (in wb.sheets[name].id) and open relative file to buffer
	local s_str = file:read(("xl/worksheets/sheet%d.xml"):format(idx))
	local s_xml = parser:read(s_str) -- parse xml file data
	
	-- get dimension and sheetdata from root element
	local dim = get_elem(s_xml, "dimension")
	local data = get_elem(s_xml, "sheetData")
	-- get rows from sheetdata element
	local rows = get_elems(data, "row")
	
	local sheet = {} -- main table
	
	-- get sheet matrix dimension
	if type(dim) == "table" then
		-- parse expression like "A1:C5" to get rows and columns range
		local col_start, row_start, col_end, row_end = dim.attr.ref:match('(%a+)(%d+):(%a+)(%d+)')
		
		-- assemble columns identification array. Columns are identified by letters.
		columns = {}
		for col = letter2num(col_start), letter2num(col_end) do -- use auxiliary functions to convert letters to numbers
			columns[#columns + 1] = num2letter(col) -- and vice-versa
		end
		
		-- assemble rows identification array (numeric identification)
		row_s = {}
		for row = row_start, row_end do
			row_s[#row_s + 1] = row
		end
		
		-- store dimension information, with 'dim' key
		sheet.dim = {rows=row_s, cols=columns}
	end
	
	-- get sheet data -> iterate over rows
	for i, r in ipairs(rows) do
		r_idx = tonumber(r.attr.r) -- current row id (numeric)
		row = {} -- table to store current row data
		cells = get_elems(r, "c") -- look for 'c' elements in row, for cells data
		for j, c in ipairs(cells) do
			c_ref = c.attr.r:match('%a+') -- current column id (letter)
			value = get_elem(c, "v") -- look for 'v' element in cell, for cell's value
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
	
	-- return main table
	return sheet
end

function open_xlsx(path)
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
	ss_t = parser:read(ss) -- convert xml string to Lua table
	ssi = get_elems(ss_t, "si") -- look for 'si' elements
	for i, s in ipairs(ssi) do
		str = get_elem(s, "t") -- in each element, look for 't' id
		if type(str) == "table" then 
			shar_str[i] = str.cont -- store content in workbook
		end
	end
	
	-- get sheets information
	wb_t = parser:read(wb) -- convert xml string to Lua table
	sheets = get_elem(wb_t, "sheets") -- look for 'sheets' section
	sheets_t = get_elems(sheets, "sheet") -- look for 'sheet' elements in section
	for _, sheet in ipairs(sheets_t) do
		-- in each element, look for sheet name and id (index) in its attributes
		if type(sheet.attr) == "table" then
			-- store in workbook
			workbook.sheets[sheet.attr.name] = {} -- sheet's name as key
			local idx = sheet.attr["r:id"]
			idx = tonumber(idx:match('rId(%d+)')) -- get id index, by matching string
			workbook.sheets[sheet.attr.name].idx = idx
			-- get sheet data
			workbook.sheets[sheet.attr.name].data = get_sheet (zip, parser, shar_str, idx)
		end
	end
	
	zip:close()
	parser:close()
	
	return workbook

end

path = "demo.xlsx"

workbook = open_xlsx(path)

for key, sheet in pairs(workbook.sheets) do
	cadzinho.db_print (("Sheet: %s, index = %s"):format(key, sheet.idx))
	
	-- sparse scan
	cadzinho.db_print ("------Sparse data:-------")
	for r_i, row in ipairs(sheet.data) do
		if type(row) == "table" then
			for c_i, cell in pairs (row) do
				cadzinho.db_print (("%s%d"):format(c_i, r_i), cell)
			end
		end
	end
	
	-- tabular scan
	cadzinho.db_print ("------ Tabular data:-------")
	for _, r in ipairs(sheet.data.dim.rows) do
		str = ""
		for _, c in ipairs(sheet.data.dim.cols) do
			str = str .. tostring(sheet.data[r][c]) .. "    "
		end
		cadzinho.db_print (str)
	end
	
end