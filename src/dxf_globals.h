bmp_color dxf_colors[] = {
	[0] = { .r = 0, .g = 0, .b = 0, .a = 255 },
	[1] = { .r = 255, .g = 0, .b = 0, .a = 255 },
	[2] = { .r = 255, .g = 255, .b = 0, .a = 255 },
	[3] = { .r = 0, .g = 255, .b = 0, .a = 255 },
	[4] = { .r = 0, .g = 255, .b = 255, .a = 255 },
	[5] = { .r = 0, .g = 0, .b = 255, .a = 255 },
	[6] = { .r = 255, .g = 0, .b = 255, .a = 255 },
	[7] = { .r = 255, .g = 255, .b = 255, .a = 255 },
	[8] = { .r = 128, .g = 128, .b = 128, .a = 255 },
	[9] = { .r = 192, .g = 192, .b = 192, .a = 255 },
	[10] = { .r = 255, .g = 0, .b = 0, .a = 255 },
	[11] = { .r = 255, .g = 128, .b = 128, .a = 255 },
	[12] = { .r = 221, .g = 0, .b = 0, .a = 255 },
	[13] = { .r = 221, .g = 111, .b = 111, .a = 255 },
	[14] = { .r = 184, .g = 0, .b = 0, .a = 255 },
	[15] = { .r = 184, .g = 92, .b = 92, .a = 255 },
	[16] = { .r = 149, .g = 0, .b = 0, .a = 255 },
	[17] = { .r = 149, .g = 75, .b = 75, .a = 255 },
	[18] = { .r = 114, .g = 0, .b = 0, .a = 255 },
	[19] = { .r = 114, .g = 57, .b = 57, .a = 255 },
	[20] = { .r = 255, .g = 64, .b = 0, .a = 255 },
	[21] = { .r = 255, .g = 159, .b = 128, .a = 255 },
	[22] = { .r = 221, .g = 55, .b = 0, .a = 255 },
	[23] = { .r = 221, .g = 138, .b = 111, .a = 255 },
	[24] = { .r = 184, .g = 46, .b = 0, .a = 255 },
	[25] = { .r = 184, .g = 115, .b = 92, .a = 255 },
	[26] = { .r = 149, .g = 37, .b = 0, .a = 255 },
	[27] = { .r = 149, .g = 93, .b = 75, .a = 255 },
	[28] = { .r = 114, .g = 29, .b = 0, .a = 255 },
	[29] = { .r = 114, .g = 71, .b = 57, .a = 255 },
	[30] = { .r = 255, .g = 128, .b = 0, .a = 255 },
	[31] = { .r = 255, .g = 191, .b = 128, .a = 255 },
	[32] = { .r = 221, .g = 111, .b = 0, .a = 255 },
	[33] = { .r = 221, .g = 166, .b = 111, .a = 255 },
	[34] = { .r = 184, .g = 92, .b = 0, .a = 255 },
	[35] = { .r = 184, .g = 138, .b = 92, .a = 255 },
	[36] = { .r = 149, .g = 75, .b = 0, .a = 255 },
	[37] = { .r = 149, .g = 112, .b = 75, .a = 255 },
	[38] = { .r = 114, .g = 57, .b = 0, .a = 255 },
	[39] = { .r = 114, .g = 86, .b = 57, .a = 255 },
	[40] = { .r = 255, .g = 191, .b = 0, .a = 255 },
	[41] = { .r = 255, .g = 223, .b = 128, .a = 255 },
	[42] = { .r = 221, .g = 166, .b = 0, .a = 255 },
	[43] = { .r = 221, .g = 193, .b = 111, .a = 255 },
	[44] = { .r = 184, .g = 138, .b = 0, .a = 255 },
	[45] = { .r = 184, .g = 161, .b = 92, .a = 255 },
	[46] = { .r = 149, .g = 112, .b = 0, .a = 255 },
	[47] = { .r = 149, .g = 130, .b = 75, .a = 255 },
	[48] = { .r = 114, .g = 86, .b = 0, .a = 255 },
	[49] = { .r = 114, .g = 100, .b = 57, .a = 255 },
	[50] = { .r = 255, .g = 255, .b = 0, .a = 255 },
	[51] = { .r = 255, .g = 255, .b = 128, .a = 255 },
	[52] = { .r = 221, .g = 221, .b = 0, .a = 255 },
	[53] = { .r = 221, .g = 221, .b = 111, .a = 255 },
	[54] = { .r = 184, .g = 184, .b = 0, .a = 255 },
	[55] = { .r = 184, .g = 184, .b = 92, .a = 255 },
	[56] = { .r = 149, .g = 149, .b = 0, .a = 255 },
	[57] = { .r = 149, .g = 149, .b = 75, .a = 255 },
	[58] = { .r = 114, .g = 114, .b = 0, .a = 255 },
	[59] = { .r = 114, .g = 114, .b = 57, .a = 255 },
	[60] = { .r = 191, .g = 255, .b = 0, .a = 255 },
	[61] = { .r = 223, .g = 255, .b = 128, .a = 255 },
	[62] = { .r = 166, .g = 221, .b = 0, .a = 255 },
	[63] = { .r = 193, .g = 221, .b = 111, .a = 255 },
	[64] = { .r = 138, .g = 184, .b = 0, .a = 255 },
	[65] = { .r = 161, .g = 184, .b = 92, .a = 255 },
	[66] = { .r = 112, .g = 149, .b = 0, .a = 255 },
	[67] = { .r = 130, .g = 149, .b = 75, .a = 255 },
	[68] = { .r = 86, .g = 114, .b = 0, .a = 255 },
	[69] = { .r = 100, .g = 114, .b = 57, .a = 255 },
	[70] = { .r = 128, .g = 255, .b = 0, .a = 255 },
	[71] = { .r = 191, .g = 255, .b = 128, .a = 255 },
	[72] = { .r = 111, .g = 221, .b = 0, .a = 255 },
	[73] = { .r = 166, .g = 221, .b = 111, .a = 255 },
	[74] = { .r = 92, .g = 184, .b = 0, .a = 255 },
	[75] = { .r = 138, .g = 184, .b = 92, .a = 255 },
	[76] = { .r = 75, .g = 149, .b = 0, .a = 255 },
	[77] = { .r = 112, .g = 149, .b = 75, .a = 255 },
	[78] = { .r = 57, .g = 114, .b = 0, .a = 255 },
	[79] = { .r = 86, .g = 114, .b = 57, .a = 255 },
	[80] = { .r = 64, .g = 255, .b = 0, .a = 255 },
	[81] = { .r = 159, .g = 255, .b = 128, .a = 255 },
	[82] = { .r = 55, .g = 221, .b = 0, .a = 255 },
	[83] = { .r = 138, .g = 221, .b = 111, .a = 255 },
	[84] = { .r = 46, .g = 184, .b = 0, .a = 255 },
	[85] = { .r = 115, .g = 184, .b = 92, .a = 255 },
	[86] = { .r = 37, .g = 149, .b = 0, .a = 255 },
	[87] = { .r = 93, .g = 149, .b = 75, .a = 255 },
	[88] = { .r = 29, .g = 114, .b = 0, .a = 255 },
	[89] = { .r = 71, .g = 114, .b = 57, .a = 255 },
	[90] = { .r = 0, .g = 255, .b = 0, .a = 255 },
	[91] = { .r = 128, .g = 255, .b = 128, .a = 255 },
	[92] = { .r = 0, .g = 221, .b = 0, .a = 255 },
	[93] = { .r = 111, .g = 221, .b = 111, .a = 255 },
	[94] = { .r = 0, .g = 184, .b = 0, .a = 255 },
	[95] = { .r = 92, .g = 184, .b = 92, .a = 255 },
	[96] = { .r = 0, .g = 149, .b = 0, .a = 255 },
	[97] = { .r = 75, .g = 149, .b = 75, .a = 255 },
	[98] = { .r = 0, .g = 114, .b = 0, .a = 255 },
	[99] = { .r = 57, .g = 114, .b = 57, .a = 255 },
	[100] = { .r = 0, .g = 255, .b = 64, .a = 255 },
	[101] = { .r = 128, .g = 255, .b = 159, .a = 255 },
	[102] = { .r = 0, .g = 221, .b = 55, .a = 255 },
	[103] = { .r = 111, .g = 221, .b = 138, .a = 255 },
	[104] = { .r = 0, .g = 184, .b = 46, .a = 255 },
	[105] = { .r = 92, .g = 184, .b = 115, .a = 255 },
	[106] = { .r = 0, .g = 149, .b = 37, .a = 255 },
	[107] = { .r = 75, .g = 149, .b = 93, .a = 255 },
	[108] = { .r = 0, .g = 114, .b = 29, .a = 255 },
	[109] = { .r = 57, .g = 114, .b = 71, .a = 255 },
	[110] = { .r = 0, .g = 255, .b = 128, .a = 255 },
	[111] = { .r = 128, .g = 255, .b = 191, .a = 255 },
	[112] = { .r = 0, .g = 221, .b = 111, .a = 255 },
	[113] = { .r = 111, .g = 221, .b = 166, .a = 255 },
	[114] = { .r = 0, .g = 184, .b = 92, .a = 255 },
	[115] = { .r = 92, .g = 184, .b = 138, .a = 255 },
	[116] = { .r = 0, .g = 149, .b = 75, .a = 255 },
	[117] = { .r = 75, .g = 149, .b = 112, .a = 255 },
	[118] = { .r = 0, .g = 114, .b = 57, .a = 255 },
	[119] = { .r = 57, .g = 114, .b = 86, .a = 255 },
	[120] = { .r = 0, .g = 255, .b = 191, .a = 255 },
	[121] = { .r = 128, .g = 255, .b = 223, .a = 255 },
	[122] = { .r = 0, .g = 221, .b = 166, .a = 255 },
	[123] = { .r = 111, .g = 221, .b = 193, .a = 255 },
	[124] = { .r = 0, .g = 184, .b = 138, .a = 255 },
	[125] = { .r = 92, .g = 184, .b = 161, .a = 255 },
	[126] = { .r = 0, .g = 149, .b = 112, .a = 255 },
	[127] = { .r = 75, .g = 149, .b = 130, .a = 255 },
	[128] = { .r = 0, .g = 114, .b = 86, .a = 255 },
	[129] = { .r = 57, .g = 114, .b = 100, .a = 255 },
	[130] = { .r = 0, .g = 255, .b = 255, .a = 255 },
	[131] = { .r = 128, .g = 255, .b = 255, .a = 255 },
	[132] = { .r = 0, .g = 221, .b = 221, .a = 255 },
	[133] = { .r = 111, .g = 221, .b = 221, .a = 255 },
	[134] = { .r = 0, .g = 184, .b = 184, .a = 255 },
	[135] = { .r = 92, .g = 184, .b = 184, .a = 255 },
	[136] = { .r = 0, .g = 149, .b = 149, .a = 255 },
	[137] = { .r = 75, .g = 149, .b = 149, .a = 255 },
	[138] = { .r = 0, .g = 114, .b = 114, .a = 255 },
	[139] = { .r = 57, .g = 114, .b = 114, .a = 255 },
	[140] = { .r = 0, .g = 191, .b = 255, .a = 255 },
	[141] = { .r = 128, .g = 223, .b = 255, .a = 255 },
	[142] = { .r = 0, .g = 166, .b = 221, .a = 255 },
	[143] = { .r = 111, .g = 193, .b = 221, .a = 255 },
	[144] = { .r = 0, .g = 138, .b = 184, .a = 255 },
	[145] = { .r = 92, .g = 161, .b = 184, .a = 255 },
	[146] = { .r = 0, .g = 112, .b = 149, .a = 255 },
	[147] = { .r = 75, .g = 130, .b = 149, .a = 255 },
	[148] = { .r = 0, .g = 86, .b = 114, .a = 255 },
	[149] = { .r = 57, .g = 100, .b = 114, .a = 255 },
	[150] = { .r = 0, .g = 128, .b = 255, .a = 255 },
	[151] = { .r = 128, .g = 191, .b = 255, .a = 255 },
	[152] = { .r = 0, .g = 111, .b = 221, .a = 255 },
	[153] = { .r = 111, .g = 166, .b = 221, .a = 255 },
	[154] = { .r = 0, .g = 92, .b = 184, .a = 255 },
	[155] = { .r = 92, .g = 138, .b = 184, .a = 255 },
	[156] = { .r = 0, .g = 75, .b = 149, .a = 255 },
	[157] = { .r = 75, .g = 112, .b = 149, .a = 255 },
	[158] = { .r = 0, .g = 57, .b = 114, .a = 255 },
	[159] = { .r = 57, .g = 86, .b = 114, .a = 255 },
	[160] = { .r = 0, .g = 64, .b = 255, .a = 255 },
	[161] = { .r = 128, .g = 159, .b = 255, .a = 255 },
	[162] = { .r = 0, .g = 55, .b = 221, .a = 255 },
	[163] = { .r = 111, .g = 138, .b = 221, .a = 255 },
	[164] = { .r = 0, .g = 46, .b = 184, .a = 255 },
	[165] = { .r = 92, .g = 115, .b = 184, .a = 255 },
	[166] = { .r = 0, .g = 37, .b = 149, .a = 255 },
	[167] = { .r = 75, .g = 93, .b = 149, .a = 255 },
	[168] = { .r = 0, .g = 29, .b = 114, .a = 255 },
	[169] = { .r = 57, .g = 71, .b = 114, .a = 255 },
	[170] = { .r = 0, .g = 0, .b = 255, .a = 255 },
	[171] = { .r = 128, .g = 128, .b = 255, .a = 255 },
	[172] = { .r = 0, .g = 0, .b = 221, .a = 255 },
	[173] = { .r = 111, .g = 111, .b = 221, .a = 255 },
	[174] = { .r = 0, .g = 0, .b = 184, .a = 255 },
	[175] = { .r = 92, .g = 92, .b = 184, .a = 255 },
	[176] = { .r = 0, .g = 0, .b = 149, .a = 255 },
	[177] = { .r = 75, .g = 75, .b = 149, .a = 255 },
	[178] = { .r = 0, .g = 0, .b = 114, .a = 255 },
	[179] = { .r = 57, .g = 57, .b = 114, .a = 255 },
	[180] = { .r = 64, .g = 0, .b = 255, .a = 255 },
	[181] = { .r = 159, .g = 128, .b = 255, .a = 255 },
	[182] = { .r = 55, .g = 0, .b = 221, .a = 255 },
	[183] = { .r = 138, .g = 111, .b = 221, .a = 255 },
	[184] = { .r = 46, .g = 0, .b = 184, .a = 255 },
	[185] = { .r = 115, .g = 92, .b = 184, .a = 255 },
	[186] = { .r = 37, .g = 0, .b = 149, .a = 255 },
	[187] = { .r = 93, .g = 75, .b = 149, .a = 255 },
	[188] = { .r = 29, .g = 0, .b = 114, .a = 255 },
	[189] = { .r = 71, .g = 57, .b = 114, .a = 255 },
	[190] = { .r = 128, .g = 0, .b = 255, .a = 255 },
	[191] = { .r = 191, .g = 128, .b = 255, .a = 255 },
	[192] = { .r = 111, .g = 0, .b = 221, .a = 255 },
	[193] = { .r = 166, .g = 111, .b = 221, .a = 255 },
	[194] = { .r = 92, .g = 0, .b = 184, .a = 255 },
	[195] = { .r = 138, .g = 92, .b = 184, .a = 255 },
	[196] = { .r = 75, .g = 0, .b = 149, .a = 255 },
	[197] = { .r = 112, .g = 75, .b = 149, .a = 255 },
	[198] = { .r = 57, .g = 0, .b = 114, .a = 255 },
	[199] = { .r = 86, .g = 57, .b = 114, .a = 255 },
	[200] = { .r = 191, .g = 0, .b = 255, .a = 255 },
	[201] = { .r = 223, .g = 128, .b = 255, .a = 255 },
	[202] = { .r = 166, .g = 0, .b = 221, .a = 255 },
	[203] = { .r = 193, .g = 111, .b = 221, .a = 255 },
	[204] = { .r = 138, .g = 0, .b = 184, .a = 255 },
	[205] = { .r = 161, .g = 92, .b = 184, .a = 255 },
	[206] = { .r = 112, .g = 0, .b = 149, .a = 255 },
	[207] = { .r = 130, .g = 75, .b = 149, .a = 255 },
	[208] = { .r = 86, .g = 0, .b = 114, .a = 255 },
	[209] = { .r = 100, .g = 57, .b = 114, .a = 255 },
	[210] = { .r = 255, .g = 0, .b = 255, .a = 255 },
	[211] = { .r = 255, .g = 128, .b = 255, .a = 255 },
	[212] = { .r = 221, .g = 0, .b = 221, .a = 255 },
	[213] = { .r = 221, .g = 111, .b = 221, .a = 255 },
	[214] = { .r = 184, .g = 0, .b = 184, .a = 255 },
	[215] = { .r = 184, .g = 92, .b = 184, .a = 255 },
	[216] = { .r = 149, .g = 0, .b = 149, .a = 255 },
	[217] = { .r = 149, .g = 75, .b = 149, .a = 255 },
	[218] = { .r = 114, .g = 0, .b = 114, .a = 255 },
	[219] = { .r = 114, .g = 57, .b = 114, .a = 255 },
	[220] = { .r = 255, .g = 0, .b = 191, .a = 255 },
	[221] = { .r = 255, .g = 128, .b = 223, .a = 255 },
	[222] = { .r = 221, .g = 0, .b = 166, .a = 255 },
	[223] = { .r = 221, .g = 111, .b = 193, .a = 255 },
	[224] = { .r = 184, .g = 0, .b = 138, .a = 255 },
	[225] = { .r = 184, .g = 92, .b = 161, .a = 255 },
	[226] = { .r = 149, .g = 0, .b = 112, .a = 255 },
	[227] = { .r = 149, .g = 75, .b = 130, .a = 255 },
	[228] = { .r = 114, .g = 0, .b = 86, .a = 255 },
	[229] = { .r = 114, .g = 57, .b = 100, .a = 255 },
	[230] = { .r = 255, .g = 0, .b = 128, .a = 255 },
	[231] = { .r = 255, .g = 128, .b = 191, .a = 255 },
	[232] = { .r = 221, .g = 0, .b = 111, .a = 255 },
	[233] = { .r = 221, .g = 111, .b = 166, .a = 255 },
	[234] = { .r = 184, .g = 0, .b = 92, .a = 255 },
	[235] = { .r = 184, .g = 92, .b = 138, .a = 255 },
	[236] = { .r = 149, .g = 0, .b = 75, .a = 255 },
	[237] = { .r = 149, .g = 75, .b = 112, .a = 255 },
	[238] = { .r = 114, .g = 0, .b = 57, .a = 255 },
	[239] = { .r = 114, .g = 57, .b = 86, .a = 255 },
	[240] = { .r = 255, .g = 0, .b = 64, .a = 255 },
	[241] = { .r = 255, .g = 128, .b = 159, .a = 255 },
	[242] = { .r = 221, .g = 0, .b = 55, .a = 255 },
	[243] = { .r = 221, .g = 111, .b = 138, .a = 255 },
	[244] = { .r = 184, .g = 0, .b = 46, .a = 255 },
	[245] = { .r = 184, .g = 92, .b = 115, .a = 255 },
	[246] = { .r = 149, .g = 0, .b = 37, .a = 255 },
	[247] = { .r = 149, .g = 75, .b = 93, .a = 255 },
	[248] = { .r = 114, .g = 0, .b = 29, .a = 255 },
	[249] = { .r = 114, .g = 57, .b = 71, .a = 255 },
	[250] = { .r = 51, .g = 51, .b = 51, .a = 255 },
	[251] = { .r = 91, .g = 91, .b = 91, .a = 255 },
	[252] = { .r = 132, .g = 132, .b = 132, .a = 255 },
	[253] = { .r = 173, .g = 173, .b = 173, .a = 255 },
	[254] = { .r = 214, .g = 214, .b = 214, .a = 255 },
	[255] = { .r = 254, .g = 254, .b = 254, .a = 255 }
};

/* DXF lineweight*/
int dxf_lw[] = {0, 5, 9, 13, 15, 18, 20, 25, 30, 35, 40, 50, 53, 60, 70, 80, 90, 100, 106, 120, 140, 158, 200, 211, -1, -2};
const char *dxf_lw_descr[] = {
	"0.00 mm",
	"0.05 mm",
	"0.09 mm",
	"0.13 mm",
	"0.15 mm",
	"0.18 mm",
	"0.20 mm",
	"0.25 mm",
	"0.30 mm",
	"0.35 mm",
	"0.40 mm",
	"0.50 mm",
	"0.53 mm",
	"0.60 mm",
	"0.70 mm",
	"0.80 mm",
	"0.90 mm",
	"1.00 mm",
	"1.06 mm",
	"1.20 mm",
	"1.40 mm",
	"1.58 mm",
	"2.00 mm",
	"2.11 mm",
	"By Layer",
	"By Block"
};

#ifndef DXF_LW_LEN
	#define DXF_LW_LEN 24
#endif

/* DXF text vertical alignment definitions */
const char *text_al_v[] = {"Base Line", "Bottom", "Middle", "Top"};
#ifndef T_AL_V_LEN
	#define T_AL_V_LEN 4
#endif

/* DXF text horizontal alignment definitions */
const char *text_al_h[] = {"Left", "Center", "Right", "Aligned", "Middle", "Fit"};
#ifndef T_AL_H_LEN	
	#define T_AL_H_LEN 6
#endif

int cp1252[] = {
	0, //NULL
	1, //START OF HEADING
	2, //START OF TEXT
	3, //END OF TEXT
	4, //END OF TRANSMISSION
	5, //ENQUIRY
	6, //ACKNOWLEDGE
	7, //BELL
	8, //BACKSPACE
	9, //HORIZONTAL TABULATION
	10, //LINE FEED
	11, //VERTICAL TABULATION
	12, //FORM FEED
	13, //CARRIAGE RETURN
	14, //SHIFT OUT
	15, //SHIFT IN
	16, //DATA LINK ESCAPE
	17, //DEVICE CONTROL ONE
	18, //DEVICE CONTROL TWO
	19, //DEVICE CONTROL THREE
	20, //DEVICE CONTROL FOUR
	21, //NEGATIVE ACKNOWLEDGE
	22, //SYNCHRONOUS IDLE
	23, //END OF TRANSMISSION BLOCK
	24, //CANCEL
	25, //END OF MEDIUM
	26, //SUBSTITUTE
	27, //ESCAPE
	28, //FILE SEPARATOR
	29, //GROUP SEPARATOR
	30, //RECORD SEPARATOR
	31, //UNIT SEPARATOR
	32, //SPACE
	33, //EXCLAMATION MARK
	34, //QUOTATION MARK
	35, //NUMBER SIGN
	36, //DOLLAR SIGN
	37, //PERCENT SIGN
	38, //AMPERSAND
	39, //APOSTROPHE
	40, //LEFT PARENTHESIS
	41, //RIGHT PARENTHESIS
	42, //ASTERISK
	43, //PLUS SIGN
	44, //COMMA
	45, //HYPHEN-MINUS
	46, //FULL STOP
	47, //SOLIDUS
	48, //DIGIT ZERO
	49, //DIGIT ONE
	50, //DIGIT TWO
	51, //DIGIT THREE
	52, //DIGIT FOUR
	53, //DIGIT FIVE
	54, //DIGIT SIX
	55, //DIGIT SEVEN
	56, //DIGIT EIGHT
	57, //DIGIT NINE
	58, //COLON
	59, //SEMICOLON
	60, //LESS-THAN SIGN
	61, //EQUALS SIGN
	62, //GREATER-THAN SIGN
	63, //QUESTION MARK
	64, //COMMERCIAL AT
	65, //LATIN CAPITAL LETTER A
	66, //LATIN CAPITAL LETTER B
	67, //LATIN CAPITAL LETTER C
	68, //LATIN CAPITAL LETTER D
	69, //LATIN CAPITAL LETTER E
	70, //LATIN CAPITAL LETTER F
	71, //LATIN CAPITAL LETTER G
	72, //LATIN CAPITAL LETTER H
	73, //LATIN CAPITAL LETTER I
	74, //LATIN CAPITAL LETTER J
	75, //LATIN CAPITAL LETTER K
	76, //LATIN CAPITAL LETTER L
	77, //LATIN CAPITAL LETTER M
	78, //LATIN CAPITAL LETTER N
	79, //LATIN CAPITAL LETTER O
	80, //LATIN CAPITAL LETTER P
	81, //LATIN CAPITAL LETTER Q
	82, //LATIN CAPITAL LETTER R
	83, //LATIN CAPITAL LETTER S
	84, //LATIN CAPITAL LETTER T
	85, //LATIN CAPITAL LETTER U
	86, //LATIN CAPITAL LETTER V
	87, //LATIN CAPITAL LETTER W
	88, //LATIN CAPITAL LETTER X
	89, //LATIN CAPITAL LETTER Y
	90, //LATIN CAPITAL LETTER Z
	91, //LEFT SQUARE BRACKET
	92, //REVERSE SOLIDUS
	93, //RIGHT SQUARE BRACKET
	94, //CIRCUMFLEX ACCENT
	95, //LOW LINE
	96, //GRAVE ACCENT
	97, //LATIN SMALL LETTER A
	98, //LATIN SMALL LETTER B
	99, //LATIN SMALL LETTER C
	100, //LATIN SMALL LETTER D
	101, //LATIN SMALL LETTER E
	102, //LATIN SMALL LETTER F
	103, //LATIN SMALL LETTER G
	104, //LATIN SMALL LETTER H
	105, //LATIN SMALL LETTER I
	106, //LATIN SMALL LETTER J
	107, //LATIN SMALL LETTER K
	108, //LATIN SMALL LETTER L
	109, //LATIN SMALL LETTER M
	110, //LATIN SMALL LETTER N
	111, //LATIN SMALL LETTER O
	112, //LATIN SMALL LETTER P
	113, //LATIN SMALL LETTER Q
	114, //LATIN SMALL LETTER R
	115, //LATIN SMALL LETTER S
	116, //LATIN SMALL LETTER T
	117, //LATIN SMALL LETTER U
	118, //LATIN SMALL LETTER V
	119, //LATIN SMALL LETTER W
	120, //LATIN SMALL LETTER X
	121, //LATIN SMALL LETTER Y
	122, //LATIN SMALL LETTER Z
	123, //LEFT CURLY BRACKET
	124, //VERTICAL LINE
	125, //RIGHT CURLY BRACKET
	126, //TILDE
	127, //DELETE
	8364, //EURO SIGN
	0, //UNDEFINED
	8218, //SINGLE LOW-9 QUOTATION MARK
	402, //LATIN SMALL LETTER F WITH HOOK
	8222, //DOUBLE LOW-9 QUOTATION MARK
	8230, //HORIZONTAL ELLIPSIS
	8224, //DAGGER
	8225, //DOUBLE DAGGER
	710, //MODIFIER LETTER CIRCUMFLEX ACCENT
	8240, //PER MILLE SIGN
	352, //LATIN CAPITAL LETTER S WITH CARON
	8249, //SINGLE LEFT-POINTING ANGLE QUOTATION MARK
	338, //LATIN CAPITAL LIGATURE OE
	0, //UNDEFINED
	381, //LATIN CAPITAL LETTER Z WITH CARON
	0, //UNDEFINED
	0, //UNDEFINED
	8216, //LEFT SINGLE QUOTATION MARK
	8217, //RIGHT SINGLE QUOTATION MARK
	8220, //LEFT DOUBLE QUOTATION MARK
	8221, //RIGHT DOUBLE QUOTATION MARK
	8226, //BULLET
	8211, //EN DASH
	8212, //EM DASH
	732, //SMALL TILDE
	8482, //TRADE MARK SIGN
	353, //LATIN SMALL LETTER S WITH CARON
	8250, //SINGLE RIGHT-POINTING ANGLE QUOTATION MARK
	339, //LATIN SMALL LIGATURE OE
	0, //UNDEFINED
	382, //LATIN SMALL LETTER Z WITH CARON
	376, //LATIN CAPITAL LETTER Y WITH DIAERESIS
	160, //NO-BREAK SPACE
	161, //INVERTED EXCLAMATION MARK
	162, //CENT SIGN
	163, //POUND SIGN
	164, //CURRENCY SIGN
	165, //YEN SIGN
	166, //BROKEN BAR
	167, //SECTION SIGN
	168, //DIAERESIS
	169, //COPYRIGHT SIGN
	170, //FEMININE ORDINAL INDICATOR
	171, //LEFT-POINTING DOUBLE ANGLE QUOTATION MARK
	172, //NOT SIGN
	173, //SOFT HYPHEN
	174, //REGISTERED SIGN
	175, //MACRON
	176, //DEGREE SIGN
	177, //PLUS-MINUS SIGN
	178, //SUPERSCRIPT TWO
	179, //SUPERSCRIPT THREE
	180, //ACUTE ACCENT
	181, //MICRO SIGN
	182, //PILCROW SIGN
	183, //MIDDLE DOT
	184, //CEDILLA
	185, //SUPERSCRIPT ONE
	186, //MASCULINE ORDINAL INDICATOR
	187, //RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK
	188, //VULGAR FRACTION ONE QUARTER
	189, //VULGAR FRACTION ONE HALF
	190, //VULGAR FRACTION THREE QUARTERS
	191, //INVERTED QUESTION MARK
	192, //LATIN CAPITAL LETTER A WITH GRAVE
	193, //LATIN CAPITAL LETTER A WITH ACUTE
	194, //LATIN CAPITAL LETTER A WITH CIRCUMFLEX
	195, //LATIN CAPITAL LETTER A WITH TILDE
	196, //LATIN CAPITAL LETTER A WITH DIAERESIS
	197, //LATIN CAPITAL LETTER A WITH RING ABOVE
	198, //LATIN CAPITAL LETTER AE
	199, //LATIN CAPITAL LETTER C WITH CEDILLA
	200, //LATIN CAPITAL LETTER E WITH GRAVE
	201, //LATIN CAPITAL LETTER E WITH ACUTE
	202, //LATIN CAPITAL LETTER E WITH CIRCUMFLEX
	203, //LATIN CAPITAL LETTER E WITH DIAERESIS
	204, //LATIN CAPITAL LETTER I WITH GRAVE
	205, //LATIN CAPITAL LETTER I WITH ACUTE
	206, //LATIN CAPITAL LETTER I WITH CIRCUMFLEX
	207, //LATIN CAPITAL LETTER I WITH DIAERESIS
	208, //LATIN CAPITAL LETTER ETH
	209, //LATIN CAPITAL LETTER N WITH TILDE
	210, //LATIN CAPITAL LETTER O WITH GRAVE
	211, //LATIN CAPITAL LETTER O WITH ACUTE
	212, //LATIN CAPITAL LETTER O WITH CIRCUMFLEX
	213, //LATIN CAPITAL LETTER O WITH TILDE
	214, //LATIN CAPITAL LETTER O WITH DIAERESIS
	215, //MULTIPLICATION SIGN
	216, //LATIN CAPITAL LETTER O WITH STROKE
	217, //LATIN CAPITAL LETTER U WITH GRAVE
	218, //LATIN CAPITAL LETTER U WITH ACUTE
	219, //LATIN CAPITAL LETTER U WITH CIRCUMFLEX
	220, //LATIN CAPITAL LETTER U WITH DIAERESIS
	221, //LATIN CAPITAL LETTER Y WITH ACUTE
	222, //LATIN CAPITAL LETTER THORN
	223, //LATIN SMALL LETTER SHARP S
	224, //LATIN SMALL LETTER A WITH GRAVE
	225, //LATIN SMALL LETTER A WITH ACUTE
	226, //LATIN SMALL LETTER A WITH CIRCUMFLEX
	227, //LATIN SMALL LETTER A WITH TILDE
	228, //LATIN SMALL LETTER A WITH DIAERESIS
	229, //LATIN SMALL LETTER A WITH RING ABOVE
	230, //LATIN SMALL LETTER AE
	231, //LATIN SMALL LETTER C WITH CEDILLA
	232, //LATIN SMALL LETTER E WITH GRAVE
	233, //LATIN SMALL LETTER E WITH ACUTE
	234, //LATIN SMALL LETTER E WITH CIRCUMFLEX
	235, //LATIN SMALL LETTER E WITH DIAERESIS
	236, //LATIN SMALL LETTER I WITH GRAVE
	237, //LATIN SMALL LETTER I WITH ACUTE
	238, //LATIN SMALL LETTER I WITH CIRCUMFLEX
	239, //LATIN SMALL LETTER I WITH DIAERESIS
	240, //LATIN SMALL LETTER ETH
	241, //LATIN SMALL LETTER N WITH TILDE
	242, //LATIN SMALL LETTER O WITH GRAVE
	243, //LATIN SMALL LETTER O WITH ACUTE
	244, //LATIN SMALL LETTER O WITH CIRCUMFLEX
	245, //LATIN SMALL LETTER O WITH TILDE
	246, //LATIN SMALL LETTER O WITH DIAERESIS
	247, //DIVISION SIGN
	248, //LATIN SMALL LETTER O WITH STROKE
	249, //LATIN SMALL LETTER U WITH GRAVE
	250, //LATIN SMALL LETTER U WITH ACUTE
	251, //LATIN SMALL LETTER U WITH CIRCUMFLEX
	252, //LATIN SMALL LETTER U WITH DIAERESIS
	253, //LATIN SMALL LETTER Y WITH ACUTE
	254, //LATIN SMALL LETTER THORN
	255, //LATIN SMALL LETTER Y WITH DIAERESIS
};

struct page_def pages_iso_a[] = {
	{PRT_MM, 1682, 2378, "4A0"},
	{PRT_MM, 1189, 1682, "2A0"},
	{PRT_MM, 841, 1189, "A0"},
	{PRT_MM, 594, 841, "A1"},
	{PRT_MM, 420, 594, "A2"},
	{PRT_MM, 297, 420, "A3"},
	{PRT_MM, 210, 297, "A4"},
	{PRT_MM, 148, 210, "A5"},
	{PRT_MM, 105, 148, "A6"},
	{PRT_MM, 74, 105, "A7"},
	{PRT_MM, 52, 74, "A8"},
	{PRT_MM, 37, 52, "A9"},
	{PRT_MM, 26, 37, "A10"},
};

struct page_def pages_iso_b[] = {
	{PRT_MM, 1000, 1414, "B0"},
	{PRT_MM, 707, 1000, "B1"},
	{PRT_MM, 500, 707, "B2"},
	{PRT_MM, 353, 500, "B3"},
	{PRT_MM, 250, 353, "B4"},
	{PRT_MM, 176, 250, "B5"},
	{PRT_MM, 125, 176, "B6"},
	{PRT_MM, 88, 125, "B7"},
	{PRT_MM, 62, 88, "B8"},
	{PRT_MM, 44, 62, "B9"},
	{PRT_MM, 31, 44, "B10"},
};

struct page_def pages_us[] = {
	{PRT_IN, 5.5, 8.5, "Half Letter"},
	{PRT_IN, 8.5, 11.0, "Letter"},
	{PRT_IN, 8.5, 14.0, "Legal"},
	{PRT_IN, 5.0, 8.0, "Junior Legal"},
	{PRT_IN, 11.0, 17.0, "Ledger / Tabloid"},

};

struct page_def pages_ansi[] = {
	{PRT_IN, 8.5, 11.0, "A"},
	{PRT_IN, 11.0, 17.0, "B"},
	{PRT_IN, 17.0, 22.0, "C"},
	{PRT_IN, 22.0, 34.0, "D"},
	{PRT_IN, 34.0, 44.0, "E"},
};

struct page_def pages_arch[] = {
	{PRT_IN, 9.0, 12.0, "Arch A"},
	{PRT_IN, 12.0, 18.0, "Arch B"},
	{PRT_IN, 18.0, 24.0, "Arch C"},
	{PRT_IN, 24.0, 36.0, "Arch D"},
	{PRT_IN, 36.0, 48.0, "Arch E"},
	{PRT_IN, 30.0, 42.0, "Arch E1"},
};

struct page_def pages_image[] = {
	{PRT_PX, 1280, 1024, "1280x1024"},
	{PRT_PX, 1024, 768, "1024x768"},
	{PRT_PX, 800, 600, "800x600"},
	{PRT_PX, 640, 480, "640x480"},
};


struct page_def *fam_pages[] = {
	pages_iso_a,
	pages_iso_b,
	pages_us,
	pages_ansi,
	pages_arch,
	pages_image,
};

const char *fam_pages_descr[] = {"ISO A", "ISO B", "US", "ANSI", "ARCH", "IMAGE"};
const int fam_pages_len[] = {13, 11, 5, 5, 6, 4};

struct func_key {
	char key[15];
	SDL_Keycode code;
	SDL_Keymod mod;
};

struct func_key func_keys[] = {
	{ "f1", SDLK_F1, KMOD_NONE},
	{ "f2", SDLK_F2, KMOD_NONE},
	{ "f3", SDLK_F3, KMOD_NONE},
	{ "f4", SDLK_F4, KMOD_NONE},
	{ "f5", SDLK_F5, KMOD_NONE},
	{ "f6", SDLK_F6, KMOD_NONE},
	{ "f7", SDLK_F7, KMOD_NONE},
	{ "f8", SDLK_F8, KMOD_NONE},
	{ "f9", SDLK_F9, KMOD_NONE},
	{ "f10", SDLK_F10, KMOD_NONE},
	{ "f11", SDLK_F11, KMOD_NONE},
	{ "f12", SDLK_F12, KMOD_NONE},
	
	{ "ctrl_f1", SDLK_F1, KMOD_CTRL},
	{ "ctrl_f2", SDLK_F2, KMOD_CTRL},
	{ "ctrl_f3", SDLK_F3, KMOD_CTRL},
	{ "ctrl_f4", SDLK_F4, KMOD_CTRL},
	{ "ctrl_f5", SDLK_F5, KMOD_CTRL},
	{ "ctrl_f6", SDLK_F6, KMOD_CTRL},
	{ "ctrl_f7", SDLK_F7, KMOD_CTRL},
	{ "ctrl_f8", SDLK_F8, KMOD_CTRL},
	{ "ctrl_f9", SDLK_F9, KMOD_CTRL},
	{ "ctrl_f10", SDLK_F10, KMOD_CTRL},
	{ "ctrl_f11", SDLK_F11, KMOD_CTRL},
	{ "ctrl_f12", SDLK_F12, KMOD_CTRL},
	
	{ "shift_f1", SDLK_F1, KMOD_SHIFT},
	{ "shift_f2", SDLK_F2, KMOD_SHIFT},
	{ "shift_f3", SDLK_F3, KMOD_SHIFT},
	{ "shift_f4", SDLK_F4, KMOD_SHIFT},
	{ "shift_f5", SDLK_F5, KMOD_SHIFT},
	{ "shift_f6", SDLK_F6, KMOD_SHIFT},
	{ "shift_f7", SDLK_F7, KMOD_SHIFT},
	{ "shift_f8", SDLK_F8, KMOD_SHIFT},
	{ "shift_f9", SDLK_F9, KMOD_SHIFT},
	{ "shift_f10", SDLK_F10, KMOD_SHIFT},
	{ "shift_f11", SDLK_F11, KMOD_SHIFT},
	{ "shift_f12", SDLK_F12, KMOD_SHIFT},
};

const char *func_key_dflt_file = "-- CadZinho function keys script file\n"
"-- This script is executed each time a function key (F1 - F12) is pressed\n"
"-- Key modifiers (ctrl and shift) are also considered\n"
"-- This file is writen in Lua language\n\n"
"-- Put custom code snipet in space after desired function key\n"
"-- Use cadzinho.exec_file(\"file.lua\") to call another script file (instead standard Lua dofile())\n"
"--NOTE: For Windows, strings with path dir separator '\\' must be escaped, eg. \"C:\\\\mydir\\\\myfile.lua\"\n\n"
"if function_key == \"f1\" then   -- don't alter this line\n    \n"
"elseif function_key == \"f2\" then   -- don't alter this line\n    \n"
"elseif function_key == \"f3\" then   -- don't alter this line\n    \n"
"elseif function_key == \"f4\" then   -- don't alter this line\n    \n"
"elseif function_key == \"f5\" then   -- don't alter this line\n    \n"
"elseif function_key == \"f6\" then   -- don't alter this line\n    \n"
"elseif function_key == \"f7\" then   -- don't alter this line\n    \n"
"elseif function_key == \"f8\" then   -- don't alter this line\n    \n"
"elseif function_key == \"f9\" then   -- don't alter this line\n    \n"
"elseif function_key == \"f10\" then   -- don't alter this line\n    \n"
"elseif function_key == \"f11\" then   -- don't alter this line\n    \n"
"elseif function_key == \"f12\" then   -- don't alter this line\n    \n"
"-- Ctrl modifier\n"
"elseif function_key == \"ctrl_f1\" then   -- don't alter this line\n    \n"
"elseif function_key == \"ctrl_f2\" then   -- don't alter this line\n    \n"
"elseif function_key == \"ctrl_f3\" then   -- don't alter this line\n    \n"
"elseif function_key == \"ctrl_f4\" then   -- don't alter this line\n    \n"
"elseif function_key == \"ctrl_f5\" then   -- don't alter this line\n    \n"
"elseif function_key == \"ctrl_f6\" then   -- don't alter this line\n    \n"
"elseif function_key == \"ctrl_f7\" then   -- don't alter this line\n    \n"
"elseif function_key == \"ctrl_f8\" then   -- don't alter this line\n    \n"
"elseif function_key == \"ctrl_f9\" then   -- don't alter this line\n    \n"
"elseif function_key == \"ctrl_f10\" then   -- don't alter this line\n    \n"
"elseif function_key == \"ctrl_f11\" then   -- don't alter this line\n    \n"
"elseif function_key == \"ctrl_f12\" then   -- don't alter this line\n    \n"
"-- Shift modifier\n"
"elseif function_key == \"shift_f1\" then   -- don't alter this line\n    \n"
"elseif function_key == \"shift_f2\" then   -- don't alter this line\n    \n"
"elseif function_key == \"shift_f3\" then   -- don't alter this line\n    \n"
"elseif function_key == \"shift_f4\" then   -- don't alter this line\n    \n"
"elseif function_key == \"shift_f5\" then   -- don't alter this line\n    \n"
"elseif function_key == \"shift_f6\" then   -- don't alter this line\n    \n"
"elseif function_key == \"shift_f7\" then   -- don't alter this line\n    \n"
"elseif function_key == \"shift_f8\" then   -- don't alter this line\n    \n"
"elseif function_key == \"shift_f9\" then   -- don't alter this line\n    \n"
"elseif function_key == \"shift_f10\" then   -- don't alter this line\n    \n"
"elseif function_key == \"shift_f11\" then   -- don't alter this line\n    \n"
"elseif function_key == \"shift_f12\" then   -- don't alter this line\n    \n"
"end    --don't alter this line";
	
const char *macro_dflt_file = "-- CadZinho macro script file\n"
"-- This script is loaded at startup and executed each time a text macro is entered in keyboard\n"
"-- This file is writen in Lua language\n\n"
"-- See typical code snipet for each macro\n"
"-- Use cadzinho.exec_file(\"file.lua\") to call another script file (instead standard Lua dofile())\n"
"--NOTE: For Windows, strings with path dir separator '\' must be escaped, eg. \"C:\\\\mydir\\\\myfile.lua\"\n\n"
"--Typical snipet\n"
"if macro == \"dd\" then   -- Macro is \"dd\"\n"
"    cadzinho.set_modal(\"duplicate\")    -- Action to execute\n"
"    accept = true    -- Action accepted. Clear text buffer for macros\n"
"elseif macro == \"aa\" then   -- Another macro \"aa\"\n"
"    cadzinho.set_modal(\"move\")    -- Another action to execute\n"
"    accept = true    -- Action accepted. Clear text buffer for macros\n"
"elseif macro == \"ss\" then\n"
"    cadzinho.set_modal(\"select\")\n"
"    accept = true\n"
"elseif macro == \"sd\" then\n"
"    cadzinho.set_modal(\"line\")\n"
"    accept = true\n"
"elseif macro == \"sa\" then\n"
"    cadzinho.set_modal(\"polyline\")\n"
"    accept = true\n"
"elseif macro == \"ad\" then\n"
"    cadzinho.set_modal(\"rect\")\n"
"    accept = true\n"
"elseif macro == \"da\" then\n"
"    cadzinho.set_modal(\"text\")\n"
"    accept = true\n"
"elseif macro == \"asd\" then\n"
"    cadzinho.set_modal(\"rotate\")\n"
"    accept = true\n"
"elseif macro == \"dsa\" then\n"
"    cadzinho.set_modal(\"scale\")\n"
"    accept = true\n"
"elseif macro == \"sw\" then\n"
"    cadzinho.set_modal(\"circle\")\n"
"    accept = true\n"
"else    --None action accepted\n"
"    accept = false   -- Wait for more text entry. Buffer is cleared after timeout (3s)\n"
"end    --end \"if\" statement\n\n";
