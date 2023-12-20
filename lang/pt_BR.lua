-- CadZinho GUI language localisation (translation) file
-- This file is writen in Lua language

-- NOTE1: The purpose of this script file is load global initial parameters. Don't expect script features to fully work.

-- Each file is specific for one variant language. The file name must be coded 
-- according POSIX convention (standarts ISO 639-1 and ISO 3166-1 alpha-2),
-- like lang_COUNTRY codes (eg. en_US.lua, pt_BR.lua, cz_CZ.lua)

-- This file must be encoded in UTF8

-- Operation principles: 
-- The table 'translate' is the main mandatory global variable in file.
-- Translated expressions are stored as keys/values in the table, like
-- translate["Original expression"] = "Translated expression".
-- The internal original expressions will be used for the absent keys or 
-- without values in the table (please, don't delete or modify the keys -
-- comment the line if you want to ignore the value)
-- The 'descr' variable string is a short description, written in translated language.
-- The 'flag' variable string is a SVG image of the country's flag

-- NOTE2: C standard printf control sequences (eg. "%d", "%.5f")
--       and also '#' char mark must be maintained in translated strings


descr = "Português (Brasil)"

flag = [[<svg width="512" height="335" xmlns="http://www.w3.org/2000/svg" xmlns:svg="http://www.w3.org/2000/svg">
	  <path id="brazil1" fill="#007f00" d="m512.05078,335.65631c-128,0 -512,0 -512,0l0,-335.4l512,0c0,0 0,245 0,326.6c0,4.9 0,8.8 0,8.8z" class="st0"/>
	  <path fill="#ffff00" id="brazil2" d="m251.2361,30.37856l-210.32621,128.11806c-6.44548,3.84467 -6.44548,13.11712 0,17.07487l210.43929,128.11806c3.1662,1.92234 7.23703,1.92234 10.40323,0l210.43929,-128.11806c6.44548,-3.84467 6.44548,-13.11712 0,-17.07487l-210.55237,-128.11806c-3.1662,-1.92234 -7.23703,-1.92234 -10.40323,0z" class="st1"/>
	  <circle id="brazil3" fill="#0000ff" r="94.59999" cy="167.95632" cx="254.05078" class="st2"/>
	  <path stroke="null" fill="#ffffff" id="svg_5" d="m172.85022,119.58436c-4.42181,7.50368 -7.90567,15.54334 -10.18357,24.25298c52.39179,-3.88584 130.10851,10.71955 179.41842,58.68952c3.21586,-8.17365 5.35977,-17.01728 6.16374,-26.1289c-51.05184,-43.01218 -121.53287,-58.28754 -175.39859,-56.8136z" class="st3"/>
	</svg>]]


translate = {}
--------- gui_attrib.c --------- 
translate["Error: Attribute must have a Tag"] = "Erro: Etiqueta precisa de um rótulo"
translate["ADD TAG"] = "NOVA ETIQUETA"
translate["Add a Tag"] = "Nova Etiqueta"
translate["Select a Insert element"] = "Selecione um bloco"
translate["Style:"] = "Estilo:"
translate["Hide"] = "Oculta"
translate["Tag:"] = "Rótulo:"
translate["Value:"] = "Valor:"
translate["Height"] = "Altura"
translate["Angle"] = "Ângulo"
translate["Base Line"] = "Lin. base"
translate["Middle"] = "Meio"
translate["Center"] = "Centro"
translate["Aligned"] = "Alinhado"
translate["Fit"] = "Ajustado"

--------- gui_block.c --------- 
translate["NEW BLOCK"] = "NOVO BLOCO"
translate["Create a new block"] = "Cria um novo bloco"
translate["Select/Add element"] = "Selec./Adic. elemento"
translate["Enter insert point"] = "Defina um ponto de inserção"
translate["Confirm"] = "Confirme"
translate["Blocks Manager"] = "Gerenciador de blocos"
translate["Name"] = "Nome"
translate["Attr"] = "Etiq"
translate["Used"] = "Usado"
translate["Hidden"] = "Oculto"
translate["Create"] = "Cria"
translate["Edit"] = "Edita"
translate["Remove"] = "Remove"
translate["Error: Don't remove Block in use"] = "Erro: não apaga bloco em uso"
translate["Remove Block"] = "Apaga Bloco"
translate["Attributes"] = "Etiquetas"
translate["Select"] = "Selecione"
translate["Edit Block"] = "Edita Bloco"
translate["New name:"] = "Novo nome:"
translate["Rename"] = "Renomeia"
translate["Error: exists Block with same name"] = "Erro: já existe este Bloco"
translate["New description:"] = "Nova descrição:"
translate["Update"] = "Atualiza"
translate["Cancel"] = "Cancela"
translate["New Block"] = "Novo Bloco"
translate["Name:"] = "Nome:"
translate["Description:"] = "Descrição:"
translate["Text to Attributes"] = "Texto p/ Etiquetas"
translate["Attrib. mark:"] = "Marca etiq:"
translate["Hide mark:"] = "Marca ocul:"
translate["Value mark:"] = "Marca valor:"
translate["Default value:"] = "Valor padrão:"
translate["Create block from:"] = "Cria bloco a partir:"
translate["Interactive"] = "Interativo"
translate["File"] = "Arquivo"
translate["Proceed"] = "Continua"
translate["Error: invalid Block name"] = "Erro: nome de bloco inválido"
translate["Path:"] = "Caminho:"
translate["Browse"] = "Explora"
translate["Drawing files (.dxf)"] = "Arquivos CAD (.dxf)"
translate["All files (*)"] = "Todos arquivos (*)"
translate["Only reference"] = "Somente refer."
translate["Full path"] = "Caminho compl"
translate["Error: in Block creation from file"] = "Erro: Bloco a partir do arquivo"
translate["Edit Attributes"] = "Edita Etiquetas"
translate["Tag"] = "Etiqueta"
translate["Value"] = "Valor"
translate["OK"] = "OK"
translate["Error: No spaces allowed in tags"] = "Erro: rótulo não deve ter espaço "
translate["Edit Block Attributes"] = "Edita Etiquetas do Bloco"
--------- gui_circle.c --------- 
translate["CIRCLE"] = "CÍRCULO"
translate["ARC"] = "ARCO"
translate["Full circle"] = "Círculo completo"
translate["Circular arc"] = "Arco circular"
translate["Place a circular arc"] = "Novo arco circular"
translate["Enter center point"] = "Entre o centro"
translate["Enter circle end point"] = "Entre o ponto final"
translate["Enter arc start point"] = "Ponto inicial arco"
translate["Enter arc end point"] = "Ponto final arco"
--------- gui_config.c --------- 
translate["Standard"] = "Padrão"
translate["Internal standard pattern library"] = "Biblioteca de tramas interna padrão"
translate["Config"] = "Configurações"
translate["Preferences"] = "Preferências"
translate["Info"] = "Informação"
translate["3D"] = "3D"
translate["Preferences folder:"] = "Pasta de preferências:"
translate["Copy"] = "Copia"
translate["Open folder"] = "Abre a pasta"
translate["Config File:"] = "Arquivo de configuração:"
translate["Open file"] = "Abre o arquivo"
translate["Reload config"] = "Recarrega as configurações"
translate["Interface language:"] = "Língua da interface:"
translate["Open Info Window"] = "Abre a janela de informações"
translate["The following window is used to visualize " ..
    "the raw parameters of the selected elements, according to " ..
    "the DXF specification. It is useful for advanced users to debug " ..
    "current file entities"] = "A janela a seguir é utilizada para " ..
    "visualizar os parâmetros brutos dos elementos selecionados, de acordo " ..
    "com a especificação de arquivos DXF. É útil para usuários avançados "..
    "depurar as entidades do arquivo atual"
translate["Open Info Window"] = "Abre a janela de informações"
translate["This is a experimental 3D view mode. " ..
    "To return to default 2D view, choose \"Top\" " ..
    "or set all angles to 0"] = 
    "Este é um modo de visualização 3D experimental. " ..
    "Para retornar à vista 2D padrão, escolha 'Topo' " ..
    "ou ajuste tods os ângulos para 0"
translate["#Alpha"] = "#Alfa"
translate["#Beta"] = "#Beta"
translate["#Gamma"] = "#Gama"
translate["Top"] = "Topo"
translate["Front"] = "Frontal"
translate["Right"] = "Direita"
translate["Bottom"] = "Inferior"
translate["Rear"] = "Traseira"
translate["Left"] = "Esquerda"
translate["Iso"] = "Isométrica"
translate["Theme:"] = "Tema:"
translate["Cursor:"] = "Cursor:"
translate["Background Color:"] = "Cor do fundo:"
translate["Hilite Color:"] = "Cor de destaque:"
translate["Black"] = "Preto"
translate["White"] = "Branco"
translate["Red"] = "Vermelho"
translate["Blue"] = "Azul"
translate["Green"] = "Verde"
translate["Brown"] = "Marrom"
translate["Purple"] = "Roxo"
translate["Dracula"] = "Drácula"
translate["Nuklear"] = "Nuklear"
translate["Cross"] = "Cruz"
translate["Square"] = "Quadrado"
translate["Circle"] = "Círculo"
--------- gui_dim.c --------- 
translate["Place a Linear Dim"] = "Nova cota linear"
translate["Enter start point"] = "Entre o ponto inicial"
translate["Enter end point"] = "Entre o ponto final"
translate["Enter distance"] = "Defina a distância"
translate["Fixed angle"] = "Ângulo fixo"
translate["Fixed distance"] = "Distância fixa"
translate["distance"] = "distância"
translate["Custom Text"] = "Texto personalizado"
translate["Linear DIMENSION"] = "COTA LINEAR"
translate["Place a Angular Dim"] = "Nova cota angular"
translate["Angular DIMENSION"] = "COTA ANGULAR"
translate["Place a Dimension:"] = "Nova cota:"
translate["Radial"] = "Radial"
translate["Diametric"] = "Diametrica"
translate["Enter quadrant point"] = "Entre o ponto quadrante"
translate["Enter oposite point"] = "Entre o ponto oposto"
translate["Enter radius point"] = "Entre o ponto raio"
translate["Enter location"] = "Entre a locação"
translate["Diametric DIMENSION"] = "COTA DIAMETRICA"
translate["Radial DIMENSION"] = "COTA RADIAL"
translate["Place a Ordinate Dim"] = "Nova cota ordenada"
translate["X"] = "X"
translate["Y"] = "Y"
translate["Enter extension"] = "Entre a extensão"
translate["Next ordinate"] = "Próxima ordenada"
translate["Ordinate DIMENSION"] = "COTA ORDENADA"
translate["Dimension Style Manager"] = "Gerenciador Estilos Cota"
translate["List of styles"] = "Lista de estilos"
translate["Delete"] = "Deleta"
translate["Error: Don't remove DIMSTYLE in use"] = "Error: não apaga estilo cota em uso"
translate["Remove DIMSTYLE"] = "APAGA ESTILO COTA"
translate["Selected parameters"] = "Parâmetros selecionados"
translate["Global Scale"] = "Escala global"
translate["Meas. Factor"] = "Escala medida"
translate["Dec. places"] = "Casas decim."
translate["Annotation text:"] = "Texto da cota:"
translate["Annotation text style:"] = "Estilo do texto:"
translate["Terminator:"] = "Tipo da Ponta:"
translate["Advanced"] = "Avançado"
translate["Term. size"] = "Tamanho term."
translate["Offset"] = "Deslocamento"
translate["Extension"] = "Extrapolação"
translate["Text size"] = "Tam. texto"
translate["Text gap"] = "Espaço texto"
translate["Dim Style Name"] = "Nome estilo cota"
translate["Error: DIMSTYLE already exists"] = "Erro: estilo cota já existe"
--------- gui_dupli.c --------- 
translate["DUPLI"] = "DUPLICA"
translate["Duplicate a selection"] = "Duplica selecionados"
translate["Enter base point"] = "Entre o ponto base"
translate["Enter destination point"] = "Entre o ponto destino"
--------- gui_edit_attr.c --------- 
translate["Error: No attributes found"] = "Erro: Sem etiquetas"
translate["Edit data"] = "Edita os dados"
translate["Edit Insert Attributes"] = "Edita etiquetas do bloco"
translate["Block:"] = "Block:"
translate["ATTRIB"] = "ETIQUETA"
--------- gui_edit_text.c --------- 
translate["EDIT TEXT"] = "EDITA TEXTO"
translate["Edit a text"] = "Edita um texto"
translate["Select a text element"] = "Selecione elemento texto"
translate["Enter a new text"] = "Entre um novo texto"
translate["Edit text"] = "Edita o texto"
--------- gui_ellip.c --------- 
translate["ELLIPSE"] = "ELIPSE"
translate["Full ellipse"] = "Elipse completa"
translate["Elliptical arc"] = "Arco elíptico"
translate["Isometric Circle"] = "Círculo isométrico"
translate["Isometric Arc"] = "Arco isométrico"
translate["Place a ellipse"] = "Nova elipse"
translate["Define major axis"] = "Defina o eixo maior"
translate["Define circle radius"] = "Defina o raio"
translate["Define minor axis"] = "Defina o eixo menor"
--------- gui_explode.c --------- 
translate["EXPLODE"] = "EXPLODE"
translate["Explode elements"] = "Explode elementos"
translate["Insert"] = "Bloco"
translate["Attributes to text:"] = "Etiquetas p/ texto:"
translate["Poly Line"] = "Polilinha"
translate["Dimension"] = "Cota"
translate["M Text"] = "I Texto"
translate["Char"] = "Char"
translate["Hatch"] = "Hachura"
translate["Raw"] = "Bruto"
translate["Line"] = "Linha"
translate["Poly"] = "Polilinha"
--------- gui_export.c --------- 
translate["HP-GL files (.plt)"] = "Arquivos HP-GL (.plt)"
translate["G-Code files (.nc)"] = "Arquivos G-Code (.nc)"
translate["Position and scale"] = "Posição e escala"
translate["Origin X:"] = "Origin X:"
translate["Origin Y:"] = "Origin Y:"
translate["Scale:"] = "Escala:"
translate["Driver specific"] = "Específico do driver"
translate["Feed:"] = "Veloc:"
translate["Init:"] = "Ini:"
translate["End:"] = "Fim:"
translate["Move:"] = "Movim:"
translate["Stroke:"] = "Traço:"
translate["Output format:"] = "Formato de saída:"
translate["Destination:"] = "Destino:"
translate["Export"] = "Exporta"
translate["Export: Created export output succesfully"] = "Exporta: Saída criada com sucesso"
translate["Export Error"] = "Erro de exportação"
--------- gui_file.c --------- 
translate["File explorer"] = "Explorador de arquivos"
translate["Current directory:"] = "Diretório atual:"
translate["Up"] = "Sobe"
translate["Size"] = "Tam."
translate["Date"] = "Data"
translate["Selected:"] = "Selecionado:"
translate["File to Open:"] = "Abrir arquivo:"
translate["Explore"] = "Explora"
translate["Open Drawing"] = "Abre Desenho"
translate["Recent:"] = "Recentes:"
translate["Discard changes"] = "Descarta alterações"
translate["Changes in drawing will be lost"] = "As alterações no desenho serão perdidas"
translate["Discard"] = "Descarta"
translate["Save Drawing"] = "Salva Desenho"
translate["File to Save on:"] = "Salvar no arquivo:"
translate["Existing File"] = "Arquivo existente"
translate["Over write existing file?"] = "Sobrescrever o arquivo existente?"
--------- gui_find.c --------- 
translate["Find/Replace text"] = "Localizar/Substituir"
translate["Search:"] = "Procura:"
translate["Text"] = "Texto"
translate["MText"] = "I Texto"
translate["Find Next"] = "Loc. próximo"
translate["End of search"] = "Fim da procura"
translate["No elements matched"] = "Nenhum encontrado"
translate["Replace:"] = "Substitui:"
translate["Current"] = "Atual"
translate["REPLACE"] = "SUBSTITUIR"
translate["Entire element"] = "El. inteiro"
translate["Selection"] = "Seleção"
translate["Total replaced: %d"] = "Total de substituições: %d"
translate["All"] = "Tudo"
--------- gui_hatch.c --------- 
translate["HATCH"] = "HACHURA"
translate["Place a Hatch:"] = "Nova Hachura:"
translate["Mode:"] = "Modo:"
translate["Points"] = "Pontos"
translate["Associative"] = "Associativo"
translate["User"] = "Usuário"
translate["Library"] = "Bibl"
translate["Solid"] = "Sólido"
translate["Spacing"] = "Espaçamento"
translate["#Scale"] = "#Scale"
translate["Enter first point"] = "Entre o primeiro ponto"
translate["Enter next point"] = "Entre o próximo ponto"
translate["Hatch Pattern"] = "Trama de hachura"
translate["Family:"] = "Família:"
translate["Ref: 10 x 10 units"] = "Ref: 10 x 10 unid"
translate["#Rotation"] = "#Rotação"
translate["Choose"] = "Escolha"
translate["Hatch Pattern Library (.pat)"] = "Bibl. Tramas hachura (.pat)"
translate["Add pattern family"] = "Inclui família de tramas"
--------- gui_image.c --------- 
translate["IMAGE"] = "IMAGEM"
translate["Image PNG (.png)"] = "Imagem PNG (.png)"
translate["Image JPG (.jpg)"] = "Imagem JPG (.jpg)"
translate["Image Bitmap (.bmp)"] = "Imagem Bitmap (.bmp)"
translate["Place Raster Image"] = "Nova imagem"
translate["Proportional"] = "Proporcional"
translate["Attach"] = "Anexa"
translate["Enter first corner"] = "Primeiro canto"
translate["Enter last corner"] = "Último canto"
--------- gui_info.c --------- 
translate["Generate DB"] = "Generate DB"
translate["Cannot open database: %s\n"] = "Cannot open database: %s\n"
translate["Failed to fetch data: %s\n"] = "Failed to fetch data: %s\n"
translate["Failed to insert data: %s\n"] = "Failed to insert data: %s\n"
translate["Failed to put data\n"] = "Failed to put data\n"
translate["BLK:"] = "BLOCOS:"
translate["ENTS:"] = "ENTS:"
--------- gui_insert.c --------- 
translate["INSERT"] = "INSERT"
translate["Place a Insert"] = "Insere um bloco"
translate["Choose Block:"] = "Escolha um bloco:"
translate["Error: Block not allowed"] = "Erro: Bloco não permitido"
translate["Error: Block not found"] = "Erro: Bloco não encontrado"
translate["Enter place point"] = "Posicione o bloco"
translate["Scale"] = "Escala"
translate["Select Block"] = "Selecione o Bloco"
--------- gui_lay.c --------- 
translate["Layer Manager"] = "Gerenciador de camadas"
translate["C"] = "C"
translate["Line type"] = "Tipo de linha"
translate["Line weight"] = "Esp. Linha"
translate["Error: Don't remove Layer in use"] = "Erro: não apaga camada em uso"
translate["Remove Layer"] = "Apaga Camada"
translate["Layer Color"] = "Cor da camada"
translate["Layer Name"] = "Nome da camada"
translate["Error: Layer already exists"] = "Erro: Camada já existe"
translate["Error: exists Layer with same name"] = "Erro: existe camada com mesmo nome"
--------- gui_line.c --------- 
translate["LINE"] = "LINHA"
translate["Place a single line"] = "Nova linha simples"
--------- gui_ltype.c --------- 
translate["Line Types Manager"] = "Gerenciador de tipos de linha"
translate["Description"] = "Descrição"
translate["Preview"] = "Prévia"
translate["Add"] = "Adiciona"
translate["Error: Don't remove Line Type in use"] = "Erro: não apaga tipo linha em uso"
translate["Remove Line Type"] = "Apaga tipo linha"
translate["Global Scale Factor:"] = "Fator de escala global:"
translate["Current Object Scale:"] = "Escala elem. atual:"
translate["Line Type Name"] = "Nome do tipo de linha"
translate["Error: exists Line Type with same name"] = "Erro: existe tipo linha com mesmo nome"
translate["Add Line Type"] = "Acresc. tipo de linha"
translate["Apply Scale:"] = "Aplica escala:"
translate["Factor"] = "Fator"
translate["From:"] = "De:"
translate["Source:"] = "Origem:"
translate["Default"] = "Padrão"
translate["Extra"] = "Extra"
translate["Line Type Library (.lin)"] = "Bibl. tipos linha (.lin)"
translate["Load"] = "Abre"
translate["Preview:"] = "Prévia:"
translate["Error: Line Type already exists"] = "Erro: Tipo linha já existe"
translate["Error: Select Source Line Type"] = "Erro: Selecione tipo linha origem"
--------- gui_measure.c --------- 
translate["Measure Distance"] = "Mede distância"
translate["Distance: %.9g"] = "Distância: %.9g"
translate["X: %.9g"] = "X: %.9g"
translate["Y: %.9g"] = "Y: %.9g"
translate["Angle: %.7g"] = "Ângulo: %.7g"
translate["Points: %d"] = "Pontos: %d"
translate["Total: %.9g"] = "Total: %.9g"
translate["Current:"] = "Atual:"
translate["Last:"] = "Último:"
--------- gui_mirror.c --------- 
translate["MIRROR"] = "ESPELHO"
translate["Mirror a selection"] = "Espelha selecionados"
translate["Keep Original"] = "Mantém original"
translate["Set the reflection line"] = "Defina a linha de reflexão"
translate["Enter second point"] = "Entre o segundo ponto"
--------- gui_move.c --------- 
translate["MOVE"] = "MOVE"
translate["Move a selection"] = "Move selecionados"
--------- gui_mtext.c --------- 
translate["MTEXT"] = "TEXTO INTEL"
translate["Place an inteli text"] = "Novo texto inteligente"
translate["Text:"] = "Texto:"
translate["Text Height"] = "Altura texto"
translate["Rect width"] = "Largura"
translate["Edit inteli text"] = "Edita texto inteligente"
--------- gui_paste.c --------- 
translate["PASTE"] = "COLA"
translate["Paste a selection"] = "Cola selecionados"
--------- gui_pline.c --------- 
translate["POLYLINE"] = "POLILINHA"
translate["Place a poly line"] = "Nova Polilinha"
translate["Closed"] = "Fechada"
translate["Bulge"] = "Bojo"
--------- gui_plugins.c --------- 
translate["Plugins"] = "Extensões"
--------- gui_point.c --------- 
translate["Place a single point"] = "Novo ponto simples"
translate["POINT"] = "PONTO"
--------- gui_print.c --------- 
translate["Print"] = "Imprime"
translate["Portable Document Format (.pdf)"] = "Portable Document Format (.pdf)"
translate["Scalable Vector Graphics (.svg)"] = "Scalable Vector Graphics (.svg)"
translate["Postscript (.ps)"] = "Postscript (.ps)"
translate["Page setup"] = "Configuração de página"
translate["mm"] = "mm"
translate["inches"] = "poleg."
translate["pixels"] = "pixels"
translate["Width:"] = "Largura:"
translate["Height:"] = "Altura:"
translate["Rotate"] = "Gira"
translate["Scale & Position"] = "Posição e escala"
translate["View"] = "Vista"
translate["Fit all"] = "Ajusta"
translate["Centralize"] = "Centraliza"
translate["Color options:"] = "Opções de cor:"
translate["Monochrome"] = "Monocromático"
translate["Dark"] = "Escuro"
translate["Print: Created print output succesfully"] = "Imprime: Saída criada com sucesso"
translate["Print Error"] = "Erro de impressão"
--------- gui_prop.c --------- 
translate["Edit Properties"] = "Edita parâmetros"
translate["Select a element"] = "Selecione um elemento"
translate["Entity: %s"] = "Entidade: %s"
translate["Layer:"] = "Camada:"
translate["Ltype:"] = "Linha:"
translate["Color:"] = "Cor:"
translate["By Layer"] = "por Camada"
translate["By Block"] = "por Bloco"
translate["LW:"] = "Esp:"
translate["Modify"] = "Modifica"
translate["CHANGE PROPERTIES"] = "MUDA PARÂMETROS"
translate["Pick"] = "Pega"
translate["Angle:"] = "Ângulo:"
translate["Scale X:"] = "Escala X:"
translate["Scale Y:"] = "Escala Y:"
translate["Scale Z:"] = "Escala Z:"
translate["Choose Color"] = "Escolha a cor"
--------- gui_rect.c --------- 
translate["RECT"] = "RETANGULO"
translate["Place a rectangle"] = "Novo retângulo"
--------- gui_rotate.c --------- 
translate["ROTATE"] = "ROTACIONA"
translate["Active angle"] = "Ângulo atual"
translate["3 points"] = "3 pontos"
translate["Rotate a selection"] = "Rotaciona selecionados"
translate["Enter pivot point"] = "Entre o ponto pivô"
translate["First point"] = "Primeiro ponto"
translate["Confirm rotation"] = "Confirme a rotação"
translate["End point"] = "Ponto final"
translate["Angle=%.4g°"] = "Ângulo=%.4g°"
--------- gui_scale.c --------- 
translate["SCALE"] = "ESCALA"
translate["Active factor"] = "Fator atual"
translate["Scale a selection"] = "Escala selecionados"
translate["X factor"] = "Fator X"
translate["Y factor"] = "Fator Y"
translate["Confirm scale"] = "Confirme a escala"
translate["Factor=%.4g"] = "Fator=%.4g"
--------- gui_script.c --------- 
translate["DB client error: Open connection"] = "Erro DB remoto: Abrir conexão"
translate["DB client error: Init connection"] = "Erro DB remoto: Iniciar conexão"
translate["DB client error: Resolve host"] = "Erro DB remoto: Resolver host"
translate["DB client error: Send data to server"] = "Erro DB remoto: Envio dados ao servidor"
translate["Remote Debugger:"] = "Depurador remoto:"
translate["Host:"] = "Host:"
translate["Port:"] = "Porta:"
translate["Connect"] = "Conecta"
translate["Disconnect"] = "Desconecta"
translate["Remote"] = "Remoto"
translate["Script"] = "Script"
translate["Execute"] = "Executa"
translate["Breakpoints"] = "Pontos de parada"
translate["Variables"] = "Variáveis"
translate["Script file:"] = "Arquivo Script:"
translate["Lua Script (.lua)"] = "Script Lua (.lua)"
translate["error: %s"] = "erro: %s"
translate["Line:"] = "Linha:"
translate["Breakpoints:"] = "Pontos de parada:"
translate["On/Off"] = "Liga/Des"
translate["Breaks"] = "Paradas"
translate["On"] = "Lig"
translate["Off"] = "Des"
translate["All Globals"] = "Todos globais"
translate["All Locals"] = "Todos locais"
translate["Global:"] = "Global:"
translate["Global %s - %s\n"] = "Global %s - %s\n"
translate["Local:"] = "Local:"
translate["Local %s - %s\n"] = "Local %s - %s\n"
translate["Output:"] = "Saída:"
translate["Clear"] = "Limpa"
translate["Script %s"] = "Script %s"
--------- gui_select.c --------- 
translate["Select objects"] = "Seleciona elementos"
translate["Toggle"] = "Inverte"
translate["+"] = "+"
translate["-"] = "-"
translate["Type:"] = "Tipo:"
--------- gui_spline.c --------- 
translate["SPLINE"] = "CURVA"
translate["Control points"] = "Pontos controle"
translate["Fit points"] = "Pontos exatos"
translate["Place a spline by:"] = "Nova curva por:"
translate["Degree"] = "Grau"
--------- gui_text.c --------- 
translate["TEXT"] = "TEXTO"
translate["Place an text"] = "Novo texto simples"
--------- gui_toolbox.c --------- 
translate["Toolbox"] = "Ferramentas"
translate["Place"] = "Novo"
translate["Dimensions"] = "Cotas"
translate["XData"] = "Dados extra"
translate["Layer: "] = "Camada: "
translate["Color: "] = "Cor: "
translate["ByB"] = "Blc"
translate["ByL"] = "Cam"
translate["Line type: "] = "Tipo linha: "
translate["Line weight: "] = "Espessura: "
translate["About"] = "Sobre"
translate["By Ezequiel Rabelo de Aguiar"] = "por Ezequiel Rabelo de Aguiar"
translate["CadZinho is licensed under the MIT License."] = "CadZinho está sob a licença MIT."
translate["Build for: "] = "Plataforma: "
translate["Version: "] = "Versão: "
translate["%d of %d"] = "%d de %d"
--------- gui_tstyle.c --------- 
translate["Text Styles Manager"] = "Gerenciador de estilos de texto"
translate["Font"] = "Fonte"
translate["Subst"] = "Subst"
translate["Width"] = "Largura"
translate["Fix H"] = "Alt. fixa"
translate["Flags"] = "Flags"
translate["Error: Don't remove Standard Style"] = "Erro: Não remova o estilo Standard"
translate["Error: Don't remove Style in use"] = "Erro: Não remova estilo em uso"
translate["Remove Style"] = "Remove Estilo"
translate["Fonts"] = "Fontes"
translate["Style Name"] = "Nome do estilo"
translate["Error: Text Style already exists"] = "Erro: Estilo Texto já existe"
translate["Edit Text Style"] = "Edita Estilo Texto"
translate["Font:"] = "Fonte:"
translate["Width factor:"] = "Fator larg.:"
translate["Fixed height:"] = "Altura fixa:"
translate["Oblique angle:"] = "Âng. inclina.:"
translate["Shape"] = "Shape"
translate["Vertical"] = "Vertical"
translate["Backward"] = "Reverso"
translate["Upside down"] = "Invertido"
translate["Error: Duplicated Text Style"] = "Erro: Estilo Texto duplicado"
translate["Error: STANDARD style can't be renamed"] = "Erro: Estilo STANDARD não pode ser renomeado"
translate["Manage Fonts"] = "Gerencia Fontes"
translate["Available Fonts"] = "Fontes disponíveis"
translate["Shapes files (.shp)"] = "Aquivos Shape (.shp)"
translate["Binary shapes file (.shx)"] = "Arquivos shape binários (.shx)"
translate["True type font file (.ttf)"] = "Arquivos fonte True type (.ttf)"
translate["Open font file (.otf)"] = "Arquivos fonte Open font (.otf)"
translate["Load font"] = "Abre fonte"
--------- gui_txt_prop.c --------- 
translate["Edit Text Properties"] = "Edita parâm. de texto"
translate["Vert:"] = "Vert:"
translate["Horiz:"] = "Horiz:"
translate["Rec W:"] = "Larg.:"
translate["CHANGE TEXT PROPERTIES"] = "PARÂMETROS TEXTO"
--------- gui_vertex.c --------- 
translate["Edit Vertex"] = "Edita Vértices"
translate["Click again to modify"] = "Clique novam. p/ modificar"
translate["Select Vertex"] = "Selecione o vértice"
translate["Confirm modification"] = "Confirme a modificação"
translate["Selected vertex:"] = "Vértice selec.:"
translate["X:"] = "X:"
translate["Y:"] = "Y:"
translate["Z:"] = "Z:"
translate["Bulge:"] = "Bojo:"
translate["#Bulge"] = "#Bojo"
translate["EDIT VERTEX"] = "EDITA VERTICE"
--------- gui_xy.c --------- 
translate["X="] = "X="
translate["len="] = "dist="
translate["Y="] = "Y="
translate["ang="] = "ang="
translate["Polar"] = "Polar"
translate["Rectangular"] = "Retangular"
translate["Relative"] = "Relativo"
translate["Absolute"] = "Absoluto"
--------- gui_zoom.c --------- 
translate["Zoom in rectangle"] = "Retângulo de zoom"
translate["Enter zoom area"] = "Entre a área de zoom"
