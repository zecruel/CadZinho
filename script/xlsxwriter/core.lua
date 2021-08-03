----
-- Core - A class for writing the Excel XLSX Core file.
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
local Core = {}
setmetatable(Core,{__index = Xmlwriter})

function Core:new()
  local instance = {

    properties = {},
    localtime  = os.date("%Y-%m-%dT%H:%M:%SZ")
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
function Core:_assemble_xml_file()

  self:_xml_declaration()
  self:_write_cp_core_properties()
  self:_write_dc_title()
  self:_write_dc_subject()
  self:_write_dc_creator()
  self:_write_cp_keywords()
  self:_write_dc_description()
  self:_write_cp_last_modified_by()
  self:_write_dcterms_created()
  self:_write_dcterms_modified()
  self:_write_cp_category()
  self:_write_cp_content_status()

  self:_xml_end_tag("cp:coreProperties")

  -- Close the XML writer filehandle.
  self:_xml_close()

end

----
-- Set the document properties.
--
function Core:_set_properties(properties)
  self.properties = properties
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
-- Write the <cp:coreProperties> element.
--
function Core:_write_cp_core_properties()

  local xmlns_cp       = "http://schemas.openxmlformats.org/package/2006/metadata/core-properties"
  local xmlns_dc       = "http://purl.org/dc/elements/1.1/"
  local xmlns_dcterms  = "http://purl.org/dc/terms/"
  local xmlns_dcmitype = "http://purl.org/dc/dcmitype/"
  local xmlns_xsi      = "http://www.w3.org/2001/XMLSchema-instance"

  local attributes = {
    {["xmlns:cp"]       = xmlns_cp},
    {["xmlns:dc"]       = xmlns_dc},
    {["xmlns:dcterms"]  = xmlns_dcterms},
    {["xmlns:dcmitype"] = xmlns_dcmitype},
    {["xmlns:xsi"]      = xmlns_xsi},
  }

  self:_xml_start_tag("cp:coreProperties", attributes)
end

----
-- Write the <dc:creator> element.
--
function Core:_write_dc_creator()

  local data = self.properties["author"] or ""

  self:_xml_data_element("dc:creator", data)
end

----
-- Write the <cp:lastModifiedBy> element.
--
function Core:_write_cp_last_modified_by()

  local data = self.properties["author"] or ""

  self:_xml_data_element("cp:lastModifiedBy", data)
end

----
-- Write the <dcterms:created> element.
--
function Core:_write_dcterms_created()

  local date     = self.properties["created"] or self.localtime
  local xsi_type = "dcterms:W3CDTF"

  local attributes = {{["xsi:type"] = xsi_type}}

  self:_xml_data_element("dcterms:created", date, attributes)
end

----
-- Write the <dcterms:modified> element.
--
function Core:_write_dcterms_modified()

  local date     = self.properties["created"] or self.localtime
  local xsi_type = "dcterms:W3CDTF"

  local attributes = {{["xsi:type"] = xsi_type}}

  self:_xml_data_element("dcterms:modified", date, attributes)
end

----
-- Write the <dc:title> element.
--
function Core:_write_dc_title()

  local data = self.properties["title"]

  if data then
    self:_xml_data_element("dc:title", data)
  end
end

----
-- Write the <dc:subject> element.
--
function Core:_write_dc_subject()

  local data = self.properties["subject"]

  if data then
    self:_xml_data_element("dc:subject", data)
  end
end

----
-- Write the <cp:keywords> element.
--
function Core:_write_cp_keywords()

  local data = self.properties["keywords"]

  if data then
    self:_xml_data_element("cp:keywords", data)
  end
end

----
-- Write the <dc:description> element.
--
function Core:_write_dc_description()

  local data = self.properties["comments"]

  if data then
    self:_xml_data_element("dc:description", data)
  end
end

----
-- Write the <cp:category> element.
--
function Core:_write_cp_category()

  local data = self.properties["category"]

  if data then
    self:_xml_data_element("cp:category", data)
  end
end

----
-- Write the <cp:contentStatus> element.
--
function Core:_write_cp_content_status()

  local data = self.properties["status"]

  if data then
    self:_xml_data_element("cp:contentStatus", data)
  end
end

return Core
