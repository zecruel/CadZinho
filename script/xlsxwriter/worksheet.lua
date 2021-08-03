----
-- Worksheet - A class for writing the Excel XLSX Worksheet file.
--
-- Copyright 2014-2015, John McNamara, jmcnamara@cpan.org
--
require "xlsxwriter.strict"

local Utility   = require "xlsxwriter.utility"
local Xmlwriter = require "xlsxwriter.xmlwriter"


local xl_rowmax = 1048576
local xl_colmax = 16384
local xl_strmax = 32767


------------------------------------------------------------------------------
--
-- Public methods.
--
------------------------------------------------------------------------------


----
-- The constructor inherits from xmlwriter.lua.
--

local Worksheet = {}
setmetatable(Worksheet,{__index = Xmlwriter})

function Worksheet:new()
  local instance = {
    optimization           = false,
    data_table             = {},

    ext_sheets             = {},
    fileclosed             = false,
    excel_version          = 2007,
    xls_rowmax             = xl_rowmax,
    xls_colmax             = xl_colmax,
    xls_strmax             = xl_strmax,
    dim_rowmin             = nil,
    dim_rowmax             = nil,
    dim_colmin             = nil,
    dim_colmax             = nil,
    colinfo                = {},
    selections             = {},
    hidden                 = false,
    active                 = false,
    tab_color              = false,
    panes                  = {},
    active_pane            = 3,
    selected               = false,
    page_setup_changed     = false,
    paper_size             = 0,
    orientation            = true,
    print_options_changed  = false,
    hcenter                = false,
    vcenter                = false,
    print_gridlines        = false,
    screen_gridlines       = true,
    print_headers          = false,
    header_footer_changed  = false,
    header                 = "",
    footer                 = "",
    margin_left            = 0.7,
    margin_right           = 0.7,
    margin_top             = 0.75,
    margin_bottom          = 0.75,
    margin_header          = 0.3,
    margin_footer          = 0.3,
    repeat_row_range       = "",
    repeat_col_range       = "",
    print_area_range       = "",
    page_order             = false,
    black_white            = false,
    draft_quality          = false,
    print_comments         = false,
    page_start             = 0,
    fit_page               = false,
    fit_width              = 0,
    fit_height             = 0,
    hbreaks                = {},
    vbreaks                = {},
    protect_options        = false,
    password               = nil,
    set_cols               = {},
    set_rows               = {},
    zoom                   = 100,
    zoom_scale_normal      = true,
    print_scale            = 100,
    right_to_left          = false,
    show_zeros             = true,
    leading_zeros          = false,
    outline_row_level      = 0,
    outline_col_level      = 0,
    outline_style          = 0,
    outline_below          = true,
    outline_right          = true,
    outline_on             = true,
    outline_changed        = false,
    default_row_height     = 15,
    default_row_zeroed     = false,
    names                  = {},
    write_match            = {},
    merge                  = {},
    has_vml                = false,
    has_comments           = false,
    comments               = {},
    comments_array         = {},
    comments_author        = "",
    comments_visible       = false,
    vml_shape_id           = 1024,
    buttons_array          = {},
    autofilter_area        = "",
    filter_on              = false,
    filter_range           = {},
    filter_cols            = {},
    col_sizes              = {},
    row_sizes              = {},
    col_formats            = {},
    col_size_changed       = false,
    row_size_changed       = false,
    last_shape_id          = 1,
    rel_count              = 0,
    hlink_count            = 0,
    hlink_refs             = {},
    external_hyper_links   = {},
    external_drawing_links = {},
    external_comment_links = {},
    external_vml_links     = {},
    external_table_links   = {},
    drawing_links          = {},
    charts                 = {},
    images                 = {},
    tables                 = {},
    sparklines             = {},
    shapes                 = {},
    shape_hash             = {},
    has_shapes             = false,
    drawing                = false,
    rstring                = "",
    previous_row           = 0,
    hyperlinks             = {},
  }

  setmetatable(instance, self)
  self.__index = self
  return instance
end

----
-- Initialise the worksheet from the workbook.
--
function Worksheet:_initialize(init_data)

  self.name                = init_data['name']
  self.index               = init_data['index']
  self.str_table           = init_data['str_table']
  self.worksheet_meta      = init_data['worksheet_meta']
  self.optimization        = init_data['optimization']
  self.tmpdir              = init_data['tmpdir']
  self.date_1904           = init_data['date_1904']
  self.strings_to_numbers  = init_data['strings_to_numbers']
  self.strings_to_formulas = init_data['strings_to_formulas']
  self.strings_to_urls     = init_data['strings_to_urls']
  self.default_date_format = init_data['default_date_format']
  self.default_url_format  = init_data['default_url_format']

  -- Open a temp filehandle to store row data in optimization mode.
  if self.optimization then
    self.row_data_fh = io.tmpfile()
    -- Set as the worksheet filehandle until the file is assembled.
    self.fh = self.row_data_fh
  end
end


----
-- Assemble and write the XML file.
--
function Worksheet:_assemble_xml_file()

  self:_xml_declaration()

  -- Write the root worksheet element.
  self:_write_worksheet()

  -- Write the worksheet properties.
  self:_write_sheet_pr()

  -- Write the worksheet dimensions.
  self:_write_dimension()

  -- Write the sheet view properties.
  self:_write_sheet_views()

  -- Write the sheet format properties.
  self:_write_sheet_format_pr()

  -- Write the sheet column info.
  self:_write_cols()

  -- Write the worksheet data such as rows columns and cells.
  if self.optimization then
    self:_write_optimized_sheet_data()
  else
    self:_write_sheet_data()
  end

  -- Write the sheetProtection element.
  self:_write_sheet_protection()

  -- Write the autoFilter element.
  -- self:_write_auto_filter()

  -- Write the mergeCells element.
  self:_write_merge_cells()

  -- Write the conditional formats.
  -- self:_write_conditional_formats()

  -- Write the dataValidations element.
  -- self:_write_data_validations()

  -- Write the hyperlink element.
  self:_write_hyperlinks()

  -- Write the printOptions element.
  self:_write_print_options()

  -- Write the worksheet page_margins.
  self:_write_page_margins()

  -- Write the worksheet page setup.
  self:_write_page_setup()

  -- Write the headerFooter element.
  self:_write_header_footer()

  -- Write the rowBreaks element.
  self:_write_row_breaks()

  -- Write the colBreaks element.
  self:_write_col_breaks()

  -- Write the drawing element.
  -- self:_write_drawings()

  -- Write the legacyDrawing element.
  -- self:_write_legacy_drawing()

  -- Write the tableParts element.
  -- self:_write_table_parts()

  -- Write the extLst and sparklines.
  -- self:_write_ext_sparklines()

  -- Close the worksheet tag.
  self:_xml_end_tag("worksheet")

  -- Close the XML writer filehandle.
  self:_xml_close()
end


----
-- Write data to a worksheet cell by calling the appropriate write_*()
-- method based on the type of data being passed.
--
-- Args:
--     row:   The cell row (zero indexed).
--     col:   The cell column (zero indexed).
--     args:  Args to pass to sub functions.
--
-- Returns:
--     0:     Success.
--     -1:    Row or column is out of worksheet bounds.
--     other: Return value of called method.
--
function Worksheet:write(...)
  local row, col, token, format = self:_convert_cell_args(...)

  if type(token) == "string" then
    if token == "" then
      return self:_write_blank(row, col, token, format)
    elseif string.match(token, "^=") then
      return self:_write_formula(row, col, token, format)
    else
      return self:_write_string(row, col, token, format)
    end
  elseif type(token) == "number" then
    return self:_write_number(row, col, token, format)
  elseif type(token) == "boolean" then
    return self:_write_boolean(row, col,token, format)
  elseif type(token) == "nil" then
    return self:_write_blank(row, col, "", format)
  else
    return self:_write_string(row, col, tostring(token), format)
  end
end

----
-- Write a string to a worksheet cell.
--
-- Args:
--     row:    The cell row (zero indexed).
--     col:    The cell column (zero indexed).
--     string: Cell data. string.
--     format: An optional cell Format object.
--
-- Returns:
--     0:  Success.
--     -1: Row or column is out of worksheet bounds.
--     -2: String > 32k characters.
--
function Worksheet:write_string(...)
  return self:_write_string(self:_convert_cell_args(...))
end

----
-- Write a number to a worksheet cell.
--
-- Args:
--     row:    The cell row (zero indexed).
--     col:    The cell column (zero indexed).
--     number: Cell data. number.
--     format: An optional cell Format object.
--
-- Returns:
--     0:  Success.
--     -1: Row or column is out of worksheet bounds.
--
function Worksheet:write_number(...)
  return self:_write_number(self:_convert_cell_args(...))
end

----
-- Write a blank cell with formatting to a worksheet cell. The blank
-- token is ignored and the format only is written to the cell.
--
-- Args:
--     row:         The cell row (zero indexed).
--     col:         The cell column (zero indexed).
--     blank:       Any value. It is ignored.
--     format:      An optional cell Format object.
--
-- Returns:
--     0:  Success.
--     -1: Row or column is out of worksheet bounds.
--
function Worksheet:write_blank(...)
  return self:_write_blank(self:_convert_cell_args(...))
end

----
-- Write a formula to a worksheet cell.
--
-- Args:
--     first_row:    The first row of the cell range. (zero indexed).
--     first_col:    The first column of the cell range.
--     last_row:     The last row of the cell range. (zero indexed).
--     last_col:     The last column of the cell range.
--     formula:      Cell formula.
--     format:       An optional cell Format object.
--     value:        An optional value for the formula. Default is 0.
--
-- Returns:
--     0:  Success.
--     -1: Row or column is out of worksheet bounds.
--
--
function Worksheet:write_formula(...)
  return self:_write_formula(self:_convert_cell_args(...))
end

----
-- Write a formula to a worksheet cell.
--
-- Args:
--     first_row:    The first row of the cell range. (zero indexed).
--     first_col:    The first column of the cell range.
--     last_row:     The last row of the cell range. (zero indexed).
--     last_col:     The last column of the cell range.
--     formula:      Cell formula.
--     format:       An optional cell Format object.
--     value:        An optional value for the formula. Default is 0.
--
-- Returns:
--     0:  Success.
--     -1: Row or column is out of worksheet bounds.
--
function Worksheet:write_array_formula(...)
  return self:_write_array_formula(self:_convert_range_args(...))
end

----
-- Write an os.time style table as a date or time to a worksheet cell.
--
-- Args:
--     row:         The cell row (zero indexed).
--     col:         The cell column (zero indexed).
--     date_time:   An os.time style table.
--     format:      A cell Format object.
--
-- Returns:
--     0:  Success.
--     -1: Row or column is out of worksheet bounds.
--
function Worksheet:write_date_time(...)
  return self:_write_date_time(self:_convert_cell_args(...))
end

----
-- Write a datetime string in ISO8601 "yyyy-mm-ddThh:mm:ss.ss" format as
-- a date or time to a worksheet cell.
--
-- Args:
--     row:         The cell row (zero indexed).
--     col:         The cell column (zero indexed).
--     date_string: An ISO8601 sytle date string.
--     format:      A cell Format object.
--
-- Returns:
--     0:  Success.
--     -1: Row or column is out of worksheet bounds.
--
function Worksheet:write_date_string(...)
  return self:_write_date_string(self:_convert_cell_args(...))
end

----
-- Write a boolean value to a worksheet cell.
--
-- Args:
--     row:         The cell row (zero indexed).
--     col:         The cell column (zero indexed).
--     boolean:     Cell data. bool type.
--     format:      An optional cell Format object.
--
-- Returns:
--     0:  Success.
--     -1: Row or column is out of worksheet bounds.
--
function Worksheet:write_boolean(...)
  return self:_write_boolean(self:_convert_cell_args(...))
end

----
-- Write a hyperlink to a worksheet cell.
--
-- Args:
--     row:    The cell row (zero indexed).
--     col:    The cell column (zero indexed).
--     url:    Hyperlink url.
--     format: An optional cell Format object.
--     string: An optional display string for the hyperlink.
--     tip:    An optional tooltip.
-- Returns:
--     0:  Success.
--     -1: Row or column is out of worksheet bounds.
--     -2: String longer than 32767 characters.
--     -3: URL longer than Excel limit of 255 characters
--     -4: Exceeds Excel limit of 65,530 urls per worksheet
function Worksheet:write_url(...)
  return self:_write_url(self:_convert_cell_args(...))
end

----
-- Set this worksheet as the active worksheet, i.e. the worksheet that is
-- displayed when the workbook is opened. Also set it as selected.
--
-- Note: An active worksheet cannot be hidden.
--
-- Args:
--     None.
--
-- Returns:
--     Nothing.
--
function Worksheet:select()
  -- Selected worksheet can't be hidden.
  self.hidden   = false
  self.selected = true
end

----
-- Set this worksheet as the active worksheet, i.e. the worksheet that is
-- displayed when the workbook is opened. Also set it as selected.
--
-- Note: An active worksheet cannot be hidden.
--
-- Args:
--     None.
--
-- Returns:
--     Nothing.
--
function Worksheet:activate()
  -- Active worksheet can't be hidden.
  self.hidden   = false
  self.selected = true
  self.worksheet_meta.activesheet = self.index
end

----
-- Hide the current worksheet.
--
-- Args:
--     None.
--
-- Returns:
--     Nothing.
--
function Worksheet:hide()
  self.hidden = true

  -- A hidden worksheet shouldn't be active or selected.
  self.selected = false
end

----
-- Set the password and protection options of the worksheet.
--
-- Args:
--     password: An optional password string.
--     options:  A table of worksheet objects to protect.
--
-- Returns:
--     Nothing.
--
function Worksheet:protect(password, options)

  password = password or ''
  options  = options or {}

  -- Default values for objects that can be protected.
  local defaults = {
    ["sheet"]                 = true,
    ["content"]               = false,
    ["objects"]               = false,
    ["scenarios"]             = false,
    ["format_cells"]          = false,
    ["format_columns"]        = false,
    ["format_rows"]           = false,
    ["insert_columns"]        = false,
    ["insert_rows"]           = false,
    ["insert_hyperlinks"]     = false,
    ["delete_columns"]        = false,
    ["delete_rows"]           = false,
    ["select_locked_cells"]   = true,
    ["sort"]                  = false,
    ["autofilter"]            = false,
    ["pivot_tables"]          = false,
    ["select_unlocked_cells"] = true,
  }

  -- Overwrite the defaults with user specified values.
  for key, _ in pairs(options) do
    if defaults[key] ~= nil then
      defaults[key] = options[key]
    else
      Utility.warn("Unknown protection object: " .. key .. "\n")
    end
  end

  if password ~= '' then
    -- Set the password after the user defined values.
    defaults.password = self:_encode_password(password)
  end

  self.protect_options = defaults
end

----
-- Set current worksheet as the first visible sheet. This is necessary
-- when there are a large number of worksheets and the activated
-- worksheet is not visible on the screen.
--
-- Note: A selected worksheet cannot be hidden.
--
-- Args:
--     None.
--
-- Returns:
--     Nothing.
--
function Worksheet:set_first_sheet()
  -- Active worksheet can't be hidden.
  self.hidden = false
  self.worksheet_meta.firstsheet = self.index
end

----
-- Set the option to hide gridlines on the screen and the printed page.
--
-- Args:
--     option:    0 : Don't hide gridlines
--                1 : Hide printed gridlines only
--                2 : Hide screen and printed gridlines
--
-- Returns:
--     Nothing.
--
function Worksheet:hide_gridlines(option)

  -- Default to hiding printed gridlines, option = 1.
  if option == 0 then
    self.print_gridlines       = true
    self.screen_gridlines      = true
    self.print_options_changed = true
  elseif not option or option == 1 then
    self.print_gridlines  = false
    self.screen_gridlines = true
  else
    self.print_gridlines  = false
    self.screen_gridlines = false
  end
end

----
-- Set the option to print the row and column headers on the printed page.
--
-- Args:
--     None.
--
-- Returns:
--     Nothing.
--
function Worksheet:print_row_col_headers()
  self.print_headers         = true
  self.print_options_changed = true
end

----
-- Fit the printed area to a specific number of pages both vertically and
-- horizontally.
--
-- Args:
--     width:  Number of pages horizontally.
--     height: Number of pages vertically.
--
-- Returns:
--     Nothing.
--
function Worksheet:fit_to_pages(width, height)
  self.fit_page           = true
  self.fit_width          = width  or 1
  self.fit_height         = height or 1
  self.page_setup_changed = true
end

----
-- Set the horizontal page breaks on a worksheet.
--
-- Args:
--     breaks: List of rows where the page breaks should be added.
--
-- Returns:
--     Nothing.
--

function Worksheet:set_h_pagebreaks(breaks)
  self.hbreaks = breaks
end

----
-- Set the horizontal page breaks on a worksheet.
--
-- Args:
--     breaks: List of columns where the page breaks should be added.
--
-- Returns:
--     Nothing.
--
function Worksheet:set_v_pagebreaks(breaks)
  self.vbreaks = breaks
end

----
-- Set the worksheet zoom factor.
--
-- Args:
--     zoom: Scale factor: 10 <= zoom <= 400.
--
-- Returns:
--     Nothing.
--
function Worksheet:set_zoom(scale)
  -- Confine the scale to Excel's range
  if scale < 10 or scale > 400 then
    Utility.warn("Zoom factor scale outside range: 10 <= zoom <= 400\n")
    scale = 100
  end

  self.zoom = math.floor(scale)
end

----
-- Set the scale factor for the printed page.
--
-- Args:
--     scale: Print scale. 10 <= scale <= 400.
--
-- Returns:
--     Nothing.
--
function Worksheet:set_print_scale(scale)

  -- Confine the scale to Excel's range
  if scale < 10 or scale > 400 then
    Utility.warn("Print scale scale outside range: 10 <= zoom <= 400\n")
    scale = 100
  end

  -- Turn off "fit to page" option.
  self.fit_page = false

  self.print_scale        = math.floor(scale)
  self.page_setup_changed = true
end

----
-- Set the print area in the current worksheet.
--
-- Args:
--     first_row:    The first row of the cell range. (zero indexed).
--     first_col:    The first column of the cell range.
--     last_row:     The last row of the cell range. (zero indexed).
--     last_col:     The last column of the cell range.
--
-- Returns:
--     0:  Success.
--     -1: Row or column is out of worksheet bounds.
--

function Worksheet:print_area(...)

  local row1, col1, row2, col2 = self:_convert_range_args(...)

  -- Ignore max print area since this is the same as no print area for Excel.
  if  row1 == 0             and col1 == 0
  and row2 == xl_rowmax - 1 and col2 == xl_colmax - 1 then
    return
  end

  -- Build up the print area range "=Sheet2!R1C1:R2C1"
  local range = self:_convert_name_area(row1, col1, row2, col2)

  self.print_area_range = range
end

----
-- Display the worksheet right to left for some versions of Excel.
--
-- Args:
--     None.
--
-- Returns:
--     Nothing.
--
function Worksheet:set_right_to_left()
  self.right_to_left = true
end

----
-- Hide zero values in worksheet cells.
--
-- Args:
--     None.
--
-- Returns:
--     Nothing.
--
function Worksheet:hide_zero()
  self.show_zeros = false
end

----
-- Set the order in which pages are printed.
--
-- Args:
--     None.
--
-- Returns:
--     Nothing.
--
function Worksheet:print_across()
  self.page_order         = true
  self.page_setup_changed = true
end

----
-- Set the start page number when printing.
--
-- Args:
--     start_page: Start page number.
--
-- Returns:
--     Nothing.
--
function Worksheet:set_start_page(value)
  self.page_start   = value
  self.custom_start = true
end

----
-- Set the page orientation as portrait.
--
-- Args:
--     None.
--
-- Returns:
--     Nothing.
--
function Worksheet:set_portrait()
  self.orientation        = true
  self.page_setup_changed = true
end

----
-- Set the page orientation as landscape.
--
-- Args:
--     None.
--
-- Returns:
--     Nothing.
--
function Worksheet:set_landscape()
  self.orientation        = false
  self.page_setup_changed = true
end

----
-- Set the page view mode.
--
-- Args:
--     None.
--
-- Returns:
--     Nothing.
--
function Worksheet:set_page_view()
  self.page_view = true
end

----
-- Set the colour of the worksheet tab.
--
-- Args:
--     color: A #RGB color index.
--
-- Returns:
--     Nothing.
--
function Worksheet:set_tab_color(color)
  self.tab_color = color
end

----
-- Set the paper type. US Letter = 1, A4 = 9.
--
-- Args:
--     paper_size: Paper index.
--
-- Returns:
--     Nothing.
--
function Worksheet:set_paper(paper_size)
  self.paper_size         = paper_size
  self.page_setup_changed = true
end

----
-- Set the page header caption and optional margin.
--
-- Args:
--     header: Header string.
--     margin: Header margin.
--
-- Returns:
--     Nothing.
--
function Worksheet:set_header(header, margin)

  if #header >= 255 then
    Utility.warn("Header string must be less than 255 characters\n")
    return
  end

  self.header                = header
  self.margin_header         = margin and margin or 0.3
  self.header_footer_changed = true
end

----
-- Set the page footer caption and optional margin.
--
-- Args:
--     footer: Footer string.
--     margin: Footer margin.
--
-- Returns:
--     Nothing.
--
function Worksheet:set_footer(footer, margin)

  if #footer >= 255 then
    Utility.warn("Footer string must be less than 255 characters\n")
    return
  end

  self.footer                = footer
  self.margin_footer         = margin and margin or 0.3
  self.header_footer_changed = true
end

----
-- Center the page horizontally.
--
-- Args:
--     None.
--
-- Returns:
--     Nothing.
--
function Worksheet:center_horizontally()
  self.print_options_changed = true
  self.hcenter               = true
end

----
-- Center the page vertically.
--
-- Args:
--     None.
--
-- Returns:
--     Nothing.
--
function Worksheet:center_vertically()
  self.print_options_changed = true
  self.vcenter               = true
end

----
-- Set all the page margins in inches.
--
-- Args:
--     left:   Left margin.
--     right:  Right margin.
--     top:    Top margin.
--     bottom: Bottom margin.
--
-- Returns:
--     Nothing.
--
function Worksheet:set_margins(left, right, top, bottom)
  self.margin_left   = left   and left   or 0.7
  self.margin_right  = right  and right  or 0.7
  self.margin_top    = top    and top    or 0.75
  self.margin_bottom = bottom and bottom or 0.75
end


function Worksheet:repeat_rows(row_min, row_max)

  -- Second row is optional.
  local row_max = row_max or row_min

  -- Convert to 1 based.
  row_min = row_min + 1
  row_max = row_max + 1

  local range = "$" .. row_min .. ':' .. '$' .. row_max

  -- Build up the print titles "Sheet1!1:2"
  local sheetname = self:_quote_sheetname(self.name)
  range = sheetname .. "!" .. range

  self.repeat_row_range = range
end


----
-- Set the width, and other properties of a single column or a
-- range of columns.
--
-- Args:
--     firstcol:    First column (zero-indexed).
--     lastcol:     Last column (zero-indexed). Can be same as firstcol.
--     width:       Column width. (optional).
--     format:      Column cell_format. (optional).
--     options:     Table of options such as hidden and level.
--
-- Returns:
--     0:  Success.
--     -1: Column number is out of worksheet bounds.
--
function Worksheet:set_column(...)
  return self:_set_column(self:_convert_column_args(...))
end

----
-- Set the width, and other properties of a row.
-- range of columns.
--
-- Args:
--     row:         Row number (zero-indexed).
--     height:      Row width. (optional).
--     format:      Row cell_format. (optional).
--     options:     Table of options such as hidden, level and collapsed.
--
-- Returns:
--     0:  Success.
--     -1: Row number is out of worksheet bounds.
--
function Worksheet:set_row(row, height, format, options)

  -- Set the optional column values.
  options = options or {}
  local hidden    = options["hidden"] or false
  local collapsed = options["collapsed"] or false
  local level     = options["level"] or 0

  -- Use min col in _check_dimensions(). Default to 0 if undefined.
  local min_col = self.dim_colmin and self.dim_colmin or 0

  -- Check that row is valid.
  if not self:_check_dimensions(row, min_col) then
    return -1
  end

  -- Get the default row height.
  local default_height = self.default_row_height

  if not height then
    height = default_height
  end

  -- If the height is 0 the row is hidden and the height is the default.
  if height == 0 then
    hidden = 1
    height = default_height
  end

  -- Set the limits for the outline levels (0 <= x <= 7).
  if level < 0 then level = 0  end
  if level > 7 then level = 7  end

  if level > self.outline_row_level then
    self.outline_row_level = level
  end

  -- Store the row properties.
  self.set_rows[row] = {height, format, hidden, level, collapsed}

  -- Store the row change to allow optimisations.
  self.row_size_changed = 1

  -- Store the row sizes for use when calculating image vertices.
  -- Hidden rows are calculted as 0.
  if hidden then height = 0 end
  self.row_sizes[row] = height

end

----
-- Set the default author of the cell comments.
--
-- Args:
--     author: Comment author name. String.
--
-- Returns:
--     Nothing.
--
function Worksheet:get_name()
  return self.name
end

----
-- Merge a range of cells.
--
-- Args:
--     first_row:    The first row of the cell range. (zero indexed).
--     first_col:    The first column of the cell range.
--     last_row:     The last row of the cell range. (zero indexed).
--     last_col:     The last column of the cell range.
--     data:         Cell data.
--     format:       Cell Format object.
--
-- Returns:
--      0:    Success.
--     -1:    Row or column is out of worksheet bounds.
--     other: Return value of write().
--
function Worksheet:merge_range(...)

  local first_row, first_col, last_row, last_col, data, format
    = self:_convert_range_args(...)


  -- Excel doesn't allow a single cell to be merged.
  if first_row == last_row and first_col == last_col then
    Utility.warn( "Can't merge single cell\n")
  end

  -- Swap last row/col with first row/col as necessary.
  if first_row > last_row then first_row, last_row = last_row, first_row end
  if first_col > last_col then first_col, last_col = last_col, first_col end

  -- Check that column number is valid and store the max value
  if not self:_check_dimensions(last_row, last_col) then
    return
  end

  -- Store the merge range.
  table.insert(self.merge, {first_row, first_col, last_row, last_col})

  -- Write the first cell.
  self:write(first_row, first_col, data, format)

  -- Pad out the rest of the area with formatted blank cells.
  for row = first_row, last_row do
    for col = first_col, last_col do
      if row ~= first_row or col ~= first_col then
        self:write_blank(row, col, "", format)
      end
    end
  end
end

------------------------------------------------------------------------------
--
-- Internal methods.
--
------------------------------------------------------------------------------

----
-- Sheetnames used in references should be quoted if they contain any spaces,
-- special characters or if the look like something that isn't a sheet name.
--
function Worksheet:_quote_sheetname(sheetname)
  if sheetname:match("^Sheet%d+$") then
    return sheetname
  else
    return '"' .. sheetname .. '"'
  end
end


----
-- Decorator function to convert "A1" notation in cell method calls
-- to the default row/col notation.
--
function Worksheet:_convert_cell_args(...)
  if type(...) == "string" then
    -- Convert "A1" style cell to row, col.
    local cell = ...
    local row, col = Utility.cell_to_rowcol(cell)
    return row, col, unpack({...}, 2)
  else
    -- Parameters are already in row, col format.
    return ...
  end
end

----
-- Decorator function to convert "A1:G9" style ranges to row/cols.
--
function Worksheet:_convert_range_args(...)
  if type(...) == "string" then
    -- Convert "A1:G9" style range to row1, col1, row2, col2.
    local range = ...
    local range_start, range_end = range:match("(%S+):(%S+)")
    local row_1, col_1 = Utility.cell_to_rowcol(range_start)
    local row_2, col_2 = Utility.cell_to_rowcol(range_end)
    return row_1, col_1, row_2, col_2, unpack({...}, 2)
  else
    -- Parameters are already in row, col format.
    return ...
  end
end

----
-- Decorator function to convert "A:Z" column range calls to column numbers.
--
function Worksheet:_convert_column_args(...)
  if type(...) == "string" then
    -- Convert "A:Z" style range to col, col.
    local range = ...
    local range_start, range_end = range:match("(%S+):(%S+)")
    local _, col_1 = Utility.cell_to_rowcol(range_start .. "1")
    local _, col_2 = Utility.cell_to_rowcol(range_end   .. "1")
    return col_1, col_2, unpack({...}, 2)
  else
    -- Parameters are already in col, col format.
    return ...
  end
end

----
-- Write a string to a Worksheet cell.
--
function Worksheet:_write_string(row, col, str, format)

  if not self:_check_dimensions(row, col) then
    return -1
  end

  -- Write a shared string if not in optimisation mode.
  if not self.optimization then
    str = self.str_table:_get_string_index(str)
  end

  -- Write previous row if this is a new row, in optimization mode.
  if self.optimization and row > self.previous_row then
    self:_write_single_row(row)
  end

  if not self.data_table[row] then
    self.data_table[row] = {}
  end

  self.data_table[row][col] = {'s', str, format}

  return 0

end

----
-- Write a number to a Worksheet cell.
--
function Worksheet:_write_number(row, col, num, format)

  if not self:_check_dimensions(row, col) then
    return -1
  end

  -- Write previous row if this is a new row, in optimization mode.
  if self.optimization and row > self.previous_row then
    self:_write_single_row(row)
  end

  if not self.data_table[row] then
    self.data_table[row] = {}
  end

  self.data_table[row][col] = {'n', num, format}

  return 0
end

----
-- Write a blank to a Worksheet cell.
--
function Worksheet:_write_blank(row, col, num, format)

  -- Don't write a blank cell unless it has a format.
  if not format then return 0 end

  if not self:_check_dimensions(row, col) then
    return -1
  end

  -- Write previous row if this is a new row, in optimization mode.
  if self.optimization and row > self.previous_row then
    self:_write_single_row(row)
  end

  if not self.data_table[row] then
    self.data_table[row] = {}
  end

  self.data_table[row][col] = {'b', num, format}

  return 0
end

----
-- Write a formula to a Worksheet cell.
--
function Worksheet:_write_formula(row, col, formula, format, value)

  if not self:_check_dimensions(row, col) then
    return -1
  end

  -- Hand off array formulas.
  if formula:match('^{') and formula:match('}$') then
    return self:write_array_formula(row, col, row, col, formula, format, value)
  end

  -- Strip the formula = sign, if it exists.
  if formula:match('^=') then formula = formula:sub(2) end

  -- Write previous row if this is a new row, in optimization mode.
  if self.optimization and row > self.previous_row then
    self:_write_single_row(row)
  end

  if not self.data_table[row] then
    self.data_table[row] = {}
  end

  self.data_table[row][col] = {'f', formula, format, value}

  return 0
end

----
-- Write a array_formula to a Worksheet cell.
--
function Worksheet:_write_array_formula(first_row, first_col,
                                        last_row, last_col,
                                        formula, format, value)

  -- Swap last row/col with first row/col as necessary.
  if first_row > last_row then first_row, last_row = last_row, first_row end
  if first_col > last_col then first_col, last_col = last_col, first_col end

  if not self:_check_dimensions(last_row, last_col) then
    return -1
  end

  -- Define array range
  local range
  if first_row == last_row and first_col == last_col then
    range = Utility.rowcol_to_cell(first_row, first_col)
  else
    range = Utility.range(first_row, first_col, last_row, last_col)
  end

  -- Remove array formula braces and the leading =.
  if formula:match('^{') then formula = formula:sub(2) end
  if formula:match('^=') then formula = formula:sub(2) end
  if formula:match('}$') then formula = formula:sub(1, -2) end

  -- Set a default formula value.
  value = value or 0

  -- Write previous row if this is a new row, in optimization mode.
  if self.optimization and first_row > self.previous_row then
    self:_write_single_row(first_row)
  end

  if not self.data_table[first_row] then
    self.data_table[first_row] = {}
  end

  self.data_table[first_row][first_col] = {'a', formula, format, value, range}

  -- Pad out the rest of the area with formatted zeroes.
  if not self.optimization then
    for row = first_row, last_row do
      for col = first_col, last_col do
        if row ~= first_row or col ~= first_col then
          self:write_number(row, col, 0, format)
        end
      end
    end
  end

return 0
end

----
-- Write a os.time() style table to a Worksheet cell.
--
function Worksheet:_write_date_time(row, col, date_time, format)
  local date = Utility.convert_date_time(date_time, self.date_1904)

  return self:_write_number(row, col, date, format)
end

----
-- Write an ISO8601 style date string to a Worksheet cell.
-- Todo.
--
function Worksheet:_write_date_string(row, col, date_string, format)
  local date = Utility.convert_date_string(date_string, self.date_1904)

  return self:_write_number(row, col, date, format)
end

----
-- Write a boolean to a Worksheet cell.
--
function Worksheet:_write_boolean(row, col, bool, format)

  if not self:_check_dimensions(row, col) then
    return -1
  end

  -- Write previous row if this is a new row, in optimization mode.
  if self.optimization and row > self.previous_row then
    self:_write_single_row(row)
  end

  if not self.data_table[row] then
    self.data_table[row] = {}
  end

  bool = bool and 1 or 0

  self.data_table[row][col] = {'l', bool, format}

  return 0
end

function Worksheet:_write_url(row, col, url, format, str, tip)

  local link_type = 1
  local matched   = 0

  -- Remove the URI scheme from internal links.
  url, matched = url:gsub("internal:", "")
  if matched > 0 then link_type = 2 end

  -- Remove the URI scheme from external links.
  url, matched = url:gsub("external:", "")
  if matched > 0 then link_type = 3 end

  -- The displayed string defaults to the url string.
  if not str then str = url end

  -- For external links change the directory separator from Unix to Dos.
  if link_type == 3 then
    url = url:gsub("/", "\\")
    str = str:gsub("/", "\\")
  end

  -- Strip the mailto header.
  str = str:gsub("^mailto:", "")

  -- Check that row and col are valid and store max and min values
  if not self:_check_dimensions(row, col) then
    return -1
  end

  -- Check that the string is < 32767 chars
  if #str > self.xls_strmax then
    return -2
  end

  -- Copy string for use in hyperlink elements.
  local url_str = str

  -- External links to URLs and to other Excel workbooks have slightly
  -- different characteristics that we have to account for.
  if link_type == 1 then

    -- Escape URL unless it looks already escaped.
    if not url:match("%%%x%x") then
      url = url:gsub("%%", "%%25")
      url = url:gsub('"',  "%%22")
      url = url:gsub(" ",  "%%20")
      url = url:gsub("<",  "%%3c")
      url = url:gsub(">",  "%%3e")
      url = url:gsub("%[", "%%5b")
      url = url:gsub("%]", "%%5d")
      url = url:gsub("%^", "%%5e")
      url = url:gsub("`",  "%%60")
      url = url:gsub("{",  "%%7b")
      url = url:gsub("}",  "%%7d")
    end

    -- Ordinary URL style external links don't have a "location" string.
    url_str = nil

  elseif link_type == 3 then
    -- External Workbook links need to be modified into the right format.
    -- The URL will look something like 'c:\temp\file.xlsx#Sheet!A1'.
    -- We need the part to the left of the # as the URL and the part to
    -- the right as the "location" string (if it exists).
    if url:match("#") then
      url, url_str = url:match("([^#]+)#(.*)")
    else
      url_str = nil
    end

    -- Add the file:/// URI to the url if non-local. For:
    --    Windows style "C:/" link.
    --    Network share.
    if url:match("^%a:") or url:match("^\\\\") then
      url = "file:///" .. url
    end

    -- Convert a ./dir/file.xlsx link to dir/file.xlsx.
    url = url:gsub("^.\\", "")

    -- Treat as a default external link now that the data has been modified.
    link_type = 1
  end

  -- Excel limits escaped URL to 255 characters.
  if #url > 255 then
    Utility.warn("Ignoring URL > 255 characters since it "
                 .. "exceeds Excel's limit for URLS.\n")
    return -3
  end

  -- Check the limit of URLS per worksheet.
  self.hlink_count = self.hlink_count + 1

  if self.hlink_count > 65530 then
    Utility.warn("Ignoring URL since it exceeds Excel's limit of 65,530 "
                   .. "URLS per worksheet.\n")
    return -4
  end

  -- Write previous row if in in-line string optimization mode.
  if self.optimization == 1 and row > self.previous_row then
    self:_write_single_row(row)
  end

  -- Write the hyperlink string.
  self:write_string(row, col, str, format)

  -- Store the hyperlink data in a separate structure.
  if not self.hyperlinks[row] then self.hyperlinks[row] = {} end

  self.hyperlinks[row][col] = {
    ["link_type"] = link_type,
    ["url"]       = url,
    ["str"]       = url_str or false,
    ["tip"]       = tip     or false,
  }
end

----
-- Check that row and col are valid and store max and min values for use in
-- other methods/elements.
--
-- The ignore_row/ignore_col flags is used to indicate that we wish to
-- perform the dimension check without storing the value.
--
-- The ignore flags are use by set_row() and data_validate.
--
function Worksheet:_check_dimensions(row, col, ignore_row, ignore_col)

  if row >= xl_rowmax or col >= xl_colmax or row < 0 or col < 0 then
    return false
  end

  -- In optimization mode we don't change dimensions for rows that are
  -- already written.
  if self.optimization and not ignore_row and not ignore_col then
    if row < self.previous_row then
      return -2
    end
  end

  if not ignore_row then
    if not self.dim_rowmin or row < self.dim_rowmin then
      self.dim_rowmin = row
    end

    if not self.dim_rowmax or row > self.dim_rowmax then
      self.dim_rowmax = row
    end
  end

  if not ignore_col then
    if not self.dim_colmin or col < self.dim_colmin then
      self.dim_colmin = col
    end

    if not self.dim_colmax or col > self.dim_colmax then
      self.dim_colmax = col
    end
  end

  return true
end

----
-- Set the width of a single column or a range of columns.
--
function Worksheet:_set_column(firstcol, lastcol, width, format, options)

  -- Ensure 2nd col is larger than first.
  if firstcol > lastcol then firstcol, lastcol = lastcol, firstcol end

  -- Set the optional column values.
  options = options or {}
  local hidden    = options["hidden"]    or false
  local collapsed = options["collapsed"] or false
  local level     = options["level"]     or 0

  -- Check that cols are valid and store max and min values with default row.
  -- NOTE: The check shouldn't modify the row dimensions and should only modify
  --       the column dimensions in certain cases.
  local ignore_row = true
  local ignore_col = true

  if format or (width and hidden) then ignore_col = false end

  if not self:_check_dimensions(0, firstcol, ignore_row, ignore_col) then
    return -1
  end

  if not self:_check_dimensions(0, lastcol, ignore_row, ignore_col) then
    return -1
  end

  -- Set the limits for the outline levels (0 <= x <= 7).
  if level < 0 then level = 0  end
  if level > 7 then level = 7  end

  if level > self.outline_col_level then
    self.outline_col_level = level
  end

  -- Store the column data based on the first column.
  self.colinfo[firstcol] = {["firstcol"]  = firstcol,
                            ["lastcol"]   = lastcol,
                            ["width"]     = width,
                            ["format"]    = format,
                            ["hidden"]    = hidden,
                            ["level"]     = level,
                            ["collapsed"] = collapsed}

  -- Store the column change to allow optimisations.
  self.col_size_changed = true

  -- Store the col sizes for use when calculating image vertices taking
  -- hidden columns into account. Also store the column formats.
  if hidden then width = 0 end

  for col = firstcol, lastcol do
    self.col_sizes[col] = width
    if format then
      self.col_formats[col] = format
    end
  end
end

----
-- Convert zero indexed rows and columns to the format required by worksheet
-- named ranges, eg, "Sheet1!A1:C13".
--
function Worksheet:_convert_name_area(row_num_1, col_num_1, row_num_2, col_num_2)
  local range1       = ''
  local range2       = ''
  local row_col_only = false
  local area         = ''

  -- Convert to A1 notation.
  local col_char_1 = Utility.col_to_name_abs(col_num_1, true)
  local col_char_2 = Utility.col_to_name_abs(col_num_2, true)
  local row_char_1 = "$" .. (row_num_1 + 1)
  local row_char_2 = "$" .. (row_num_2 + 1)

  -- We need to handle some special cases that refer to rows or columns only.
  if row_num_1 == 0 and row_num_2 == xl_rowmax - 1 then
    range1       = col_char_1
    range2       = col_char_2
    row_col_only = true
  elseif col_num_1 == 0 and col_num_2 == xl_colmax - 1 then
    range1       = row_char_1
    range2       = row_char_2
    row_col_only = true
  else
    range1 = col_char_1 .. row_char_1
    range2 = col_char_2 .. row_char_2
  end

  -- A repeated range is only written once (if it isn't a special case).
  if range1 == range2 and not row_col_only then
    area = range1
  else
    area = range1 .. ":" .. range2
  end

  -- Build up the print area range "Sheet1!A1:C13".
  local sheetname = self:_quote_sheetname(self.name)
  area = sheetname .. "!" .. area

  return area
end

----
-- Iternal method that is used to filter the pagebreaks tables. It:
--   1. Removes duplicate entries from the table.
--   2. Sorts the list.
--   3. Removes 0 from the table if present.
--
function Worksheet:_sort_pagebreaks(breaks)

  if not breaks then return {} end

  table.sort(breaks)

  if breaks[1] == 0 then
    table.remove(breaks, 1)
  end


  -- The Excel 2007 specification says that the maximum number of page breaks
  -- is 1026. However, in practice it is actually 1023.
  local max_num_breaks = 1023
  if #breaks > max_num_breaks then
    breaks[max_num_breaks + 1] = nil
  end

  -- Remove duplicates.
  local unique_breaks = {}

  for _, row in ipairs(breaks) do
    if row ~= unique_breaks[#unique_breaks] then
      table.insert(unique_breaks, row)
    end
  end

  return unique_breaks
end

----
-- _encode_password(plaintext)
--
-- Based on the algorithm provided by Daniel Rentz of OpenOffice.
--
function Worksheet:_encode_password(plaintext)


  -- Use ZipWriter's import of bit/bit32 on Lua5.1/5.2.
  local ziputils = require "ZipWriter.utils"
  local bit = ziputils.bit

  local password_hash = 0x0000
  local count         = #plaintext
  local i             = 1
  local digits        = {}

  for char in string.gmatch(plaintext, "%a") do
    char = string.byte(char)
    char = bit.lshift(char, i)

    local low_15  = bit.band(char, 0x7FFF)
    local high_15 = bit.band(char, 0x3fff8000)

    char = bit.bor(low_15, high_15)
    digits[i] = char
    i = i + 1
  end

  for i = 1, count do
    password_hash = bit.bxor(password_hash, digits[i])
  end

  password_hash = bit.bxor(password_hash, count)
  password_hash = bit.bxor(password_hash, 0xCE4B)

  return string.format("%X", password_hash)
end


------------------------------------------------------------------------------
--
-- XML writing methods.
--
------------------------------------------------------------------------------

----
-- Write the <Worksheet> element. This is the root element of Worksheet.
--
function Worksheet:_write_worksheet()

  local schema   = "http://schemas.openxmlformats.org/"
  local xmlns    = schema .. "spreadsheetml/2006/main"
  local xmlns_r  = schema .. "officeDocument/2006/relationships"

  local attributes = {
    {["xmlns"]   = xmlns},
    {["xmlns:r"] = xmlns_r},
  }

  self:_xml_start_tag("worksheet", attributes)
end

----
-- Write the <sheetPr> element for Sheet level properties.
--
function Worksheet:_write_sheet_pr()

  local attributes = {}

  if not self.fit_page and not self.filter_on and not self.tab_color
  and not self.outline_changed and not self.vba_codename then
    return
  end

  if self.vba_codename then
     table.insert(attributes, {["codeName"] = self.vba_codename})
  end

  if self.filter_on then
     table.insert(attributes, {["filterMode"] = "1"})
  end


  if self.fit_page or self.tab_color or self.outline_changed then
    self:_xml_start_tag("sheetPr", attributes)
    self:_write_tab_color()
    self:_write_outline_pr()
    self:_write_page_set_up_pr()
    self:_xml_end_tag("sheetPr")
  else
    self:_xml_empty_tag("sheetPr", attributes)
  end
end

----
-- Write the <pageSetUpPr> element.
--
function Worksheet:_write_page_set_up_pr()

  if not self.fit_page then return end

  local attributes = {{["fitToPage"] = "1"}}

  self:_xml_empty_tag("pageSetUpPr", attributes)
end

----
-- Write the <dimension> element. This specifies the range of cells in the
-- Worksheet. As a special case, empty spreadsheets use "A1" as a range.
--
function Worksheet:_write_dimension()
  local ref = ""

  if not self.dim_rowmin and not self.dim_colmin then
    -- If the min dims are undefined then no dimensions have been set
    -- and we use the default "A1".
    ref = "A1"
  elseif not self.dim_rowmin and self.dim_colmin then
    -- If the row dims aren't set but the column dims are then they
    -- have been changed via set_column().
    if self.dim_colmin == self.dim_colmax then
      -- The dimensions are a single cell and not a range.
      ref = Utility.rowcol_to_cell(0, self.dim_colmin)
    else
      -- The dimensions are a cell range.
      ref  = Utility.range(0, self.dim_colmin, 0, self.dim_colmax)
    end
  elseif self.dim_rowmin == self.dim_rowmax
  and self.dim_colmin == self.dim_colmax then
    -- The dimensions are a single cell and not a range.
    ref = Utility.rowcol_to_cell(self.dim_rowmin, self.dim_colmin)
  else
    -- The dimensions are a cell range.
    ref = Utility.range(self.dim_rowmin, self.dim_colmin,
                        self.dim_rowmax, self.dim_colmax)
  end

  self:_xml_empty_tag("dimension", {{["ref"] = ref}})
end


----
-- Write the <sheetViews> element.
--
function Worksheet:_write_sheet_views()


  local attributes = {}

  self:_xml_start_tag("sheetViews", attributes)
  self:_write_sheet_view()
  self:_xml_end_tag("sheetViews")
end

----
-- Write the <sheetView> element.
--
-- Sample structure:
--     <sheetView
--         showGridLines="0"
--         showRowColHeaders="0"
--         showZeros="0"
--         rightToLeft="1"
--         tabSelected="1"
--         showRuler="0"
--         showOutlineSymbols="0"
--         view="pageLayout"
--         zoomScale="121"
--         zoomScaleNormal="121"
--         workbookViewId="0"
--      />
--
function Worksheet:_write_sheet_view()

  local gridlines        = self.screen_gridlines
  local show_zeros       = self.show_zeros
  local right_to_left    = self.right_to_left
  local tab_selected     = self.selected
  local view             = self.page_view
  local zoom             = self.zoom
  local workbook_view_id = 0
  local attributes       = {}

  -- Hide screen gridlines if required
  if not gridlines then
    table.insert(attributes, {["showGridLines"] = "0"})
  end

  -- Hide zeroes in cells.
  if not show_zeros then
    table.insert(attributes, {["showZeros"] = "0"})
  end

  -- Display worksheet right to left for Hebrew, Arabic and others.
  if right_to_left then
    table.insert(attributes, {["rightToLeft"] = "1"})
  end

  -- Show that the sheet tab is selected.
  if tab_selected then
    table.insert(attributes, {["tabSelected"] = "1"})
  end

  -- Turn outlines off. Also required in the outlinePr element.
  if not self.outline_on then
    table.insert(attributes, {["showOutlineSymbols"] = "0"})
  end

  -- Set the page view/layout mode if required.
  if view then
    table.insert(attributes, {["view"] = "pageLayout"})
  end

  -- Set the zoom level.
  if zoom ~= 100 then
    if not view then
      table.insert(attributes, {["zoomScale"] = zoom})
    end
    if self.zoom_scale_normal then
      table.insert(attributes, {["zoomScaleNormal"] = zoom})
    end
  end

  table.insert(attributes, {["workbookViewId"] = workbook_view_id})

  self:_xml_empty_tag("sheetView", attributes)
end

----
-- Write the <pageMargins> element.
--
function Worksheet:_write_page_margins()
  local attributes = {
    {["left"]   = self.margin_left},
    {["right"]  = self.margin_right},
    {["top"]    = self.margin_top},
    {["bottom"] = self.margin_bottom},
    {["header"] = self.margin_header},
    {["footer"] = self.margin_footer},
  }

  self:_xml_empty_tag("pageMargins", attributes)
end

----
-- Write the <pageSetup> element.
--
-- The following is an example taken from Excel.
--
-- <pageSetup
--     paperSize="9"
--     scale="110"
--     fitToWidth="2"
--     fitToHeight="2"
--     pageOrder="overThenDown"
--     orientation="portrait"
--     blackAndWhite="1"
--     draft="1"
--     horizontalDpi="200"
--     verticalDpi="200"
--     r:id="rId1"
-- />
--
function Worksheet:_write_page_setup()

  local attributes = {}

  if not self.page_setup_changed then return end

  -- Set paper size.
  if self.paper_size > 0 then
    table.insert(attributes, {["paperSize"] = self.paper_size})
  end

  -- Set the print_scale
  if self.print_scale ~= 100 then
    table.insert(attributes, {["scale"] = self.print_scale})
  end

  -- Set the "Fit to page" properties.
  if self.fit_page and self.fit_width ~= 1 then
    table.insert(attributes, {["fitToWidth"] = self.fit_width})
  end

  if self.fit_page and self.fit_height ~= 1 then
    table.insert(attributes, {["fitToHeight"] = self.fit_height})
  end

  -- Set the page print direction.
  if self.page_order then
    table.insert(attributes, {["pageOrder"] = "overThenDown"})
  end

  -- Set page orientation.
  if self.orientation then
    table.insert(attributes, {["orientation"] = "portrait"})
  else
    table.insert(attributes, {["orientation"] = "landscape"})
  end

  -- Set start page.
  if self.page_start ~= 0 then
    table.insert(attributes, {["useFirstPageNumber"] = self.page_start})
  end

  self:_xml_empty_tag("pageSetup", attributes)
end

----
-- Write the <printOptions> element.
--
function Worksheet:_write_print_options()

  if not self.print_options_changed then return end

  local attributes = {}

  -- Set horizontal centering.
  if self.hcenter then
    table.insert(attributes, {["horizontalCentered"] = "1"})
  end

  -- Set vertical centering.
  if self.vcenter then
    table.insert(attributes, {["verticalCentered"] = "1"})
  end

  -- Enable row and column headers.
  if self.print_headers then
    table.insert(attributes, {["headings"] = "1"})
  end

  -- Set printed gridlines.
  if self.print_gridlines then
    table.insert(attributes, {["gridLines"] = "1"})
  end

  self:_xml_empty_tag("printOptions", attributes)
end

----
-- Write the <sheetFormatPr> element.
--
function Worksheet:_write_sheet_format_pr()

  local base_col_width     = 10
  local default_row_height = self.default_row_height
  local row_level          = self.outline_row_level
  local col_level          = self.outline_col_level
  local zero_height        = self.default_row_zeroed

  local attributes = {{["defaultRowHeight"] = default_row_height}}

  if self.default_row_height ~= 15 then
    table.insert(attributes, {["customHeight"] = "1"})
  end

  if self.default_row_zeroed then
    table.insert(attributes, {["zeroHeight"] = "1"})
  end

  if row_level > 0 then
    table.insert(attributes, {["outlineLevelRow"] = row_level})
  end

  if col_level > 0 then
    table.insert(attributes, {["outlineLevelCol"] = col_level})
  end

  if self.excel_version == 2010 then
    table.insert(attributes, {["x14ac:dyDescent"] = "0.25"})
  end

  self:_xml_empty_tag("sheetFormatPr", attributes)
end

----
-- Write the <sheetData> element.
--
function Worksheet:_write_sheet_data()
  if not self.dim_rowmin then
    -- If the dimensions aren't defined then there is no data to write.
    self:_xml_empty_tag("sheetData")
  else
    self:_xml_start_tag("sheetData")
    self:_write_rows()
    self:_xml_end_tag("sheetData")
  end
end

----
-- Write the <sheetData> element when the memory optimisation is on. In which
-- case we read the data stored in the temp file and rewrite it to the XML
-- sheet file.
--
function Worksheet:_write_optimized_sheet_data()

  if not self.dim_rowmin then
    -- If the dimensions aren't defined then there is no data to write.
    self:_xml_empty_tag("sheetData")
  else
    self:_xml_start_tag("sheetData")

    -- Rewind the temp file with the row data.
    self.row_data_fh:seek('set', 0)
    local buffer = self.row_data_fh:read(4096)

    while buffer do
      self.fh:write(buffer)
      buffer = self.row_data_fh:read(4096)
    end

    self:_xml_end_tag("sheetData")
  end
end

----
-- Write out the worksheet data as a series of rows and cells.
--
function Worksheet:_write_rows()
  -- Calculate the row span attributes.
  self:_calculate_spans()

  for row_num = self.dim_rowmin, self.dim_rowmax do

    -- Only write rows if they contain row formatting, cell data or a comment.
    if self.set_rows[row_num] or self.data_table[row_num] or self.comments[row_num] then

      local span_index = math.floor(row_num / 16)
      local span       = self.row_spans[span_index]

      -- Write the cells if the row contains data.
      if self.data_table[row_num] then

        if not self.set_rows[row_num] then
          self:_write_row(row_num, span)
        else
          self:_write_row(row_num, span, unpack(self.set_rows[row_num]))
        end

        for col_num = self.dim_colmin, self.dim_colmax do
          if self.data_table[row_num][col_num] then
            self:_write_cell(row_num, col_num, self.data_table[row_num][col_num])
          end
        end

        self:_xml_end_tag("row")

      elseif self.comments[row_num] then
        self:_write_empty_row(row_num, span, unpack(self.set_rows[row_num]))
      else
        -- Row attributes only.
        self:_write_empty_row(row_num, span, unpack(self.set_rows[row_num]))
      end
    end
  end
end

----
-- Write out the worksheet data as a single row with cells. This method is
-- used when memory optimisation is on. A single row is written and the data
-- table is reset. That way only one row of data is kept in memory at any one
-- time. We don't write span data in the optimised case since it is optional.
--
function Worksheet:_write_single_row(current_row)

  local row_num = self.previous_row

  -- Set the new previous row as the current row.
  self.previous_row = current_row

  -- Only write rows if they contain row formatting, cell data or a comment.
  if self.set_rows[row_num] or self.data_table[row_num] or self.comments[row_num] then

    -- Write the cells if the row contains data.
    if self.data_table[row_num] then

      if not self.set_rows[row_num] then
        self:_write_row(row_num)
      else
        self:_write_row(row_num, nil, unpack(self.set_rows[row_num]))
      end

      for col_num = self.dim_colmin, self.dim_colmax do
        if self.data_table[row_num][col_num] then
          self:_write_cell(row_num, col_num, self.data_table[row_num][col_num])
        end
      end

      self:_xml_end_tag("row")
    else
      -- Row attributes or comments only.
      self:_write_empty_row(row_num, nil, unpack(self.set_rows[row_num]))
    end

    -- Reset table.
    self.data_table = {}
  end
end


----
-- Calculate the "spans" attribute of the <row> tag. This is an XLSX
-- optimisation and isn't strictly required. However, it makes comparing
-- files easier.
--
-- The span is the same for each block of 16 rows.
--
function Worksheet:_calculate_spans()
  local spans = {}
  local span_min
  local span_max

  for row_num = self.dim_rowmin, self.dim_rowmax do
    -- Calculate spans for cell data.
    if self.data_table[row_num] then
      for col_num = self.dim_colmin, self.dim_colmax do
        if self.data_table[row_num][col_num] then
          if not span_min then
            span_min = col_num
            span_max = col_num
          else
            if col_num < span_min then
              span_min = col_num
            end
            if col_num > span_max then
              span_max = col_num
            end
          end
        end
      end
    end

    -- Calculate spans for comments.
    if self.comments[row_num] then
      for col_num = self.dim_colmin, self.dim_colmax do
        if self.comments[row_num][col_num] then
          if not span_min then
            span_min = col_num
            span_max = col_num
          else
            if col_num < span_min then
              span_min = col_num
            end
            if col_num > span_max then
              span_max = col_num
            end
          end
        end
      end
    end

    if (row_num + 1) % 16 == 0  or row_num == self.dim_rowmax then
      local span_index = math.floor(row_num / 16)

      if span_min then
        span_min = span_min + 1
        span_max = span_max + 1
        spans[span_index] = string.format("%d:%d", span_min, span_max)

        span_min = nil
        span_max = nil
      end
    end
  end

  self.row_spans = spans
end

----
-- Write the <row> element.
--
function Worksheet:_write_row(r, spans, height, format,
                              hidden, level, collapsed, empty_row)

  local xf_index = 0

  level = level or 0

  if not height then
    height = self.default_row_height
  end

  local attributes = {{["r"] = math.modf(r + 1)}}

  -- Get the format index.
  if format then
    xf_index = format:_get_xf_index()
  end

  if spans then
    attributes[#attributes + 1] = {["spans"] = spans}
  end

  if xf_index > 0 then
    attributes[#attributes + 1] = {["s"] = xf_index}
  end

  if format then
    attributes[#attributes + 1] = {["customFormat"] = "1"}
  end

  if height ~= 15 then
    attributes[#attributes + 1] = {["ht"] = height}
  end

  if hidden then
    attributes[#attributes + 1] = {["hidden"] = "1"}
  end

  if height ~= 15 then
    attributes[#attributes + 1] = {["customHeight"] = 1}
  end

  if level > 0 then
    attributes[#attributes + 1] = {["outlineLevel"] = level}
  end

  if collapsed then
    attributes[#attributes + 1] = {["collapsed"]    = "1"}
  end

  if self.excel_version == 2010 then
    attributes[#attributes + 1] = {["x14ac:dyDescent"] = "0.25"}
  end

  if empty_row then
    self:_xml_empty_tag_unencoded("row", attributes)
  else
    self:_xml_start_tag_unencoded("row", attributes)
  end
end

----
-- Write and empty <row> element, i.e., attributes only, no cell data.
--
function Worksheet:_write_empty_row(r, spans, height, format, hidden, level, collapsed)
  -- Set the $empty_row parameter.
  local empty_row = 1
  self:_write_row(r, spans, height, format, hidden, level, collapsed, empty_row)
end


----
-- Write the <cell> element. This is the innermost loop so efficiency is
-- important where possible. The basic methodology is that the data of every
-- cell type is passed in as follows:
--
--      (row, col, cell)
--
-- The cell is a table containing the following structure in all types:
--
--     {cell_type, token, xf, args}
--
-- Where cell_type: represents the cell type, such as string, number, formula.
--       token:     is the actual data for the string, number, formula, etc.
--       xf:        is the XF format object.
--       args:      additional args relevant to the specific data type.
--
function Worksheet:_write_cell(row, col, cell)

  local cell_type = cell[1]
  local token     = cell[2]
  local format    = cell[3]
  local xf_index  = 0

  -- Get the format index.
  if format then
    xf_index = format:_get_xf_index()
  end


  local range = Utility.rowcol_to_cell(row, col)
  local attributes = {{["r"] = range}}

  -- Add the cell format index.
  if xf_index > 0 then

    attributes[#attributes + 1] = {["s"] = xf_index}

  elseif self.set_rows[row] and self.set_rows[row][2] then

    local row_xf = self.set_rows[row][2]
    attributes[#attributes + 1] = {["s"] = row_xf:_get_xf_index()}

  elseif self.col_formats[col] then

    local col_xf = self.col_formats[col]
    attributes[#attributes + 1] = {["s"] = col_xf:_get_xf_index()}
  end

  -- Write the various cell types.
  if cell_type == "n" then
    -- Write a number.
    self:_xml_number_element(token, attributes)

  elseif cell_type == "s" then
    -- Write a string.
    if not self.optimization then
      self:_xml_string_element(token, attributes)
    else
      local str = token
      -- Escape control characters. See SharedString.pm for details.
      --str =~ s/(_x[0-9a-fA-F]{4}_)/_x005F1/g
      --str =~ s/([\x00-\x08\x0B-\x1F])/sprintf "_x04X_", ord(1)/eg

      -- Write any rich strings without further tags.
      -- if str =~ m{^<r>} and str =~ m{</r>$} then
      --   self:_xml_rich_inline_string(str, attributes)
      -- else

      -- Add attribute to preserve leading or trailing whitespace.
      local preserve = false
      if string.match(str, "^%s") or string.match(str, "%s$") then
        preserve = true
      end
      self:_xml_inline_string(str, preserve, attributes)
    end

  elseif cell_type == "f" then

    -- Write a formula.
    local value = cell[4] or 0

    -- Check if the formula value is a string.
    if type(value) == "string" then
      attributes[#attributes + 1] = {["t"] = "str"}
    end

    self:_xml_formula_element(token, value, attributes)

  elseif cell_type == "a" then

    -- Write an array formula.
    self:_xml_start_tag("c", attributes)
    self:_write_cell_array_formula(token, cell[5])
    self:_write_cell_value(cell[4])
    self:_xml_end_tag("c")

  elseif cell_type == "b" then

    -- Write a empty cell.
    self:_xml_empty_tag("c", attributes)

  elseif cell_type == "l" then

    -- Write a boolean value.
      attributes[#attributes + 1] = {["t"] = "b"}
    self:_xml_start_tag("c", attributes)
    self:_write_cell_value(token)
    self:_xml_end_tag("c")

  end
end

----
-- Write the cell value <v> element.
--
function Worksheet:_write_cell_value(value)
  self:_xml_data_element("v", value or '')
end

----
-- Write the cell formula <f> element.
--
function Worksheet:_write_cell_formula(formula)
  self:_xml_data_element("f", formula or '')
end

----
-- Write the cell array formula <f> element.
--
function Worksheet:_write_cell_array_formula(formula, range)
  local attributes = {{["t"] = "array"}, {["ref"] = range}}
  self:_xml_data_element("f", formula, attributes)
end

----
-- Write the <cols> element and <col> sub elements.
--
function Worksheet:_write_cols()

  -- Return unless some column have been formatted.
  if not self.col_size_changed then return end

  self:_xml_start_tag("cols")

  for col, colinfo in Utility.sorted_pairs(self.colinfo) do
    self:_write_col_info(self.colinfo[col])
  end

  self:_xml_end_tag("cols")
end

----
-- Write the <col> element.
--
function Worksheet:_write_col_info(colinfo)

  local firstcol     = colinfo["firstcol"]
  local lastcol      = colinfo["lastcol"]
  local width        = colinfo["width"]
  local format       = colinfo["format"]
  local hidden       = colinfo["hidden"]
  local level        = colinfo["level"]
  local collapsed    = colinfo["collapsed"]
  local custom_width = true
  local xf_index     = 0

  -- Get the format index.
  if format then
    xf_index = format:_get_xf_index()
  end


  -- Set the Excel default col width.
  if not width then
    if not hidden then
      width        = 8.43
      custom_width = false
    else
      width = 0
    end
  else

    -- Width is defined but same as default.
    if width == 8.43 then
      custom_width = false
    end
  end

  -- Convert column width from user units to character width.
  local max_digit_width = 7    -- For Calabri 11.
  local padding         = 5

  if width > 0 then
    if width < 1 then
      width = math.floor((math.floor(width*(max_digit_width + padding) + 0.5))
                           / max_digit_width * 256) / 256
    else
      width = math.floor((math.floor(width*max_digit_width + 0.5) + padding)
                           / max_digit_width * 256) / 256
    end
  end

  local attributes = {
    {["min"]   = math.modf(firstcol + 1)},
    {["max"]   = math.modf(lastcol  + 1)},
    {["width"] = width},
  }

  if xf_index > 0 then
    table.insert(attributes, {["style"] = xf_index})
  end

  if hidden then
    table.insert(attributes, {["hidden"] = "1"})
  end

  if custom_width then
    table.insert(attributes, {["customWidth"] = "1"})
  end

  if level > 0 then
    table.insert(attributes, {["outlineLevel"] = level})
  end

  if collapsed then
    table.insert(attributes, {["collapsed"] = "1"})
  end

  self:_xml_empty_tag("col", attributes)
end

----
-- Write the <tabColor> element.
--
function Worksheet:_write_tab_color()

  local color_index = self.tab_color

  if not color_index then return end

  local rgb        = Utility.excel_color(color_index)
  local attributes = {{["rgb"] = rgb}}

  self:_xml_empty_tag("tabColor", attributes)
end

----
-- Write the <outlinePr> element.
--
function Worksheet:_write_outline_pr()

  local attributes = {}
  if not self.outline_changed then return end

  if self.outline_style > 0 then
     table.insert(attributes, {["applyStyles"] = "1"})
  end

  if not self.outline_below then
     table.insert(attributes, {["summaryBelow"] = "0"})
  end

  if not self.outline_right then
     table.insert(attributes, {["summaryRight"] = "0"})
  end

  if not self.outline_on then
     table.insert(attributes, {["showOutlineSymbols"] = "0"})
  end

  self:_xml_empty_tag("outlinePr", attributes)
end

----
-- Write the <sheetProtection> element.
--
function Worksheet:_write_sheet_protection()

  local attributes = {}

  if not self.protect_options then
     return
  end

  local options = self.protect_options

  if options.password then
    table.insert(attributes, {["password"] = options.password})
  end

  if options.sheet then
     table.insert(attributes, {["sheet"] = "1"})
  end

  if options.content then
     table.insert(attributes, {["content"] = "1"})
  end

  if not options.objects then
     table.insert(attributes, {["objects"] = "1"})
  end

  if not options.scenarios then
     table.insert(attributes, {["scenarios"] = "1"})
  end

  if options.format_cells then
     table.insert(attributes, {["formatCells"] = "0"})
  end

  if options.format_columns then
     table.insert(attributes, {["formatColumns"] = "0"})
  end

  if options.format_rows then
     table.insert(attributes, {["formatRows"] = "0"})
  end

  if options.insert_columns then
     table.insert(attributes, {["insertColumns"] = "0"})
  end

  if options.insert_rows then
     table.insert(attributes, {["insertRows"] = "0"})
  end

  if options.insert_hyperlinks then
     table.insert(attributes, {["insertHyperlinks"] = "0"})
  end

  if options.delete_columns then
     table.insert(attributes, {["deleteColumns"] = "0"})
  end

  if options.delete_rows then
     table.insert(attributes, {["deleteRows"] = "0"})
  end

  if not options.select_locked_cells then
    table.insert(attributes, {["selectLockedCells"] = "1"})
  end

  if options.sort then
     table.insert(attributes, {["sort"] = "0"})
  end

  if options.autofilter then
     table.insert(attributes, {["autoFilter"] = "0"})
  end

  if options.pivot_tables then
     table.insert(attributes, {["pivotTables"] = "0"})
  end

  if not options.select_unlocked_cells then
    table.insert(attributes, {["selectUnlockedCells"] = "1"})
  end

  self:_xml_empty_tag("sheetProtection", attributes)
end


----
-- Write the <headerFooter> element.
--
function Worksheet:_write_header_footer()

  if not self.header_footer_changed then return end

  self:_xml_start_tag("headerFooter")

  if self.header ~= "" then
     self:_write_odd_header()
  end

  if self.footer ~= "" then
     self:_write_odd_footer()
  end

  self:_xml_end_tag("headerFooter")
end

----
-- Write the <oddHeader> element.
--
function Worksheet:_write_odd_header()
  local data = self.header

  self:_xml_data_element("oddHeader", data)
end

----
-- Write the <oddFooter> element.
--
function Worksheet:_write_odd_footer()
  local data = self.footer

  self:_xml_data_element("oddFooter", data)
end

----
-- Write the <rowBreaks> element.
--
function Worksheet:_write_row_breaks()

  local page_breaks = self:_sort_pagebreaks(self.hbreaks)
  local count       = #page_breaks

  if count == 0 then return end

  local attributes = {
    {["count"]            = count},
    {["manualBreakCount"] = count},
  }

  self:_xml_start_tag("rowBreaks", attributes)

  for _, row_num in ipairs(page_breaks) do
    self:_write_brk(row_num, 16383)
  end

  self:_xml_end_tag("rowBreaks")
end

----
-- Write the <colBreaks> element.
--
function Worksheet:_write_col_breaks()

  local page_breaks = self:_sort_pagebreaks(self.vbreaks)
  local count       = #page_breaks

  if count == 0 then return end

  local attributes = {
    {["count"]            = count},
    {["manualBreakCount"] = count},
  }

  self:_xml_start_tag("colBreaks", attributes)

  for _, col_num in ipairs(page_breaks) do
    self:_write_brk(col_num, 1048575)
  end

  self:_xml_end_tag("colBreaks")
end

----
-- Write the <brk> element.
--
function Worksheet:_write_brk(id, max)

  local attributes = {
    {["id"]  = id},
    {["max"] = max},
    {["man"] = "1"},
  }

  self:_xml_empty_tag("brk", attributes)
end

----
-- Write the <mergeCells> element.
--
function Worksheet:_write_merge_cells()

  local merged_cells = self.merge
  local count        = #merged_cells

  if count == 0 then return end

  local attributes = {{["count"] = count}}

  self:_xml_start_tag("mergeCells", attributes)

  for _, merged_range in ipairs(merged_cells) do
    -- Write the mergeCell element.
    self:_write_merge_cell(merged_range)
  end

  self:_xml_end_tag("mergeCells")
end

----
-- Write the <mergeCell> element.
--
function Worksheet:_write_merge_cell(merged_range)

  local row_min, col_min, row_max, col_max = unpack(merged_range)

  -- Convert the merge dimensions to a cell range.
  local cell_1 = Utility.rowcol_to_cell(row_min, col_min)
  local cell_2 = Utility.rowcol_to_cell(row_max, col_max)
  local ref    = cell_1 .. ":" .. cell_2

  local attributes = {{["ref"] = ref}}

  self:_xml_empty_tag("mergeCell", attributes)
end

----
-- Process any stored hyperlinks in row/col order and write the <hyperlinks>
-- element. The attributes are different for internal and external links.
--
function Worksheet:_write_hyperlinks()

  local hlink_refs = {}
  local has_hyperlinks = false

  -- Iterate over the rows and cols in sorted order.
  for row_num in Utility.sorted_keys(self.hyperlinks) do

    for col_num in Utility.sorted_keys(self.hyperlinks[row_num]) do

      -- Get the link data for this cell.
      local link      = self.hyperlinks[row_num][col_num]
      local link_type = link.link_type

      -- If the cell isn't a string then we have to add the url as
      -- the string to display.
      local display = false
      if self.data_table[row_num] and self.data_table[row_num][col_num] then
        local cell = self.data_table[row_num][col_num]
        if cell[1] ~= "s" then
          display = link.url
        end
      end

      if link_type == 1 then
        -- External link with rel file relationship.
        self.rel_count = self.rel_count + 1
        table.insert(hlink_refs, {link_type, row_num, col_num, self.rel_count,
                                  link.str, display, link.tip})

        -- Links for use by the packager.
        table.insert(self.external_hyper_links,
                     {"/hyperlink", link.url, 'External'})
      else
        -- Internal link with rel file relationship.
        table.insert(hlink_refs, {link_type, row_num, col_num, link.url,
                                  link.str, link.tip})
      end
    end
    has_hyperlinks = true
  end


  if not has_hyperlinks then return end

  -- Write the hyperlink elements.
  self:_xml_start_tag("hyperlinks")

  for _, hlink_data in ipairs(hlink_refs) do
    local link_type = table.remove(hlink_data, 1)

    if link_type == 1 then
      self:_write_hyperlink_external(unpack(hlink_data))
    elseif link_type == 2 then
      self:_write_hyperlink_internal(unpack(hlink_data))
    end
  end

  self:_xml_end_tag("hyperlinks")
end

----
-- Write the <hyperlink> element for external links.
--
function Worksheet:_write_hyperlink_external(row, col, id, location, display, tooltip)

  local ref = Utility.rowcol_to_cell(row, col)
  local r_id = "rId" .. id

  local attributes = {
    {["ref"]  = ref},
    {["r:id"] = r_id},
  }

  if location then
    table.insert(attributes, {["location"] = location})
  end

  if display then
    table.insert(attributes, {["display"]  = display})
  end

  if tooltip then
    table.insert(attributes, {["tooltip"]  = tooltip})
  end


  self:_xml_empty_tag("hyperlink", attributes)
end

----
-- Write the <hyperlink> element for internal links.
--
function Worksheet:_write_hyperlink_internal(row, col, location, display, tooltip)


  local ref = Utility.rowcol_to_cell(row, col)

  local attributes = {
    {["ref"]      = ref},
    {["location"] = location},
  }

  if tooltip then table.insert(attributes, {["tooltip"] = tooltip}) end

  table.insert(attributes, {["display"] = display})

  self:_xml_empty_tag("hyperlink", attributes)
end


return Worksheet
