count = 0
radius = {value = '4'}

function round_dyn(event)
	if count == 0 then
		if event.type == 'enter' then
			count = count + 1
			x0 = event.x
			y0 = event.y
			
			cadzinho.db_print('next corner')
		elseif event.type == 'cancel' then
			cadzinho.stop_dynamic()
		end
	else
		x1 = event.x
		y1 = event.y
		if event.type == 'motion' then
			r = tonumber(radius.value)
			
			if math.abs(x1-x0) < 2*r or math.abs(y1-y0) < 2*r then
				pline = cadzinho.new_pline(x0, y0, 0, x1, y0, 0)
				cadzinho.pline_append(pline, x1,y1,0)
				cadzinho.pline_append(pline, x0,y1,0)
				cadzinho.pline_close(pline, true)
				cadzinho.ent_draw(pline)
			else
				if x0 > x1 then
					xx1 = x0
					xx0 = x1
				else
					xx0 = x0
					xx1 = x1
				end
				
				if y0 > y1 then
					yy1 = y0
					yy0 = y1
				else
					yy0 = y0
					yy1 = y1
				end
				
				pline = cadzinho.new_pline(xx0, yy0+r, 0, xx0+r, yy0, 0.414214)
				cadzinho.pline_append(pline, xx1-r,yy0,0)
				cadzinho.pline_append(pline, xx1,yy0+r, 0.414214)
				cadzinho.pline_append(pline, xx1,yy1-r,0)
				cadzinho.pline_append(pline, xx1-r,yy1,0.414214)
				cadzinho.pline_append(pline, xx0+r,yy1,0)
				cadzinho.pline_append(pline, xx0,yy1-r,0.414214)
				cadzinho.pline_close(pline, true)
				cadzinho.ent_draw(pline)
			end
			
		elseif event.type == 'enter' then
			r = tonumber(radius.value)
			if math.abs(x1-x0) < 2*r or math.abs(y1-y0) < 2*r then
				pline = cadzinho.new_pline(x0, y0, 0, x1, y0, 0)
				cadzinho.pline_append(pline, x1,y1,0)
				cadzinho.pline_append(pline, x0,y1,0)
				cadzinho.pline_close(pline, true)
				cadzinho.ent_append(pline)
			else
				if x0 > x1 then
					xx1 = x0
					xx0 = x1
				else
					xx0 = x0
					xx1 = x1
				end
				
				if y0 > y1 then
					yy1 = y0
					yy0 = y1
				else
					yy0 = y0
					yy1 = y1
				end
				
				pline = cadzinho.new_pline(xx0, yy0+r, 0, xx0+r, yy0, 0.414214)
				cadzinho.pline_append(pline, xx1-r,yy0,0)
				cadzinho.pline_append(pline, xx1,yy0+r, 0.414214)
				cadzinho.pline_append(pline, xx1,yy1-r,0)
				cadzinho.pline_append(pline, xx1-r,yy1,0.414214)
				cadzinho.pline_append(pline, xx0+r,yy1,0)
				cadzinho.pline_append(pline, xx0,yy1-r,0.414214)
				cadzinho.pline_close(pline, true)
				cadzinho.ent_append(pline)
			end
			
			
		
			count = 0
			cadzinho.db_print('first corner')
		elseif event.type == 'cancel' then
			count = 0
			cadzinho.db_print('first corner')
		end
	end
end

function round_win()
	cadzinho.nk_layout(20, 1)
	
	cadzinho.nk_label("Radius:")
	cadzinho.nk_edit(radius)
	
end

cadzinho.win_show("round_win", "Rounded rectangle", 200,200,200,200)

cadzinho.start_dynamic("round_dyn")