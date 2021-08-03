----
-- App - A class for writing the Excel XLSX App file.
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
local App = {}
setmetatable(App,{__index = Xmlwriter})

function App:new()
  local instance = {

    part_names    = {},
    heading_pairs = {},
    properties    = {},
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
function App:_assemble_xml_file()

  self:_xml_declaration()
  self:_write_properties()
  self:_write_application()
  self:_write_doc_security()
  self:_write_scale_crop()
  self:_write_heading_pairs()
  self:_write_titles_of_parts()
  self:_write_manager()
  self:_write_company()
  self:_write_links_up_to_date()
  self:_write_shared_doc()
  self:_write_hyperlinks_changed()
  self:_write_app_version()

  self:_xml_end_tag("Properties")

  -- Close the XML writer filehandle.
  self:_xml_close()

end

----
-- Add the name of a workbook Part such as 'Sheet1' or 'Print_Titles'.
--
function App:_add_part_name(part_name)
  table.insert(self.part_names, part_name)
end

----
-- Add the name of a workbook Heading Pair such as 'Worksheets', 'Charts' or
-- 'Named Ranges'.
--
function App:_add_heading_pair(name, size)
  if size > 0 then
    table.insert(self.heading_pairs, {["lpstr"] = name})
    table.insert(self.heading_pairs, {["i4"]    = size})
  end
end

----
-- Set the document properties.
--
function App:_set_properties(properties)
  self.properties = properties
end

------------------------------------------------------------------------------
--
-- XML writing methods.
--
------------------------------------------------------------------------------

----
-- Write the <Properties> element.
--
function App:_write_properties()

  local schema   = "http://schemas.openxmlformats.org/officeDocument/2006/"
  local xmlns    = schema .. "extended-properties"
  local xmlns_vt = schema .. "docPropsVTypes"

  local attributes = {
    {["xmlns"]    = xmlns},
    {["xmlns:vt"] = xmlns_vt},
  }

  self:_xml_start_tag("Properties", attributes)
end

----
-- Write the <Application> element.
--
function App:_write_application()
  self:_xml_data_element("Application", "Microsoft Excel")
end

----
-- Write the <DocSecurity> element.
--
function App:_write_doc_security()
  self:_xml_data_element("DocSecurity", "0")
end

----
-- Write the <ScaleCrop> element.
--
function App:_write_scale_crop()
  self:_xml_data_element("ScaleCrop", "false")
end

----
-- Write the <HeadingPairs> element.
--
function App:_write_heading_pairs()

  self:_xml_start_tag("HeadingPairs")

  self:_write_vt_vector("variant", self.heading_pairs)

  self:_xml_end_tag("HeadingPairs")
end

----
-- Write the <TitlesOfParts> element.
--
function App:_write_titles_of_parts()
  local parts_data = {}

  self:_xml_start_tag("TitlesOfParts")

  for _, part_name in ipairs(self.part_names) do
    table.insert(parts_data, {["lpstr"] = part_name})
  end

  self:_write_vt_vector("lpstr", parts_data)

  self:_xml_end_tag("TitlesOfParts")
end

----
-- Write the <vt:vector> element.
--
function App:_write_vt_vector(base_type, data)
  local size      = #data

  local attributes = {
    {["size"]     = size},
    {["baseType"] = base_type},
  }

  self:_xml_start_tag("vt:vector", attributes)

  for _, vt_data in ipairs(data) do
    if base_type == "variant" then
      self:_xml_start_tag('vt:variant')
    end

    self:_write_vt_data(vt_data)

    if base_type == "variant" then
      self:_xml_end_tag('vt:variant')
    end
  end

  self:_xml_end_tag("vt:vector")
end

----
-- Write the <vt:*> elements such as <vt:lpstr> and <vt:if>.
--
function App:_write_vt_data(data)
  local vt_type, value = next(data)
  self:_xml_data_element("vt:" .. vt_type, value)
end

----
-- Write the <Company> element.
--
function App:_write_company()
  local data = self.properties["company"] or ''

  self:_xml_data_element("Company", data)
end

----
-- Write the <Manager> element.
--
function App:_write_manager()
  local data = self.properties["manager"]

  if data then
    self:_xml_data_element("Manager", data)
  end
end

----
-- Write the <LinksUpToDate> element.
--
function App:_write_links_up_to_date()
  self:_xml_data_element("LinksUpToDate", "false")
end

----
-- Write the <SharedDoc> element.
--
function App:_write_shared_doc()
  self:_xml_data_element("SharedDoc", "false")
end

----
-- Write the <HyperlinksChanged> element.
--
function App:_write_hyperlinks_changed()
  self:_xml_data_element("HyperlinksChanged", "false")
end

----
-- Write the <AppVersion> element.
--
function App:_write_app_version()
  self:_xml_data_element("AppVersion", "12.0000")
end

return App
