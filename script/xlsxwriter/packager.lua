----
-- Packager - A class for creating the Excel XLSX file.
--
-- This module is used in conjunction with xlsxwriter.lua to create
-- an Excel XLSX container file.
-- 
-- From Wikipedia: The Open Packaging Conventions (OPC) is a
-- container-file technology initially created by Microsoft to store
-- a combination of XML and non-XML files that together form a single
-- entity such as an Open XML Paper Specification (OpenXPS)
-- document. http://en.wikipedia.org/wiki/Open_Packaging_Conventions.
-- 
-- At its simplest an Excel XLSX file contains the following elements:
-- 
--      ____ [Content_Types].xml
--     |
--     |____ docProps
--     | |____ app.xml
--     | |____ core.xml
--     |
--     |____ xl
--     | |____ workbook.xml
--     | |____ worksheets
--     | | |____ sheet1.xml
--     | |
--     | |____ styles.xml
--     | |
--     | |____ theme
--     | | |____ theme1.xml
--     | |
--     | |_____rels
--     | |____ workbook.xml.rels
--     |
--     |_____rels
--       |____ .rels
-- 
-- The Packager class coordinates the classes that represent the
-- elements of the package and writes them into the XLSX file.
--
-- Copyright 2014-2015, John McNamara, jmcnamara@cpan.org
--

-- **** Modified in aug/2021 by ZeCruel, to use in Cadzinho

require "xlsxwriter.strict"

--[[ -- **** Modified in aug/2021 by ZeCruel, to use in Cadzinho
local ZipWriter     = require "ZipWriter" ]]--
local App           = require "xlsxwriter.app"
local Core          = require "xlsxwriter.core"
local ContentTypes  = require "xlsxwriter.contenttypes"
local Styles        = require "xlsxwriter.styles"
local Theme         = require "xlsxwriter.theme"
local Relationships = require "xlsxwriter.relationships"

------------------------------------------------------------------------------
--
-- Constructor.
--
------------------------------------------------------------------------------

local Packager = {}

function Packager:new(filename)
  local instance = {
    filename         = filename,
    workbook         = false,
    sheet_names      = {},
    worksheet_count  = 0,
    chartsheet_count = 0,
    chart_count      = 0,
    drawing_count    = 0,
    table_count      = 0,
    named_ranges     = {},
    file_descriptor  = {
      istext   = true,
      isfile   = true,
      isdir    = false,
      exattrib = 0x81800020,
      platform = 'unix'},
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
-- Add the workbook object to the package.
--
function Packager:_add_workbook(workbook)

  local sheet_names = workbook.sheetnames

  self.workbook          = workbook
  self.sheet_names       = workbook.sheet_names
  self.chart_count       = #workbook.charts
  self.drawing_count     = #workbook.drawings
  self.num_vml_files     = workbook.num_vml_files
  self.num_comment_files = workbook.num_comment_files
  self.named_ranges      = workbook.named_ranges

  for _, worksheet in ipairs(self.workbook:worksheets()) do
    if worksheet.is_chartsheet then
      self.chartsheet_count = self.chartsheet_count + 1
    else
      self.worksheet_count  = self.worksheet_count  + 1
    end
  end
end

----
-- Write the xml files that make up the XLXS OPC package.
--
function Packager:_create_package()
--[[ -- **** Modified in aug/2021 by ZeCruel, to use in Cadzinho
  self.zip = ZipWriter.new()
  self.zip:open_stream(assert(io.open(self.filename, 'w+b')), true)
]]--
  self:_write_worksheet_files()
  self:_write_chartsheet_files()
  self:_write_workbook_file()
  self:_write_chart_files()
  self:_write_drawing_files()
  self:_write_vml_files()
  self:_write_comment_files()
  self:_write_table_files()
  self:_write_shared_strings_file()
  self:_write_app_file()
  self:_write_core_file()
  self:_write_content_types_file()
  self:_write_styles_file()
  self:_write_theme_file()
  self:_write_root_rels_file()
  self:_write_workbook_rels_file()
  self:_write_worksheet_rels_files()
  self:_write_chartsheet_rels_files()
  self:_write_drawing_rels_files()
  self:_add_image_files()
--[[ -- **** Modified in aug/2021 by ZeCruel, to use in Cadzinho
  self.zip:close() ]]--
end

------------------------------------------------------------------------------
--
-- Internal methods.
--
------------------------------------------------------------------------------

----
-- Add the components to the zip file.
--
function Packager:_add_to_zip(writer)

  writer:_set_filehandle(io.tmpfile())
  writer:_assemble_xml_file()
--[[ -- **** Modified in aug/2021 by ZeCruel, to use in Cadzinho
  self.zip:write(writer.filename, 
                 self.file_descriptor,
                 writer:_get_xml_reader()) ]]--
miniz.write(self.filename, writer.filename, writer:_get_data()) 
-- ***** end modification
end

----
-- Write the workbook.xml file.
--
function Packager:_write_workbook_file()

  local dir      = self.package_dir
  local workbook = self.workbook

  workbook:_set_filename("xl/workbook.xml")
  self:_add_to_zip(workbook)
end

----
-- Write the worksheet files.
--
function Packager:_write_worksheet_files()

  local index = 1
  for _, worksheet in ipairs(self.workbook:worksheets()) do
    if not worksheet.is_chartsheet then
      -- Flush the row data in optimisation mode.
      if worksheet.optimization then worksheet:_write_single_row() end

      worksheet:_set_filename("xl/worksheets/sheet" .. index .. '.xml')
      self:_add_to_zip(worksheet)
      index = index + 1
    end
  end
end

----
-- Write the chartsheet files.
--
function Packager:_write_chartsheet_files()

  local index = 1
  for _, worksheet in ipairs(self.workbook:worksheets()) do
    if worksheet.is_chartsheet then
      worksheet:_set_filename("xl/chartsheets/sheet" .. index .. '.xml')
      self:_add_to_zip(worksheet)
      index = index + 1
    end
  end
end

----
-- Write the chart files.
--
function Packager:_write_chart_files()

  if not self.workbook.charts then return end

  local index = 1
  for _, chart in ipairs(self.workbook.charts) do
    chart:_set_filename("xl/charts/chart" .. index .. '.xml')
    self:_add_to_zip(chart)
    index = index + 1
  end
end

----
-- Write the drawing files.
--
function Packager:_write_drawing_files()

  if not self.drawing_count then return end

  local index = 1
  for _, drawing in ipairs(self.workbook.drawings) do
    drawing:_set_filename("xl/drawings/drawing" .. index .. '.xml')
    self:_add_to_zip(drawing)
    index = index + 1
  end
end

----
-- Write the comment VML files.
--
function Packager:_write_vml_files()

  local index = 1
  for _, worksheet in ipairs(self.workbook:worksheets()) do
    if worksheet.has_vml then
      -- local vml = Vml:new()

      -- vml:_set_filename("xl/drawings/vmlDrawing" .. index .. '.vml')
      -- vml:_assemble_xml_file(worksheet.vml_data_id, 
      --                        worksheet.vml_shape_id, 
      --                        worksheet.comments_array, 
      --                        worksheet.buttons_array)
      index = index + 1
    end
  end
end

----
-- Write the comment files.
--
function Packager:_write_comment_files()

  local index = 1
  for _, worksheet in ipairs(self.workbook:worksheets()) do
    if not worksheet.has_comments then
      -- local comment = Comments:new()

      -- comment:_set_filename("xl/comments" .. index .. '.xml')
      -- comment:_assemble_xml_file(worksheet.comments_array)
      index = index + 1
    end
  end
end

----
-- Write the sharedStrings.xml file.
--
function Packager:_write_shared_strings_file()
  if self.workbook.str_table.string_count > 0 then
    local sst  = self.workbook.str_table
    sst:_set_filename("xl/sharedStrings.xml")
    self:_add_to_zip(sst)
  end
end

----
-- Write the app.xml file.
--
function Packager:_write_app_file()
  local properties = self.workbook.doc_properties
  local app        = App:new()

  -- Add the Worksheet heading pairs.
  app:_add_heading_pair("Worksheets", self.worksheet_count)

  -- Add the Chartsheet heading pairs.
  app:_add_heading_pair("Charts", self.chartsheet_count)

  -- Add the Worksheet parts.
  for _, worksheet in ipairs(self.workbook:worksheets()) do
    if not worksheet.is_chartsheet then
      app:_add_part_name(worksheet:get_name())
    end
  end

  -- Add the Chartsheet parts.
  for _, worksheet in ipairs(self.workbook:worksheets()) do
    if worksheet.is_chartsheet then
      app:_add_part_name(worksheet:get_name())
    end
  end

  -- Add the Named Range heading pairs.
  local range_count = #self.named_ranges
  if range_count > 0 then
    app:_add_heading_pair("Named Ranges", range_count)
  end

  -- Add the Named Ranges parts.
  for _, named_range in ipairs(self.named_ranges) do
    app:_add_part_name(named_range)
  end

  app:_set_properties(properties)

  app:_set_filename("docProps/app.xml")
  self:_add_to_zip(app)
end

----
-- Write the core.xml file.
--
function Packager:_write_core_file()

  local properties = self.workbook.doc_properties
  local core       = Core:new()

  core:_set_properties(properties)
  core:_set_filename("docProps/core.xml")
  self:_add_to_zip(core)
end

----
-- Write the ContentTypes.xml file.
--
function Packager:_write_content_types_file()

  local content = ContentTypes:new()

  content:_add_image_types(self.workbook.image_types)

  local worksheet_index  = 1
  local chartsheet_index = 1
  for _, worksheet in ipairs(self.workbook:worksheets()) do
    if worksheet.is_chartsheet then
      content:_add_chartsheet_name("sheet" .. chartsheet_index)
      chartsheet_index = chartsheet_index
    else
      content:_add_worksheet_name("sheet" .. worksheet_index)
      worksheet_index = worksheet_index + 1
    end
  end

  for i = 1, self.chart_count do
    content:_add_chart_name("chart" .. i)
  end

  for i = 1, self.drawing_count do
    content:_add_drawing_name("drawing" .. i)
  end

  if self.num_vml_files > 0 then
    content:_add_vml_name()
  end

  for i = 1, self.table_count do
    content:_add_table_name("table" .. i)
  end

  for i = 1, self.num_comment_files do
    content:_add_comment_name("comments" .. i)
  end

  -- Add the sharedString rel if there is string data in the workbook.
  if self.workbook.str_table.string_count > 0 then
    content:_add_shared_strings()
  end

  -- Add vbaProject if present.
  if self.workbook.vba_project then
    content:_add_vba_project()
  end

  content:_set_filename("[Content_Types].xml")
  self:_add_to_zip(content)
end

----
-- Write the style xml file.
--
function Packager:_write_styles_file()

  local properties = {}
  properties["xf_formats"]       = self.workbook.xf_formats
  properties["font_count"]       = self.workbook.font_count
  properties["num_format_count"] = self.workbook.num_format_count
  properties["border_count"]     = self.workbook.border_count
  properties["fill_count"]       = self.workbook.fill_count
  properties["custom_colors"]    = self.workbook.custom_colors
  properties["dxf_formats"]      = self.workbook.dxf_formats

  local styles = Styles:new()

  styles:_set_style_properties(properties)
  styles:_set_filename("xl/styles.xml")
  self:_add_to_zip(styles)
end

----
-- Write the style xml file.
--
function Packager:_write_theme_file()

  local theme = Theme:new()

  theme:_set_filename("xl/theme/theme1.xml")
  self:_add_to_zip(theme)
end

----
-- Write the table files.
--
function Packager:_write_table_files()

  local index = 1

  for _, worksheet in ipairs(self.workbook:worksheets()) do
    local table_props = worksheet.tables

    if table_props then
      for _, table_props in ipairs(table_props) do
        -- local table = Table:new()

        -- table:_set_filename("xl/tables/table" .. index .. '.xml')
        -- table:_set_properties(table_props)
        -- self:_add_to_zip(table)
        -- self.table_count = table_count + 1
        index = index + 1

      end
    end
  end
end

----
-- Write the _rels/.rels xml file.
--
function Packager:_write_root_rels_file()
  local rels = Relationships:new()

  rels:_add_document_relationship("/officeDocument", 'xl/workbook.xml')
  rels:_add_package_relationship("/metadata/core-properties", 'docProps/core.xml')
  rels:_add_document_relationship("/extended-properties", 'docProps/app.xml')

  rels:_set_filename("_rels/.rels")
  self:_add_to_zip(rels)
end

----
-- Write the _rels/.rels xml file.
--
function Packager:_write_workbook_rels_file()
  local rels = Relationships:new()

  local worksheet_index  = 1
  local chartsheet_index = 1

  for _, worksheet in ipairs(self.workbook:worksheets()) do
    if worksheet.is_chartsheet then
      rels:_add_document_relationship("/chartsheet", 
                                      'chartsheets/sheet' .. chartsheet_index .. '.xml')
      chartsheet_index = chartsheet_index + 1
    else
      rels:_add_document_relationship("/worksheet", 
                                      'worksheets/sheet' .. worksheet_index .. '.xml')
      worksheet_index = worksheet_index + 1
    end
  end

  rels:_add_document_relationship("/theme",  'theme/theme1.xml')
  rels:_add_document_relationship("/styles", 'styles.xml')

  -- Add the sharedString rel if there is string data in the workbook.
  if self.workbook.str_table.string_count > 0 then
    rels:_add_document_relationship("/sharedStrings", 'sharedStrings.xml')
  end

  -- Add vbaProject if present.
  if self.workbook.vba_project then
    rels:_add_ms_package_relationship("/vbaProject", 'vbaProject.bin')
  end

  rels:_set_filename("xl/_rels/workbook.xml.rels")
  self:_add_to_zip(rels)
end

----
-- Write the worksheet .rels files for worksheets that contain links to external
-- data such as hyperlinks or drawings.
--
function Packager:_write_worksheet_rels_files()

  local index = 0
  for _, worksheet in ipairs(self.workbook:worksheets()) do

    if not worksheet.is_chartsheet then
      index = index + 1

      local external_links = #worksheet.external_hyper_links 
        + #worksheet.external_drawing_links 
        + #worksheet.external_vml_links 
        + #worksheet.external_table_links 
        + #worksheet.external_comment_links

      if external_links > 0 then
        local rels = Relationships:new()

        for _, link_data in ipairs(worksheet.external_hyper_links) do
          rels:_add_worksheet_relationship(unpack(link_data))
        end
        for _, link_data in ipairs(worksheet.external_drawing_links) do
          rels:_add_worksheet_relationship(unpack(link_data))
        end
        for _, link_data in ipairs(worksheet.external_vml_links) do
          rels:_add_worksheet_relationship(unpack(link_data))
        end

        for _, link_data in ipairs(worksheet.external_table_links) do
          rels:_add_worksheet_relationship(unpack(link_data))
        end

        for _, link_data in ipairs(worksheet.external_comment_links) do
          rels:_add_worksheet_relationship(unpack(link_data))
        end

        -- Create the .rels file such as /xl/worksheets/_rels/sheet1.xml.rels.
        rels:_set_filename("xl/worksheets/_rels/sheet" .. index .. '.xml.rels')
        self:_add_to_zip(rels)
      end
    end
  end
end

----
-- Write the chartsheet .rels files for links to drawing files.
--
function Packager:_write_chartsheet_rels_files()

  local index = 0
  for _, worksheet in ipairs(self.workbook:worksheets()) do

    if worksheet.is_chartsheet then
      index = index + 1

      local external_links = #worksheet.external_drawing_links

      if external_links > 0 then
        local rels = Relationships:new()

        for _, link_data in ipairs(worksheet.external_drawing_links) do
          rels:_add_worksheet_relationship(unpack(link_data))
        end

        -- Create the .rels file such as /xl/chartsheets/_rels/sheet1.xml.rels.
        rels:_set_filename("xl/chartsheets/_rels/sheet" .. index .. '.xml.rels')
        self:_add_to_zip(rels)
      end
    end
  end
end

----
-- Write the drawing .rels files for worksheets that contain charts or drawings.
--
function Packager:_write_drawing_rels_files()

  local index = 0
  for _, worksheet in ipairs(self.workbook:worksheets()) do

    if worksheet.drawing_links or worksheet.has_shapes then
      index = index + 1
    end

    if not worksheet.drawing_links then
      local rels = Relationships:new()

      for _, drawing_data in ipairs(worksheet.drawing_links) do
        rels:_add_document_relationship(drawing_data)
      end

      -- Create the .rels file such as /xl/drawings/_rels/sheet1.xml.rels.
      rels:_set_filename("xl/drawings/_rels/drawing" .. index .. '.xml.rels')
      self:_add_to_zip(rels)
    end
  end
end

----
-- Write the /xl/media/image?.xml files.
--
function Packager:_add_image_files()

  local workbook = self.workbook
  local index    = 1

  for _, image in ipairs(workbook.images) do
    local filename  = image[1]
    local extension = "." .. image[2]

    -- copy(filename, dir .. "/xl/media/image" .. index .. extension)
    index = index + 1
  end
end

return Packager
