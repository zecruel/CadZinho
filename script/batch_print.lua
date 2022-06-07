file = {value = ''}
fmt = {value = 1, "PDF", "SVG", "PS", "PNG", "JPG", "BMP"}
unit = {value = 1, "MM", "IN", "PX"}
page_w = {value = 297}
page_h = {value = 210}
single = {value = false}
single_path = {value = "output.pdf"}
list = {}
progress = 0

function GetFileName(url)
  return url:match("^.+\\(.+)$")
end

function GetFileExtension(url)
  return url:match("^.+(%..+)$")
end

-- routine to print list of drawings
function do_print()
	local single_out = fmt.value == 1 and single.value
	local pdf = nil
	
	if single_out then
		-- single output - only for pdf
		-- init pdf object
		pdf = cadzinho.pdf_new(page_w.value, page_h.value, unit[unit.value])
	end
	
	-- iterate over list
	for i=1, #list do
		-- open drawing
		cadzinho.open_drwg(list[i].path)
		if single_out then -- single output
			pdf:page() -- add a page
		else
			-- replace file extension to choosen output format
			out = string.gsub(list[i].path, "(%..+)$", "."..string.lower(fmt[fmt.value]))
			-- print file
			cadzinho.print_drwg(out, page_w.value, page_h.value, unit[unit.value])
		end
		progress = progress + 1 --update progress bar value
	end
	
	if single_out then
		-- single output - only for pdf
		-- save and close pdf object
		pdf:save(single_path.value)
		pdf:close()
	end
end

-- batch print GUI
function bprint_win()
	cadzinho.nk_layout(110, 2)
	
	-- output format and size
	if cadzinho.nk_group_begin("Format", true, true) then
		cadzinho.nk_layout(15, 2)
		cadzinho.nk_option(fmt)
		cadzinho.nk_group_end()
	end
	if cadzinho.nk_group_begin("Size", true, true) then
		cadzinho.nk_layout(20, 1)
		cadzinho.nk_propertyd("Width", page_w)
		cadzinho.nk_propertyd("Height", page_h)
		cadzinho.nk_layout(20, 3)
		cadzinho.nk_option(unit)
		cadzinho.nk_group_end()
	end
	
	-- add drawings to list
	cadzinho.nk_layout(20, 1)
	cadzinho.nk_label("Files:")
	if cadzinho.nk_edit(file)then
		local ext = string.upper(GetFileExtension(file.value))
		if ext == ".DXF" then
			local item = {path = file.value, value = false}
			list[#list+1] = item
		end
	end
	
	-- drawings list
	cadzinho.nk_layout(110, 1)
	if cadzinho.nk_group_begin("Selected", false, true, true) then
		cadzinho.nk_layout(20, 1)
		for i=1, #list do			
			cadzinho.nk_selectable(GetFileName(list[i].path), list[i])
		end
		cadzinho.nk_group_end()
	end
	
	-- option to output in a single file - only for PDF
	if fmt.value == 1 then -- PDF output
		cadzinho.nk_layout(20, 2)
		cadzinho.nk_check("Single", single)
		if single.value then
			cadzinho.nk_label("Out path:")
			cadzinho.nk_layout(20, 1)
			cadzinho.nk_edit(single_path)
		end
	end
	
	cadzinho.nk_layout(20, 1)
	if cadzinho.nk_button("Generate") then
		-- do the job
		progress = 0
		
		-- execute in  thread, to avoid interface blocking
		co = coroutine.create(do_print)
		coroutine.resume(co)
	end
	
	--show progress bar
	cadzinho.nk_progress(progress, #list)
end

cadzinho.win_show("bprint_win", "Batch Print", 500,100,400,430)