# Xlsxwriter for Lua

A Lua module for creating Excel XLSX files.

Note: this module is no longer maintained. It is functional and if it meets
your requirements then please use it. However, no new features will be added.


## The xlsxwriter module

Xlsxwriter is a Lua  module that can be used to write text, numbers, formulas
and hyperlinks to multiple worksheets in an Excel 2007+ XLSX file. It supports
features such as:

* 100% compatible Excel XLSX files.
* Full formatting.
* Memory optimisation mode for writing large files.
* Merged cells.
* Worksheet setup methods.
* Defined names.
* Document properties.

It works with Lua 5.1 and Lua 5.2.


Here is an example:


```lua
--
-- A simple example of some of the features of the xlsxwriter module.
--

local Workbook = require "xlsxwriter.workbook"

local workbook  = Workbook:new("demo.xlsx")
local worksheet = workbook:add_worksheet()

-- Widen the first column to make the text clearer.
worksheet:set_column("A:A", 20)

-- Add a bold format to use to highlight cells.
local bold = workbook:add_format({bold = true})

-- Write some simple text.
worksheet:write("A1", "Hello")

-- Text with formatting.
worksheet:write("A2", "World", bold)

-- Write some numbers, with row/column notation.
worksheet:write(2, 0, 123)
worksheet:write(3, 0, 123.456)

workbook:close()

```


![demo image](https://raw.githubusercontent.com/jmcnamara/xlsxwriter.lua/master/dev/docs/source/_images/demo.png)


See the full documentation at: http://xlsxwriterlua.readthedocs.org

Release notes: http://xlsxwriterlua.readthedocs.org/changes.html
