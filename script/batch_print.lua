file = {value = ''}
fmt = {value = 1, "PDF", "SVG", "PS", "PNG", "JPG", "BMP"}
unit = {value = 1, "MM", "IN", "PX"}
page_w = {value = 297}
page_h = {value = 210}
single = {value = false}
list = {}

function GetFileName(url)
  return url:match("^.+\\(.+)$")
end

function GetFileExtension(url)
  return url:match("^.+(%..+)$")
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
	
	cadzinho.nk_layout(20, 1)
	cadzinho.nk_check("Single", single)
	if cadzinho.nk_button("Generate") then
		
	end
end

cadzinho.win_show("bprint_win", "Batch Print", 200,200,400,400)