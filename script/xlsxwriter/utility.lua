----
-- Utility - Utility functions for xlsxwriter.lua.
--
-- Copyright 2014-2015, John McNamara, jmcnamara@cpan.org
--
require "xlsxwriter.strict"

local Utility = {}
local char_A = string.byte("A")
local col_names = {}

local named_colors = {
  ["black"]   = "#000000",
  ["blue"]    = "#0000FF",
  ["brown"]   = "#800000",
  ["cyan"]    = "#00FFFF",
  ["gray"]    = "#808080",
  ["green"]   = "#008000",
  ["lime"]    = "#00FF00",
  ["magenta"] = "#FF00FF",
  ["navy"]    = "#000080",
  ["orange"]  = "#FF6600",
  ["pink"]    = "#FF00FF",
  ["purple"]  = "#800080",
  ["red"]     = "#FF0000",
  ["silver"]  = "#C0C0C0",
  ["white"]   = "#FFFFFF",
  ["yellow"]  = "#FFFF00",
}

----
-- Convert a zero indexed column cell reference to an Excel column string.
--
function Utility.col_to_name_abs(col_num, col_abs)

  local col_str      = col_names[col_num]
  local col_num_orig = col_num

  if not col_str then
    col_str = ""
    col_num = col_num + 1

    while col_num > 0 do
      -- Set remainder from 1 .. 26
      local remainder = col_num % 26
      if remainder == 0 then remainder = 26 end

      -- Convert the remainder to a character.
      local col_letter = string.char(char_A + remainder - 1)

      -- Accumulate the column letters, right to left.
      col_str = col_letter .. col_str

      -- Get the next order of magnitude.
      col_num = math.floor((col_num - 1) / 26)
    end

    col_names[col_num_orig] = col_str
  end

  if col_abs then col_str = '$' .. col_str end

  return col_str
end

----
-- Convert a zero indexed row and column cell reference to a A1 style string.
--
function Utility.rowcol_to_cell(row, col)
  row = math.modf(row + 1)
  col = math.modf(col)
  local col_str = Utility.col_to_name_abs(col, false)
  return col_str .. row
end

----
-- Convert a zero indexed row and column cell reference to a A1 style string
-- with Excel absolute indexing.
--
function Utility.rowcol_to_cell_abs(row, col, row_abs, col_abs)
  row = math.modf(row + 1)
  col = math.modf(col)
  row_abs = row_abs and "$" or ""
  local col_str = Utility.col_to_name_abs(col, col_abs)
  return col_str .. row_abs .. row
end

----
-- Convert a cell reference in A1 notation to a zero indexed row, column.
--
function Utility.cell_to_rowcol(cell)

  local col_str, row = cell:match("$?(%u+)$?(%d+)")

  -- Convert base26 column string to number.
  local expn = 0
  local col  = 0

  for i = #col_str, 1, -1 do
    local char = col_str:sub(i, i)
    col = col + (string.byte(char) - char_A + 1) * (26 ^ expn)
    expn = expn + 1
  end

  -- Convert 1-index to zero-index
  row = math.modf(row - 1)
  col = math.modf(col - 1)

  return row, col
end

----
-- Convert zero indexed row and col cell refs to a A1:B1 style range string.
--
function Utility.range(first_row, first_col, last_row, last_col)
  local range1 = Utility.rowcol_to_cell(first_row, first_col)
  local range2 = Utility.rowcol_to_cell(last_row,  last_col )
  return range1 .. ":" .. range2
end

----
-- Convert zero indexed row and col cell refs to absolute A1:B1 range string.
--
function Utility.range_abs(first_row, first_col, last_row, last_col)
  local range1 = Utility.rowcol_to_cell_abs(first_row, first_col, true, true)
  local range2 = Utility.rowcol_to_cell_abs(last_row,  last_col,  true, true)
  return range1 .. ":" .. range2
end

----
-- Generator for returning table items in sorted order. From PIL 3rd Ed.
--
function Utility.sorted_pairs(sort_table, sort_function)
  local array = {}
  for n in pairs(sort_table) do array[#array + 1] = n end

  table.sort(array, sort_function)

  local i = 0
  return function ()
    i = i + 1
    return array[i], sort_table[array[i]]
  end
end

----
-- Generator for returning table keys in sorted order.
--
function Utility.sorted_keys(sort_table, sort_function)
  local array = {}
  for n in pairs(sort_table) do array[#array + 1] = n end

  table.sort(array, sort_function)

  local i = 0
  return function ()
    i = i + 1
    return array[i]
  end
end

----
-- Print a non-fatal warning at the highest/calling program stack level.
--
function Utility.warn(...)
  local level = 0
  local info

  -- Find the last highest stack level.
  for i = 1, math.huge do
    info = debug.getinfo(i, "Sl")
    if not info then break end
    level = level + 1
  end

  -- Print warning to stderr at the calling program stack level.
  info = debug.getinfo(level -1, "Sl")
  io.stderr:write(string.format("Warning:\n\t%s:%d: ",
                                info.short_src,
                                info.currentline))
  io.stderr:write(string.format(...))
end

----
-- Convert a Html #RGB or named colour into an Excel ARGB formatted
-- color. Used in conjunction with various xxx_color() methods.
--
function Utility.excel_color(color)
  local rgb = color

  -- Convert named colours.
  if named_colors[color] then rgb = named_colors[color] end

  -- Extract the RBG part of the color.
  rgb = rgb:match("^#(%x%x%x%x%x%x)$")

  if rgb then
    -- Convert the RGB colour to the Excel ARGB format.
    return "FF" .. rgb:upper()
  else
    Utility.warn("Color '%s' is not a valid Excel color.\n", color)
    return "FF000000" -- Return Black as a default on error.
  end

end



----
-- The function takes an os.time style date table and converts it to a decimal
-- number representing a valid Excel date.
--
-- Dates and times in Excel are represented by real numbers. The integer part of
-- the number stores the number of days since the epoch and the fractional part
-- stores the percentage of the day in seconds. The epoch can be either 1900 or
-- 1904.
--
function Utility.convert_date_time(date_time, date_1904)

  local year  = date_time["year"]
  local month = date_time["month"]
  local day   = date_time["day"]
  local hour  = date_time["hour"] or 0
  local min   = date_time["min"]  or 0
  local sec   = date_time["sec"]  or 0

  -- For times without dates set the default date for the epoch
  if not year then
    if not date_1904 then
      year  = 1899; month = 12; day = 31
    else
      year  = 1904; month = 1;  day = 1
    end
  end

  -- Converte the Excel seconds to a fraction of the seconds in 24 hours.
  local seconds = (hour * 60 * 60 + min * 60 + sec) / (24 * 60 * 60)

  -- Special cases for Excel dates.
  if not date_1904 then
      -- Excel 1900 epoch.
    if year == 1899 and month == 12 and day == 31 then return seconds end
    if year == 1900 and month == 1  and day == 0  then return seconds end
    -- Excel false leapday
    if year == 1900 and month == 2 and day == 29 then return 60 + seconds end
  end

  -- We calculate the date by calculating the number of days since the epoch
  -- and adjust for the number of leap days. We calculate the number of leap
  -- days by normalising the year in relation to the epoch. Thus the year 2000
  -- becomes 100 for 4-year and 100-year leapdays and 400 for 400-year leapdays.
  --
  local epoch  = date_1904 and 1904 or 1900
  local offset = date_1904 and 4    or 0
  local norm   = 300
  local range  = year - epoch

  -- Set month days and check for leap year.
  local mdays = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}
  local leap = 0
  if year % 4 == 0 and year % 100 > 0 or year % 400 == 0 then
     leap     = 1
     mdays[2] = 29
  end

  -- Some boundary checks
  if year  < epoch or year > 9999        then return nil end
  if month < 1     or month > 12         then return nil end
  if day   < 1     or day > mdays[month] then return nil end

  -- Accumulate the number of days since the epoch.
  local days = day    -- Add days for current month

  for i = 1, month -1 do
    -- Add days for past months.
    days = days + mdays[i]
  end

  days = days + range * 365                               -- Past year days.
  days = days + math.floor((range                ) /   4) -- 4   yr leapdays.
  days = days - math.floor((range + offset       ) / 100) -- 100 yr leapdays.
  days = days + math.floor((range + offset + norm) / 400) -- 400 yr leapdays.
  days = days - leap                                      -- Already counted.

  -- Adjust for Excel erroneously treating 1900 as a leap year.
  if not date_1904 and days > 59 then days = days + 1 end

  return days + seconds
end

----
-- The function takes a date and time in ISO8601 "yyyy-mm-ddThh:mm:ss.ss" format
-- and converts it to a decimal number representing a valid Excel date.
--
-- See convert_date_time() funciton above.
--
function Utility.convert_date_string(date_str, date_1904)

  local date_time = {}

  -- Check for invalid date char.
  if string.match(date_str, "[^0-9T:%-%.Z]") then return nil end

  -- Strip trailing Z in ISO8601 date.
  date_str = date_str:gsub("Z", "")

  -- Get the date and time parts of the date string.
  local date = ""
  local time = ""

  if string.match(date_str, "T") then
    date, time = string.match(date_str, "(.*)T(.*)")
  elseif string.match(date_str, "^%d%d%d%d%-%d%d%-%d%d$") then
    date = date_str
  elseif string.match(date_str, "^%d%d:%d%d:%d%d") then
    time = date_str
  else
    return nil
  end

  if time ~= '' then
    -- Match times like hh:mm:ss.sss.
    local hour, min, sec = string.match(time, "^(%d%d):(%d%d):(.*)$")
    date_time["hour"] = tonumber(hour)
    date_time["min"]  = tonumber(min)
    date_time["sec"]  = tonumber(sec)
  end

  if date ~= '' then
    -- Match date as yyyy-mm-dd.
    local year, month, day = string.match(date, "^(%d%d%d%d)-(%d%d)-(%d%d)$")
    date_time["year"]  = tonumber(year)
    date_time["month"] = tonumber(month)
    date_time["day"]   = tonumber(day)
  end

  return Utility.convert_date_time(date_time, date_1904)
end

return Utility
