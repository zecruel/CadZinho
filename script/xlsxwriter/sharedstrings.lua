----
-- SharedStrings - A class for writing the Excel XLSX SharedStrings file.
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
local SharedStrings = {}
setmetatable(SharedStrings,{__index = Xmlwriter})

function SharedStrings:new()
  local instance = {
    string_table = {},
    string_array = {},
    string_count = 0,
    unique_count = 0,
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
function SharedStrings:_assemble_xml_file()

  self:_xml_declaration()

  -- Write the sst table.
  self:_write_sst(self.string_count, self.unique_count)

  -- Write the sst strings.
  self:_write_sst_strings()

  -- Close the sst tag.
  self:_xml_end_tag("sst")

  -- Close the XML writer filehandle.
  self:_xml_close()

end

----
-- Get the index of the string in the Shared String table.
--
function SharedStrings:_get_string_index(str)

  local index = self.string_table[str]

  if index then
    -- String exists in the table.
    self.string_count =  self.string_count + 1
    return index
  else
    -- String isn't already stored in the table so add it.
    index = self.unique_count
    self.string_table[str] = index
    self.string_array[index + 1] = str
    self.string_count =  self.string_count + 1
    self.unique_count =  self.unique_count + 1
    return index
  end

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
-- Write the <sst> element.
--
function SharedStrings:_write_sst(string_count, unique_count)

  local schema       = "http://schemas.openxmlformats.org"
  local xmlns        = schema .. "/spreadsheetml/2006/main"

  local attributes = {
    {["xmlns"]       = xmlns},
    {["count"]       = string_count},
    {["uniqueCount"] = unique_count},
  }

  self:_xml_start_tag("sst", attributes)
end

----
-- Write the sst string elements.
--
function SharedStrings:_write_sst_strings()
  for _, str in ipairs(self.string_array) do
    self:_write_si(str)
  end
end

----
-- Write the <si> element.
--
function SharedStrings:_write_si(str)
  local attributes = {}

  -- Excel escapes control characters with _xHHHH_ and also escapes any
  -- literal strings of that type by encoding the leading underscore. So
  -- "\0" -> _x0000_ and "_x0000_" -> _x005F_x0000_.
  -- The following substitutions deal with those cases.

  -- Escape the escape.
  -- str =~ s/(_x[0-9a-fA-F]{4}_)/_x005F1/g

  -- Convert control character to the _xHHHH_ escape.
  -- str =~ s/([\x00-\x08\x0B-\x1F])/sprintf "_x04X_", ord(1)/eg

  -- Add attribute to preserve leading or trailing whitespace.
  if string.match(str, "^%s") or string.match(str, "%s$") then
    attributes = {{["xml:space"] = "preserve"}}
  end

  -- Write any rich strings without further tags.
  if string.match(str, "^<r>") or string.match(str, "</r>$") then
    self:_xml_rich_si_element(str)
  else
    self:_xml_si_element(str, attributes)
  end
end



return SharedStrings
