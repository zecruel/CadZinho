----
-- Contenttypes - A class for writing the Excel XLSX Contenttypes file.
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

local app_package  = "application/vnd.openxmlformats-package."
local app_document = "application/vnd.openxmlformats-officedocument."

-- The constructor inherits from xmlwriter.lua.
local Contenttypes = {}
setmetatable(Contenttypes,{__index = Xmlwriter})

function Contenttypes:new()

  local instance = {
    defaults  = {{"rels", app_package .. "relationships+xml" },
                 {"xml", "application/xml"}},
    overrides = {
      {"/docProps/app.xml",    app_document .. "extended-properties+xml"},
      {"/docProps/core.xml",   app_package  .. "core-properties+xml"},
      {"/xl/styles.xml",       app_document .. "spreadsheetml.styles+xml"},
      {"/xl/theme/theme1.xml", app_document .. "theme+xml"},
      {"/xl/workbook.xml",     app_document .. "spreadsheetml.sheet.main+xml"}}
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
function Contenttypes:_assemble_xml_file()

  self:_xml_declaration()
  self:_write_types()
  self:_write_defaults()
  self:_write_overrides()

  self:_xml_end_tag("Types")

  -- Close the XML writer filehandle.
  self:_xml_close()

end

----
-- Add elements to the ContentTypes defaults.
--
function Contenttypes:_add_default(part_name, content_type)
  table.insert(self.defaults, {part_name, content_type})
end

----
-- Add elements to the ContentTypes overrides.
--
function Contenttypes:_add_override(part_name, content_type)
  table.insert(self.overrides, {part_name, content_type})
end

----
-- Add the name of a worksheet to the ContentTypes overrides.
--
function Contenttypes:_add_worksheet_name(worksheet_name)
  worksheet_name = "/xl/worksheets/" .. worksheet_name ..".xml"

  self:_add_override(worksheet_name,
                     app_document .. "spreadsheetml.worksheet+xml")
end

----
-- Add the name of a chartsheet to the ContentTypes overrides.
--
function Contenttypes:_add_chartsheet_name(chartsheet_name)
  chartsheet_name = "/xl/chartsheets/" .. chartsheet_name .. ".xml"

  self:_add_override(chartsheet_name,
                     app_document .. "spreadsheetml.chartsheet+xml")
end

----
-- Add the name of a chart to the ContentTypes overrides.
--
function Contenttypes:_add_chart_name(chart_name)
  chart_name = "/xl/charts/" .. chart_name ..".xml"

  self:_add_override(chart_name, app_document .. "drawingml.chart+xml")
end

----
-- Add the name of a drawing to the ContentTypes overrides.
--
function Contenttypes:_add_drawing_name(drawing_name)
  drawing_name = "/xl/drawings/" .. drawing_name .. ".xml"

  self:_add_override(drawing_name, app_document .. "drawing+xml")
end

----
-- Add the name of a VML drawing to the ContentTypes defaults.
--
function Contenttypes:_add_vml_name()
  self:_add_default("vml", app_document .. "vmlDrawing")
end

----
-- Add the name of a comment to the ContentTypes overrides.
--
function Contenttypes:_add_comment_name(comment_name)
  comment_name = "/xl/" .. comment_name .. ".xml"

  self:_add_override(comment_name, app_document .. "spreadsheetml.comments+xml")
end

----
-- Add the sharedStrings link to the ContentTypes overrides.
--
function Contenttypes:_add_shared_strings()

  self:_add_override("/xl/sharedStrings.xml",
                     app_document .. "spreadsheetml.sharedStrings+xml")
end

----
-- Add the calcChain link to the ContentTypes overrides.
--
function Contenttypes:_add_calc_chain()
  self:_add_override("/xl/calcChain.xml",
                     app_document .. "spreadsheetml.calcChain+xml")
end

----
-- Add the image default types.
--
function Contenttypes:_add_image_types(types)
  for _, type in ipairs(types) do
    self:_add_default(type, "image/" .. type)
  end
end

----
-- Add the name of a table to the ContentTypes overrides.
--
function Contenttypes:_add_table_name(table_name)
  table_name = "/xl/tables/" .. table_name .. ".xml"

  self:_add_override(table_name, app_document .. "spreadsheetml.table+xml")
end

------------------------------------------------------------------------------
--
-- Internal methods.
--
------------------------------------------------------------------------------

----
-- Write out all of the <Default> types.
--
function Contenttypes:_write_defaults()
  for _, default in ipairs(self.defaults) do
    self:_xml_empty_tag("Default", {{["Extension"]   = default[1]},
                                    {["ContentType"] = default[2]}})
  end
end

----
-- Write out all of the <Override> types.
--
function Contenttypes:_write_overrides()
  for _, override in ipairs(self.overrides) do
    self:_xml_empty_tag("Override", {{["PartName"]    = override[1]},
                                     {["ContentType"] = override[2]}})
  end
end

------------------------------------------------------------------------------
--
-- XML writing methods.
--
------------------------------------------------------------------------------

----
-- Write the <Types> element.
--
function Contenttypes:_write_types()
  local xmlns = "http://schemas.openxmlformats.org/package/2006/content-types"
  local attributes = {{["xmlns"] = xmlns}}

  self:_xml_start_tag("Types", attributes)
end

----
-- Write the <Default> element.
--
function Contenttypes:_write_default(extension, content_type)

  local attributes = {
    {["Extension"]   = extension},
    {["ContentType"] = content_type},
  }

  self:_xml_empty_tag("Default", attributes)
end

----
-- Write the <Override> element.
--
function Contenttypes:_write_override(part_name, content_type)

  local attributes = {
    {["PartName"]    = part_name},
    {["ContentType"] = content_type},
  }

  self:_xml_empty_tag("Override", attributes)
end

return Contenttypes
