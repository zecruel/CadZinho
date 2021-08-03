----
-- Rels - A class for writing the Excel XLSX Rels file.
--
-- Copyright 2014-2015, John McNamara, jmcnamara@cpan.org
--
require "xlsxwriter.strict"

local Xmlwriter = require "xlsxwriter.xmlwriter"

local schema_root     = "http://schemas.openxmlformats.org"
local package_schema  = schema_root .. "/package/2006/relationships"
local document_schema = schema_root .. "/officeDocument/2006/relationships"

------------------------------------------------------------------------------
--
-- Constructor.
--
------------------------------------------------------------------------------

-- The constructor inherits from xmlwriter.lua.
local Rels = {}
setmetatable(Rels,{__index = Xmlwriter})

function Rels:new()
  local instance = {
    rels = {},
    id   = 1,
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
function Rels:_assemble_xml_file()

  self:_xml_declaration()
  self:_write_relationships()

  -- Close the XML writer filehandle.
  self:_xml_close()
end

----
-- Add container relationship to XLSX .rels xml files.
--
function Rels:_add_document_relationship(rel_type, target)
  rel_type = document_schema .. rel_type

  table.insert(self.rels, {rel_type, target})
end

----
-- Add container relationship to XLSX .rels xml files.
--
function Rels:_add_package_relationship(rel_type, target)
  rel_type = package_schema .. rel_type

  table.insert(self.rels, {rel_type, target})
end

----
-- Add container relationship to XLSX .rels xml files. Uses MS schema.
--
function Rels:_add_ms_package_relationship(rel_type, target)
  local schema = "http://schemas.microsoft.com/office/2006/rels"

  rel_type = schema .. rel_type

  table.insert(self.rels, {rel_type, target})
end

----
-- Add worksheet relationship to sheet.rels xml files.
--
function Rels:_add_worksheet_relationship(rel_type, target, target_mode)
  rel_type = document_schema .. rel_type

  table.insert(self.rels, {rel_type, target, target_mode})
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
-- Write the <Relationships> element.
--
function Rels:_write_relationships()

  local attributes = {{["xmlns"] = package_schema}}

  self:_xml_start_tag("Relationships", attributes)

  for _, rel in ipairs(self.rels) do
    self:_write_relationship(unpack(rel))
  end

  self:_xml_end_tag("Relationships")

end

----
-- Write the <Relationship> element.
--
function Rels:_write_relationship(rel_type, target, target_mode)

  local attributes = {
    {["Id"]      = "rId" .. self.id},
    {["Type"]    = rel_type},
    {["Target"]  = target},
  }

  self.id = self.id + 1

  if target_mode then
    table.insert(attributes, {["TargetMode"] = target_mode})
  end

  self:_xml_empty_tag("Relationship", attributes)
end

return Rels
