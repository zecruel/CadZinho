local Workbook = require "xlsxwriter.workbook"

-- Create a workbook and add a worksheet.
local workbook = Workbook:new("Expensese01.xlsx")

local worksheet = workbook:add_worksheet("exp")
-- Some data we want to write to the worksheet.
local expenses = {
	{"Rent", 1000},
	{"Gas", 100},
	{"Food", 300},
	{"Gym", 50},
}

-- Start from the first cell. Rows and columns are zero indexed.
local row = 0
local col = 0

-- Iterate over the data and write it out element by element.
for _, expense in ipairs(expenses) do
	local item, cost = unpack(expense)
	worksheet:write(row, col, item)
	worksheet:write(row, col + 1, cost)
	row = row + 1
end

-- Write a total using a formula.
worksheet:write(row, 0, "Total")
worksheet:write(row, 1, "=SUM(B1:B4)")
workbook:close()