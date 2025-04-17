local cURL = require "cURL"
local json = require("json")
local token = os.getenv("GITHUB_TOKEN")


files = {}
literals = {}
lit_per_file = {}

pfile = io.popen('ls -a ../src')
for lin in pfile:lines() do
  if string.find(lin, '^gui.*%.c$') then
    files[#files + 1] = lin
  end
end

for i = 1, #files do
  local lits = {}
  for lin in io.lines('../src/' .. files[i]) do
    for w in string.gmatch(lin, '_l%("(.+)"%)') do
      if not literals[w] then
        lits[#lits +1] = w
        literals[w] = true
      end
    end
  end
  lit_per_file[files[i]] = lits
end

function sleep(n)
  os.execute("sleep " .. tonumber(n))
end

function escape_json_string(str)
    local replacements = {
        ['"'] = '\\"',
        ['\\'] = '\\\\',
        ['\b'] = '\\b',
        ['\f'] = '\\f',
        ['\n'] = '\\n',
        ['\r'] = '\\r',
        ['\t'] = '\\t'
    }
    return str:gsub('[%z\1-\31\\"]', replacements)
end

local codes = [[
ar-SA Arábia Saudita
cs-CZ República Tcheca
da-DK Dinamarca
de-DE Alemanha
el-GR Grécia
en-AU Inglês da Austrália
en-GB Inglês do Reino Unido
en-IE Inglês da Irlanda
en-US Inglês dos Estados Unidos
en-ZA Inglês da África do Sul
es-ES Espanhol da Espanha
es-MX Espanhol do México
fi-FI Finlandês
fr-CA Francês do Canadá
fr-FR Francês da França
he-IL Hebraico de Israel
hi-IN Hindi da Índia
hu-HU Húngaro da Hungria
id-ID Indonésia
it-IT Italiano Itália
ja-JP Japão
ko-KR Coreano República da Coreia
nl-BE Bélgica
nl-NL Holanda
no-NO Noruega
pl-PL Polônia
pt-BR Português Brasil
pt-PT Português Portugal
ro-RO Romeno Romênia
ru-RU Federação Russa
sk-SK Eslovaco Eslováquia
sv-SE Suécia
th-TH Tailândia
tr-TR Turquia
zh-CN China
zh-HK Chinês Hong Kong
zh-TW Chinês Taiwan

"ar-SA", "cs-CZ", "da-DK", "de-DE", "el-GR", "en-AU", "en-GB", "en-IE", "en-US", "en-ZA", "es-ES", "es-MX", "fi-FI", "fr-CA", "fr-FR", "he-IL", "hi-IN", "hu-HU", "id-ID", "it-IT", "ja-JP", "ko-KR", "nl-BE", "nl-NL", "no-NO", "pl-PL", "pt-BR", "pt-PT", "ro-RO", "ru-RU", "sk-SK", "sv-SE", "th-TH", "tr-TR", "zh-CN", "zh-HK", "zh-TW"

]]

--local langs = {"ar-SA", "cs-CZ", "da-DK", "de-DE", "el-GR", "en-AU", "en-GB", "en-IE", "en-US", "en-ZA", "es-ES", "es-MX", "fi-FI", "fr-CA", "fr-FR", "he-IL", "hi-IN", "hu-HU", "id-ID", "it-IT", "ja-JP", "ko-KR", "nl-BE", "nl-NL", "no-NO", "pl-PL", "pt-BR", "pt-PT", "ro-RO", "ru-RU", "sk-SK", "sv-SE", "th-TH", "tr-TR", "zh-CN", "zh-HK", "zh-TW"}
local langs = {"es-ES", "pt-BR"}

local county_prompt = [=[Given an input in BCP-47 code, return the full language name in its own thonge, the corresponding writing system, and the corresponding country flag in simplified representation SVG graphic format as string.

Format the output as a Lua table:
<input> = {
language = "<language name>",
country = "<country name>",
system = "<writing system>",
flag = [[<flag SVG graphic string>]]
}

Examples:  
- Input: "pt-BR"
- Output:
   pt-BR = {
   language = "Português do Brasil",
   country = "Brazil",
   system = "Latin",
   flag = [[
<svg xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink" viewBox="0 0 512 512" width="512" height="512">
  <g>
    <path fill="#00A859" d="M0 0h512v512H0z"/>
    <path fill="#FFCC00" d="M256 0l128 128-128 128-128-128z"/>
    <circle cx="256" cy="256" r="128" fill="#3E4095"/>
    <path fill="#FFFFFF" d="M256 128l-64 64 64 64 64-64z"/>
    <text x="256" y="256" font-family="Arial" font-size="24" text-anchor="middle" fill="#FFFFFF">Ordem e Progresso</text>
  </g>
</svg>
]]
}

- Input: "ru-RU"
- Output:
ru-RU = {
language = "Русский",
country = "Russia",
system = "Cyrillic",
flag = [[ <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 512 512" width="512" height="512"> <g> <path fill="#0000FF" d="M0 0h512v170.7H0z"/> <path fill="#FFFFFF" d="M0 170.7h512v170.6H0z"/> <path fill="#D52B1E" d="M0 341.3h512v170.7H0z"/> </g> </svg> ]]
}

- Input: "zh-CN"
- Output:
zh-CN = {
language = "中文",
country = "China",
system = "Hanzi",
flag = [[ <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 512 512" width="512" height="512"> <g> <path fill="#DE2910" d="M0 0h512v341.3H0z"/> <path fill="#FFDE00" d="M0 0h170.7v170.7H0z"/> <path fill="#DE2910" d="M0 0h170.7v170.7H0z"/> <circle cx="85.35" cy="85.35" r="42.675" fill="#FFDE00"/> <circle cx="85.35" cy="85.35" r="28.45" fill="#DE2910"/> <path fill="#FFDE00" d="M42.675 0h42.68v42.68H42.675zM0 42.675h42.68v42.68H0zM85.35 42.675h42.68v42.68H85.35zM42.675 85.35h42.68v42.68H42.675zM85.35 85.35h42.68v42.68H85.35zM128.025 85.35h42.68v42.68h-42.68zM42.675 128.025h42.68v42.68H42.675zM85.35 128.025h42.68v42.68H85.35zM128.025 128.025h42.68v42.68h-42.68z"/> </g> </svg> ]]
}


Now, process following input: ]=];



local terms_prompt = "Rewrite the Lua ‘translate’ table below, translating the value strings into "


for k, lang in pairs(langs) do
  local buffer = {}
  c = cURL.easy{
    url        = "https://models.inference.ai.azure.com/chat/completions",
    
    post       = true,
    httpheader = {
      "Content-Type: application/json", "Authorization: Bearer " .. token;
    };
    postfields = [[{"messages": [
      {
        "role": "system",
        "content": "Respond with only the response no additional comments."
      },
      {
        "role": "user",
        "content": "]] .. escape_json_string(county_prompt .. lang) .. [["
      }
    ],
    "temperature": 0.0,
    "top_p": 1.0,
    "max_tokens": 4000,
    "model": "gpt-4o"}]];
  }

  c:setopt_writefunction(table.insert, buffer)
  c:perform()
  c:close()
  sleep(5)
  
  print (table.concat(buffer))
  local data = json.decode(table.concat(buffer))
  
  local str = string.gsub(data.choices[1].message.content, '```lua', "\n")
  str = string.gsub(str, '```', "\n")
  print(str)

  out_json = io.open(string.gsub(lang, '-', '_') .. ".lua", "w+")
  out_json:write(str)
  
  
  
  -- process terms
  out_json:write('\ntranslate = {}\n')
  local count = 0
  
  for i = 1, #files do
    out_json:write('--------- ' .. files[i] .. ' --------- \n')
    local lits = lit_per_file[files[i]]
    for j = 1, #lits do
      str = str .. 'translate["' ..  lits[j] .. '"] = "' .. lits[j] .. '"\n'
      count = count + 1
      
      if count >= 70 then
        buffer = {}
        c = cURL.easy{
          url        = "https://models.inference.ai.azure.com/chat/completions",
          
          post       = true,
          httpheader = {
            "Content-Type: application/json", "Authorization: Bearer " .. token;
          };
          postfields = [[{"messages": [
            {
              "role": "system",
              "content": "Respond with only the response no additional comments."
            },
            {
              "role": "user",
              "content": "]] .. escape_json_string(terms_prompt .. lang .. str) .. [["
            }
          ],
          "temperature": 0.0,
          "top_p": 1.0,
          "max_tokens": 4000,
          "model": "gpt-4o"}]];
        }

        c:setopt_writefunction(table.insert, buffer)
        c:perform()
        c:close()
        sleep(5)

        data = json.decode(table.concat(buffer))
        print (table.concat(buffer))
        str = string.gsub(data.choices[1].message.content, '```lua\n', "")
        str = string.gsub(str, '```', "")
        print(str)
      
        out_json:write(str)
      
      
      
      
      
      
        str = '\n'
        count = 0
      end
    end
    
    
    
    
    end
    
    
    
  end
  
  if count > 0 then
  buffer = {}
    c = cURL.easy{
    url        = "https://models.inference.ai.azure.com/chat/completions",
    
    post       = true,
    httpheader = {
      "Content-Type: application/json", "Authorization: Bearer " .. token;
    };
    postfields = [[{"messages": [
      {
        "role": "system",
        "content": "Respond with only the response no additional comments."
      },
      {
        "role": "user",
        "content": "]] .. escape_json_string(terms_prompt .. lang .. str) .. [["
      }
    ],
    "temperature": 0.0,
    "top_p": 1.0,
    "max_tokens": 4000,
    "model": "gpt-4o"}]];
  }

  c:setopt_writefunction(table.insert, buffer)
  c:perform()
  c:close()
  sleep(5)

  data = json.decode(table.concat(buffer))
  
  str = string.gsub(data.choices[1].message.content, '```lua\n', "")
  str = string.gsub(str, '```', "")
  print(str)

  out_json:write(str)
  
  
  out_json:close()
end

