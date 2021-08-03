----
-- Xmlwriter - A base class for xlsxwriter.lua xml classes.
--
-- Copyright 2014-2015, John McNamara, jmcnamara@cpan.org
--
require "xlsxwriter.strict"

local Xmlwriter = {}

----
-- The Xmlwriter object constructor.
--
function Xmlwriter:new (instance)
  instance = instance or {}
  setmetatable(instance, self)
  self.__index = self
  return instance
end

----
-- Create the XML writer filehandle for the object.
--
function Xmlwriter:_set_xml_writer(filename)
  self.fh = assert(io.open(filename, "w"))
  self.internal_fh = true
end

----
-- Set an externally created filehandle. Mainly for testing.
--
function Xmlwriter:_set_filehandle(filehandle)
  self.fh = filehandle
  self.internal_fh = false
end

----
-- Set the filename used in the zipfile.
--
function Xmlwriter:_set_filename(filename)
  self.filename = filename
end

----
-- Close the XML filehandle if we created it.
--
function Xmlwriter:_xml_close()
  if self.internal_fh then
    self.fh:close()
  end
end

----
-- Return all of the data in the current filehandle.
--
function Xmlwriter:_get_data()
  self.fh:seek('set', 0)
  return self.fh:read('*a')
end

----
-- Return all of the data in the current filehandle as an iterator.
--Used by the ZipWriter module.
--
function Xmlwriter:_get_xml_reader()
  self.fh:seek('set', 0)
  return function()
           local buffer = self.fh:read(4096)
           if buffer then return buffer end
         end
end


----
-- Write the XML declaration.
--
function Xmlwriter:_xml_declaration()
  self.fh:write('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\n')
end

----
-- Write an XML start tag with optional attributes.
--
function Xmlwriter:_xml_start_tag(tag, attributes)
  local attr = self._format_attributes(attributes)

  self.fh:write(string.format('<%s%s>', tag, attr))
end

----
-- Write an XML start tag with optional, unencoded, attributes.
--
function Xmlwriter:_xml_start_tag_unencoded(tag, attributes)
  local attr = self._format_attributes_unencoded(attributes)

  self.fh:write(string.format('<%s%s>', tag, attr))
end

----
-- Write an XML end tag.
--
function Xmlwriter:_xml_end_tag(tag)
  self.fh:write(string.format('</%s>', tag))
end

----
-- Write an XML empty tag with optional attributes.
--
function Xmlwriter:_xml_empty_tag(tag, attributes)
  local attr = self._format_attributes(attributes)

  self.fh:write(string.format('<%s%s/>', tag, attr))
end

----
-- Write an XML empty tag with optional, unencoded, attributes.
--
function Xmlwriter:_xml_empty_tag_unencoded(tag, attributes)
  local attr = self._format_attributes_unencoded(attributes)

  self.fh:write(string.format('<%s%s/>', tag, attr))
end

----
-- Write an XML element containing data with optional attributes.
--
function Xmlwriter:_xml_data_element(tag, data, attributes)
  local attr = self._format_attributes(attributes)

  data = self._escape_data(data)

  self.fh:write(string.format('<%s%s>%s</%s>', tag, attr, data, tag))
end

----
-- Optimised tag writer for <c> cell string elements in the inner loop.
--
function Xmlwriter:_xml_string_element(index, attributes)
  local attr = self._format_attributes(attributes)

  self.fh:write(string.format('<c%s t="s"><v>%d</v></c>', attr, index))
end

----
-- Optimised tag writer for shared strings <si> elements.
--
function Xmlwriter:_xml_si_element(str, attributes)
  local attr = self._format_attributes(attributes)
  str = self._escape_data(str)

  self.fh:write(string.format('<si><t%s>%s</t></si>', attr, str))
end

----
-- Optimised tag writer for shared strings <si> rich string elements.
--
function Xmlwriter:_xml_rich_si_element(str)
  self.fh:write(string.format('<si>%s</si>', str))
end

----
-- Optimised tag writer for <c> cell number elements in the inner loop.
--
function Xmlwriter:_xml_number_element(number, attributes)
  local attr = self._format_attributes(attributes)

  self.fh:write(string.format('<c%s><v>%.15g</v></c>', attr, number))
end

----
-- Optimised tag writer for <c> cell formula elements in the inner loop.
--
function Xmlwriter:_xml_formula_element(formula, result, attributes)
  local attr = self._format_attributes(attributes)

  formula = self._escape_data(formula)
  result  = self._escape_data(result)

  self.fh:write(string.format('<c%s><f>%s</f><v>%s</v></c>',
                              attr, formula, result))
end

----
-- Optimised tag writer for inlineStr cell elements in the inner loop.
--
function Xmlwriter:_xml_inline_string(str, preserve, attributes)
  local attr = self._format_attributes(attributes)
  local t_attr = ''

  if preserve then
    t_attr = ' xml:space="preserve"'
  end

  str = self._escape_data(str)

  self.fh:write(string.format('<c%s t="inlineStr"><is><t%s>%s</t></is></c>',
                              attr, t_attr, str))
end

----
-- Optimised tag writer for rich inlineStr in the inner loop.
--
function Xmlwriter:_xml_rich_inline_string(str, attributes)
  local attr = self._format_attributes(attributes)

  str = self._escape_data(str)

  self.fh:write(string.format('<c%s t="inlineStr"><is>%s</is></c>', attr, str))
end

----
-- Format attribute value pairs.
--
function Xmlwriter._format_attributes(attributes)
  local attr = ''

  if attributes then
    for i = 1, #attributes do
      for key, value in pairs(attributes[i]) do
        value = Xmlwriter._escape_attributes(value)
        attr = attr .. string.format(' %s="%s"', key, value)
      end
    end
  end

  return attr
end

----
-- Format attribute value pairs with unencoded attributes.
-- This is a minor speed optimisation for elements that
-- don't need encoding.
--
function Xmlwriter._format_attributes_unencoded(attributes)
  local attr = ''

  if attributes then
    for i = 1, #attributes do
      for key, value in pairs(attributes[i]) do
        attr = attr .. string.format(' %s="%s"', key, value)
      end
    end
  end

  return attr
end

----
-- Escape XML characters in attributes.
--
local escape_attrib_tbl = {
	['&'] = '&amp;',
	['"'] = '&quot;',
	['<'] = '&lt;',
	['>'] = '&gt;',
}

function Xmlwriter._escape_attributes(attribute)
  return string.gsub(attribute, '[&"<>]', escape_attrib_tbl)
end

----
-- Escape XML characters in data sections of tags. Double quotes
-- are not escaped by Excel in XML data.
--

local escape_data_tbl = {
	['&'] = '&amp;',
	['<'] = '&lt;',
	['>'] = '&gt;',
}

function Xmlwriter._escape_data(data)
  return string.gsub(data, '[&<>]', escape_data_tbl)
end

return Xmlwriter
