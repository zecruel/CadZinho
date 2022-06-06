file = {value = ''}
fmt = {value = 1, "PDF", "SVG", "PS", "PNG", "JPG", "BMP"}
unit = {value = 1, "MM", "IN", "PX"}
page_w = {value = 297}
page_h = {value = 210}
single = {value = false}
single_path = {value = "output.pdf"}
list = {}

function GetFileName(url)
  return url:match("^.+\\(.+)$")
end

function GetFileExtension(url)
  return url:match("^.+(%..+)$")
end


function do_print()
	local single_out = fmt.value == 1 and single.value
	local pdf = nil
	
	if single_out then -- single output
		pdf = cadzinho.pdf_new(page_w.value, page_h.value, unit[unit.value])
	end

	for f, sel in pairs(list) do
		-- open drawing
		cadzinho.open_drwg(f)
		if single_out then -- single output
			pdf:page() -- add a page
		else
			-- replace file extension to choosen output format
			out = string.gsub(f, "(%..+)$", "."..string.lower(fmt[fmt.value]))
			-- print file
			cadzinho.print_drwg(out, page_w.value, page_h.value, unit[unit.value])
		end
		
	end
	
	if single_out then -- single output
		pdf:save(single_path.value)
		pdf:close()
	end
end

function bprint_win()
	cadzinho.nk_layout(110, 2)
	
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
	
	cadzinho.nk_layout(20, 1)
	cadzinho.nk_label("Files:")
	if cadzinho.nk_edit(file)then
		local ext = string.upper(GetFileExtension(file.value))
		if ext == ".DXF" then
			list[file.value] = true
		end
	end
	
	cadzinho.nk_layout(110, 1)
	if cadzinho.nk_group_begin("Selected", false, true) then
		cadzinho.nk_layout(20, 1)
		for f, sel in pairs(list) do
			cadzinho.nk_button(GetFileName(f))
		end
		cadzinho.nk_group_end()
	end
	
	
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
		co = coroutine.create(do_print)
		coroutine.resume(co)
	end
end

cadzinho.win_show("bprint_win", "Batch Print", 500,100,400,400)