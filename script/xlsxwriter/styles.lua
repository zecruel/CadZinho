----
-- Styles - A class for writing the Excel XLSX Styles file.
--
-- Copyright 2014-2015, John McNamara, jmcnamara@cpan.org
--
require "xlsxwriter.strict"

local Xmlwriter = require "xlsxwriter.xmlwriter"

------------------------------------------------------------------------------
--
-- Constructor.
--
------------------------------------------------------------------------------

-- The constructor inherits from xmlwriter.lua.
local Styles = {}
setmetatable(Styles,{__index = Xmlwriter})

function Styles:new()
  local instance = {
    font_count       = 0,
    num_format_count = 0,
    border_count     = 0,
    fill_count       = 0,
    palette          = {},
    custom_colors    = {},
    xf_formats       = {},
    dxf_formats      = {},
  }

  setmetatable(instance, self)
  self.__index = self
  return instance
end


------------------------------------------------------------------------------
--
-- Public methods.
--
------------------------------------------------------------------------------

----
-- Assemble and write the XML file.
--
function Styles:_assemble_xml_file()

  self:_xml_declaration()

  -- Add the style sheet.
  self:_write_style_sheet()

  -- Write the number formats.
  self:_write_num_fmts()

  -- Write the fonts.
  self:_write_fonts()

  -- Write the fills.
  self:_write_fills()

  -- Write the borders element.
  self:_write_borders()

  -- Write the cellStyleXfs element.
  self:_write_cell_style_xfs()

  -- Write the cellXfs element.
  self:_write_cell_xfs()

  -- Write the cellStyles element.
  self:_write_cell_styles()

  -- Write the dxfs element.
  self:_write_dxfs()

  -- Write the tableStyles element.
  self:_write_table_styles()

  -- Write the colors element.
  self:_write_colors()

  -- Close the style sheet tag.
  self:_xml_end_tag("styleSheet")

  -- Close the XML writer filehandle.
  self:_xml_close()

end

----
-- Pass in the Format objects and other properties used to set the styles.
--
function Styles:_set_style_properties(properties)
  self.xf_formats       = properties["xf_formats"]
  self.font_count       = properties["font_count"]
  self.num_format_count = properties["num_format_count"]
  self.border_count     = properties["border_count"]
  self.fill_count       = properties["fill_count"]
  self.custom_colors    = properties["custom_colors"]
  self.dxf_formats      = properties["dxf_formats"]
end

------------------------------------------------------------------------------
--
-- Internal methods.
--
------------------------------------------------------------------------------

------------------------------------------------------------------------------
--
-- XML writing methods.
--
------------------------------------------------------------------------------

----
-- Write the <styleSheet> element.
--
function Styles:_write_style_sheet()
  local xmlns = "http://schemas.openxmlformats.org/spreadsheetml/2006/main"
  local attributes = {{["xmlns"] = xmlns}}
  self:_xml_start_tag("styleSheet", attributes)
end

----
-- Write the <numFmts> element.
--
function Styles:_write_num_fmts()
  local count = self.num_format_count
  if count == 0 then return end

  local attributes = {{["count"] = count}}

  self:_xml_start_tag("numFmts", attributes)

  -- Write the numFmts elements.
  for _, format in ipairs(self.xf_formats) do
    -- Ignore built-in number formats, i.e., < 164.
    if format.num_format_index >= 164 then
      self:_write_num_fmt(format.num_format_index, format.num_format)
    end
  end

  self:_xml_end_tag("numFmts")
end

----
-- Write the <numFmt> element.
--
function Styles:_write_num_fmt(num_fmt_id, format_code)

  local format_codes = {
    [0]  = 'General',
    [1]  = '0',
    [2]  = '0.00',
    [3]  = '#,##0',
    [4]  = '#,##0.00',
    [5]  = '($#,##0_);($#,##0)',
    [6]  = '($#,##0_);[Red]($#,##0)',
    [7]  = '($#,##0.00_);($#,##0.00)',
    [8]  = '($#,##0.00_);[Red]($#,##0.00)',
    [9]  = '0%',
    [10] = '0.00%',
    [11] = '0.00E+00',
    [12] = '# ?/?',
    [13] = '# ??/??',
    [14] = 'm/d/yy',
    [15] = 'd-mmm-yy',
    [16] = 'd-mmm',
    [17] = 'mmm-yy',
    [18] = 'h:mm AM/PM',
    [19] = 'h:mm:ss AM/PM',
    [20] = 'h:mm',
    [21] = 'h:mm:ss',
    [22] = 'm/d/yy h:mm',
    [37] = '(#,##0_);(#,##0)',
    [38] = '(#,##0_);[Red](#,##0)',
    [39] = '(#,##0.00_);(#,##0.00)',
    [40] = '(#,##0.00_);[Red](#,##0.00)',
    [41] = '_(* --,##0_);_(* (#,##0);_(* "-"_);_(_)',
    [42] = '_($* --,##0_);_($* (#,##0);_($* "-"_);_(_)',
    [43] = '_(* --,##0.00_);_(* (#,##0.00);_(* "-"??_);_(_)',
    [44] = '_($* --,##0.00_);_($* (#,##0.00);_($* "-"??_);_(_)',
    [45] = 'mm:ss',
    [46] = '[h]:mm:ss',
    [47] = 'mm:ss.0',
    [48] = '##0.0E+0',
    [49] = '@',
  }

  -- Set the format code for built-in number formats.
  if num_fmt_id < 164 then
    if format_codes[num_fmt_id] then
      format_code = format_codes[num_fmt_id]
    else
      format_code = "General"
    end
  end

  local attributes = {
    {["numFmtId"]   = num_fmt_id},
    {["formatCode"] = format_code},
  }

  self:_xml_empty_tag("numFmt", attributes)
end

----
-- Write the <fonts> element.
--
function Styles:_write_fonts()
  local count = self.font_count
  local attributes = {{["count"] = count}}

  self:_xml_start_tag("fonts", attributes)

  -- Write the font elements for format objects that have them.
  for _, format in ipairs(self.xf_formats) do
    if format.has_font then
      self:_write_font(format)
    end
  end

  self:_xml_end_tag("fonts")
end

----
-- Write the <font> element.
--
function Styles:_write_font(format, is_dxf_format)

  self:_xml_start_tag("font")

  -- The condense and extend elements are mainly used in dxf formats.
  if format.font_condense then
    self:_write_condense()
  end

  if format.font_extend then
    self:_write_extend()
  end

  if format.bold then
    self:_xml_empty_tag("b")
  end

  if format.italic then
    self:_xml_empty_tag("i")
  end

  if format.font_strikeout then
    self:_xml_empty_tag("strike")
  end

  if format.font_outline then
    self:_xml_empty_tag("outline")
  end

  if format.font_shadow then
    self:_xml_empty_tag("shadow")
  end

  -- Handle the underline variants.
  if format.underline then
    self:_write_underline(format.underline)
  end

  if format.font_script == 1 then
    self:_write_vert_align("superscript")
  end

  if format.font_script == 2 then
    self:_write_vert_align("subscript")
  end

  if not is_dxf_format then
    self:_xml_empty_tag("sz", {{['val'] = format.font_size}})
  end

  local theme = format.theme
  local index = format.color_indexed
  local color = format.font_color

  if theme then
    self:_write_color("theme", theme)
  elseif index then
    self:_write_color("indexed", index)
  elseif color then
    self:_write_color("rgb", color)
  elseif not is_dxf_format then
    self:_write_color("theme", "1")
  end

  if not is_dxf_format then
    self:_xml_empty_tag("name",   {{['val'] = format.font_name}})
    self:_xml_empty_tag("family", {{['val'] = format.font_family}})

    if format.font_name == "Calibri" and not format.hyperlink then
      self:_xml_empty_tag( "scheme", {{["val"] = format.font_scheme}})
    end
  end

  self:_xml_end_tag("font")
end

----
-- Write the underline font element.
--
function Styles:_write_underline(underline)
  local attributes = {}

  -- Handle the underline variants.
  if underline == 2 then
    attributes = {{["val"] = "double"}}
  elseif underline == 33 then
    attributes = {{["val"] = "singleAccounting"}}
  elseif underline == 34 then
    attributes = {{["val"] = "doubleAccounting"}}
  else
    attributes = {};    -- Default to single underline.
  end

  self:_xml_empty_tag("u", attributes)
end

----
-- Write the <vertAlign> font sub-element.
--
function Styles:_write_vert_align(val)
  local attributes = {{["val"] = val}}
  self:_xml_empty_tag("vertAlign", attributes)
end

----
-- Write the <color> element.
--
function Styles:_write_color(name, value)
  local attributes = {{[name] = value}}
  self:_xml_empty_tag("color", attributes)
end

----
-- Write the <fills> element.
--
function Styles:_write_fills()
  local count = self.fill_count
  local attributes = {{["count"] = count}}

  self:_xml_start_tag("fills", attributes)

  -- Write the default fill element.
  self:_write_default_fill("none")
  self:_write_default_fill("gray125")

  -- Write the fill elements for format objects that have them.
  for _, format in ipairs(self.xf_formats) do
    if format.has_fill then
      self:_write_fill(format)
    end

  end

  self:_xml_end_tag("fills")
end

----
-- Write the <fill> element for the default fills.
--
function Styles:_write_default_fill(pattern_type)
  self:_xml_start_tag("fill")
  self:_xml_empty_tag("patternFill", {{['patternType'] = pattern_type}})
  self:_xml_end_tag("fill")
end

----
-- Write the <fill> element.
--
function Styles:_write_fill(format, is_dxf_format)
  local pattern    = format.pattern
  local bg_color   = format.bg_color
  local fg_color   = format.fg_color

  -- Colors for dxf formats are handled differently from normal formats since
  -- the normal format reverses the meaning of BG and FG for solid fills.
  if is_dxf_format then
    bg_color = format.dxf_bg_color
    fg_color = format.dxf_fg_color
  end

  local patterns = {
    "none",
    "solid",
    "mediumGray",
    "darkGray",
    "lightGray",
    "darkHorizontal",
    "darkVertical",
    "darkDown",
    "darkUp",
    "darkGrid",
    "darkTrellis",
    "lightHorizontal",
    "lightVertical",
    "lightDown",
    "lightUp",
    "lightGrid",
    "lightTrellis",
    "gray125",
    "gray0625",
  }

  self:_xml_start_tag("fill")

  -- The "none" pattern is handled differently for dxf formats.
  if is_dxf_format and pattern <= 1 then
    self:_xml_start_tag("patternFill")
  else
    self:_xml_start_tag("patternFill",
                        {{["patternType"] = patterns[pattern + 1]}})
  end

  if fg_color then
    self:_xml_empty_tag("fgColor", {{["rgb"] = fg_color}})
  end

  if bg_color then
    self:_xml_empty_tag("bgColor", {{["rgb"] = bg_color}})
  else
    if not is_dxf_format then
      self:_xml_empty_tag("bgColor", {{["indexed"] = "64"}})
    end
  end

  self:_xml_end_tag("patternFill")
  self:_xml_end_tag("fill")
end

----
-- Write the <borders> element.
--
function Styles:_write_borders()
  local count = self.border_count
  local attributes = {{["count"] = count}}

  self:_xml_start_tag("borders", attributes)

  -- Write the border elements for format objects that have them.
  for _, format in ipairs(self.xf_formats) do
    if format.has_border then
      self:_write_border(format)
    end
  end

  self:_xml_end_tag("borders")
end

----
-- Write the <border> element.
--
function Styles:_write_border(format, is_dxf_format)
  local attributes = {}

  -- Diagonal borders add attributes to the <border> element.
  if format.diag_type == 1 then
    table.insert(attributes, {["diagonalUp"] = "1"})
  elseif format.diag_type == 2 then
    table.insert(attributes, {["diagonalDown"] = "1"})
  elseif format.diag_type == 3 then
    table.insert(attributes, {["diagonalUp"]   = "1"})
    table.insert(attributes, {["diagonalDown"] = "1"})
  end

  -- Ensure that a default diag border is set if the diag type is set.
  if format.diag_type > 0 and not format.diag_border then
    format.diag_border = 1
  end

  -- Write the start border tag.
  self:_xml_start_tag("border", attributes)

  -- Write the <border> sub elements.
  self:_write_sub_border("left",   format.left,   format.left_color)
  self:_write_sub_border("right",  format.right,  format.right_color)
  self:_write_sub_border("top",    format.top,    format.top_color)
  self:_write_sub_border("bottom", format.bottom, format.bottom_color)

  -- Condition DXF formats don't allow diagonal borders
  if not is_dxf_format then
    self:_write_sub_border("diagonal",
                           format.diag_border, format.diag_color)
  end

  if is_dxf_format then
    self:_write_sub_border("vertical")
    self:_write_sub_border("horizontal")
  end

  self:_xml_end_tag("border")
end

----
-- Write the <border> sub elements such as <right>, <top>, etc.
--
function Styles:_write_sub_border(border_type, style, color)
  local attributes = {}

  if not style then
    self:_xml_empty_tag(border_type)
    return
  end

  local border_styles = {
    "none",
    "thin",
    "medium",
    "dashed",
    "dotted",
    "thick",
    "double",
    "hair",
    "mediumDashed",
    "dashDot",
    "mediumDashDot",
    "dashDotDot",
    "mediumDashDotDot",
    "slantDashDot",
  }

  table.insert(attributes, {["style"] = border_styles[style + 1]})

  self:_xml_start_tag(border_type, attributes)

  if color then
    self:_xml_empty_tag("color", {{["rgb"] = color}})
  else
    self:_xml_empty_tag("color", {{["auto"] = "1"}})
  end

  self:_xml_end_tag(border_type)
end

----
-- Write the <cellStyleXfs> element.
--
function Styles:_write_cell_style_xfs()
  local attributes = {{["count"] = "1"}}

  self:_xml_start_tag("cellStyleXfs", attributes)
  -- Write the style_xf element.
  self:_write_style_xf()
  self:_xml_end_tag("cellStyleXfs")
end

----
-- Write the <cellXfs> element.
--
function Styles:_write_cell_xfs()
  local formats = self.xf_formats

  -- Workaround for when the last format is used for the comment font
  -- and shouldn't be used for cellXfs.
  local last_format = formats[#formats]
  if last_format and last_format.font_only then
    formats[#formats] = nil
  end

  local count = #formats
  local attributes = {{["count"] = count}}

  self:_xml_start_tag("cellXfs", attributes)

  -- Write the xf elements.
  for _, format in ipairs(formats) do
    self:_write_xf(format)
  end

  self:_xml_end_tag("cellXfs")
end

----
-- Write the style <xf> element.
--
function Styles:_write_style_xf()
  local attributes = {
    {["numFmtId"] = "0"},
    {["fontId"]   = "0"},
    {["fillId"]   = "0"},
    {["borderId"] = "0"},
  }

  self:_xml_empty_tag("xf", attributes)
end

----
-- Write the <xf> element.
--
function Styles:_write_xf(format)
  local num_fmt_id  = format.num_format_index
  local font_id     = format.font_index
  local fill_id     = format.fill_index
  local border_id   = format.border_index
  local xf_id       = 0
  local has_align   = false
  local has_protect = false

  local attributes = {
    {["numFmtId"] = num_fmt_id},
    {["fontId"]   = font_id},
    {["fillId"]   = fill_id},
    {["borderId"] = border_id},
    {["xfId"]     = xf_id},
  }

  if format.num_format_index > 0 then
    table.insert(attributes, {["applyNumberFormat"] = "1"})
  end

  -- Add applyFont attribute if XF format uses a font element.
  if format.font_index > 0 then
    table.insert(attributes, {["applyFont"] = "1"})
  end

  -- Add applyFill attribute if XF format uses a fill element.
  if format.fill_index > 0 then
    table.insert(attributes, {["applyFill"] = "1"})
  end

  -- Add applyBorder attribute if XF format uses a border element.
  if format.border_index > 0 then
    table.insert(attributes, {["applyBorder"] = "1"})
  end

  -- Check if XF format has alignment properties set.
  local apply_align, align = format:_get_align_properties()

  -- Check if an alignment sub-element should be written.
  if apply_align and align then
    has_align = true
  end

  -- We can also have applyAlignment without a sub-element.
  if apply_align then
    table.insert(attributes, {["applyAlignment"] = "1"})
  end

  -- Check for cell protection properties.
  local protection = format:_get_protection_properties()
  if #protection > 0 then
    table.insert(attributes, {["applyProtection"] = "1"})
    has_protect = true
  end

  -- Write XF with sub-elements if required.
  if has_align or has_protect then
    self:_xml_start_tag("xf", attributes)
    if has_align then
      self:_xml_empty_tag("alignment",  align)
    end

    if has_protect then
      self:_xml_empty_tag("protection", protection)
    end

    self:_xml_end_tag("xf")
  else
    self:_xml_empty_tag("xf", attributes)
  end

end

----
-- Write the <cellStyles> element.
--
function Styles:_write_cell_styles()
  local attributes = {{["count"] = "1"}}

  self:_xml_start_tag("cellStyles", attributes)
  -- Write the cellStyle element.
  self:_write_cell_style()
  self:_xml_end_tag("cellStyles")
end

----
-- Write the <cellStyle> element.
--
function Styles:_write_cell_style()
  local name       = "Normal"
  local xf_id      = 0
  local builtin_id = 0

  local attributes = {
    {["name"]      = name},
    {["xfId"]      = xf_id},
    {["builtinId"] = builtin_id},
  }

  self:_xml_empty_tag("cellStyle", attributes)
end

----
-- Write the <dxfs> element.
--
function Styles:_write_dxfs()
  local formats = self.dxf_formats
  local count = #formats

  local attributes = {{["count"] = count}}

  if count > 0 then
    self:_xml_start_tag("dxfs", attributes)

    -- Write the font elements for format objects that have them.
    for _, format in ipairs(self.dxf_formats) do
      self:_xml_start_tag("dxf")
      if format.has_dxf_font then
        self:_write_font(format, true)
      end

      if format.num_format_index > 0 then
        self:_write_num_fmt(format.num_format_index, format.num_format)
      end

      if format.has_dxf_fill then
        self:_write_fill(format, true)
      end

      if format.has_dxf_border then
        self:_write_border(format, true)
      end

      self:_xml_end_tag("dxf")
    end

    self:_xml_end_tag("dxfs")
  else
    self:_xml_empty_tag("dxfs", attributes)
  end

end

----
-- Write the <tableStyles> element.
--
function Styles:_write_table_styles()
  local count               = 0
  local default_table_style = "TableStyleMedium9"
  local default_pivot_style = "PivotStyleLight16"

  local attributes = {
    {["count"]             = count},
    {["defaultTableStyle"] = default_table_style},
    {["defaultPivotStyle"] = default_pivot_style},
  }

  self:_xml_empty_tag("tableStyles", attributes)
end

----
-- Write the <colors> element.
--
function Styles:_write_colors()

  if #self.custom_colors == 0 then return end

  self:_xml_start_tag("colors")
  self:_write_mru_colors(self.custom_colors)
  self:_xml_end_tag("colors")
end

----
-- Write the <mruColors> element for the most recently used colours.
--
function Styles:_write_mru_colors(custom_colors)

  -- Limit the mruColors to the last 10.
  if #custom_colors > 10 then
      custom_colors = unpack(custom_colors, 1, 10)
  end

  self:_xml_start_tag("mruColors")

  -- Write the custom colors in reverse order.
  for i = #custom_colors, 1, -1 do
    self:_write_color("rgb", custom_colors[i])
  end

  self:_xml_end_tag("mruColors")
end

----
-- Write the <condense> element.
--
function Styles:_write_condense()
  local attributes = {{["val"]  = "0"}}
  self:_xml_empty_tag("condense", attributes)
end

----
-- Write the <extend> element.
--
function Styles:_write_extend()
  local attributes = {{["val"] = "0"}}
  self:_xml_empty_tag("extend", attributes)
end

return Styles
