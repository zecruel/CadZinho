cx = {value = '0'}
cy = {value = '0'}
r = {value = '1'}
n = {value = '4'}

function polyg_win()
	cadzinho.nk_layout(20, 2)
	cadzinho.nk_label("Center x:")
	cadzinho.nk_edit(cx)
	cadzinho.nk_label("Center y:")
	cadzinho.nk_edit(cy)
	cadzinho.nk_label("Radius:")
	cadzinho.nk_edit(r)
	cadzinho.nk_label("Num:")
	cadzinho.nk_edit(n)
	cadzinho.nk_layout(20, 1)
	if cadzinho.nk_button("Generate") then
		assert(loadfile("reg_poly.lua"))(tonumber(cx.value), tonumber(cy.value), tonumber(r.value), tonumber(n.value))
	end
end

cadzinho.win_show("polyg_win", "script test", 200,200,200,200)