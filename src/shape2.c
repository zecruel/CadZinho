#include "shape2.h"

const double shp_dir_x[16] = {1, 1, 1, 0.5, 0, -0.5, -1, -1, -1, -1, -1, -0.5, 0, 0.5, 1, 1};
const double shp_dir_y[16] = {0, 0.5, 1, 1, 1, 1, 1, 0.5, 0, -0.5, -1, -1, -1, -1, -1, -0.5};
const double shp_oct_x[8] = {1, 0.707106781, 0, -0.707106781, -1, -0.707106781, 0, 0.707106781};	
const double shp_oct_y[8] = {0, 0.707106781, 1, 0.707106781, 0, -0.707106781, -1, -0.707106781};

void shx_font_add(shape *shx_font, long num, char *name, unsigned char *cmds, unsigned int cmd_size){
	if (shx_font){
		shape *new_font = (shape *) malloc(sizeof(shape));
		if (new_font){
			new_font->num = num;
			
			new_font->name = malloc(strlen(name)+1);
			strcpy(new_font->name, name);
			
			new_font->cmds = malloc(cmd_size);
			memcpy(new_font->cmds, cmds, cmd_size);
			
			new_font->cmd_size = cmd_size;
			
			new_font->next = NULL;
			
			
			if(shx_font->next == NULL){ //verifica se a lista esta vazia
				shx_font->next=new_font;} // o novo elemento e o ultimo
			else{ // lista nao vazia
				//busca o final da lista
				shape *tmp = shx_font->next;
				while(tmp->next != NULL){
					tmp = tmp->next;}
				tmp->next = new_font; //acrescenta o novo elemento no final
			}
		}
	}
}

void shx_font_free(shape *shx_font){
	if (shx_font){
		if(shx_font->next){ //verifica se a lista esta vazia
			shape *next_shp, *current;
			
			current = shx_font->next;
			shx_font->next = NULL;
			while(current){
				next_shp = current->next;
				free(current->name);
				free(current->cmds);
				free(current);
				current = next_shp;
			}
		}
	}
}

void shx_font_print(shape *shx_font){
	int i;
	if (shx_font){
		if(shx_font->next){ //verifica se a lista esta vazia
			shape *current;
			
			current = shx_font->next;
			while(current){
				printf("%lc %d [", current->num, current->num);
				for(i=0; i < current->cmd_size; i++){
					printf("%u,", current->cmds[i]);
				}
				printf("]\n");
				current = current->next;
			}
		}
	}
}

shape *shx_font_find(shape *shx_font, long num){
	if (shx_font){ //verifica se existe a shx_font
		if(shx_font->next){ //verifica se a lista esta vazia
			//pula o primeiro shape da shx_font que eh a descricao da propria shx_font
			if(shx_font->next->next){
				shape *current;
				current = shx_font->next->next;
				while(current){
					if(current->num == num){
						return(current);}
					current = current->next;
				}
			}
		}
	}
	return(NULL);
}

shape *shx_font_open(char *path){
	FILE *file;
	shape *shx_font;
	
	int curr; //valor lido na posicao current do arquivo
	long index = 0; //posicao current no arquivo
	long next_index = 0; //indica o nextima marca a ser processada
	
	int comment = 0; //flag que indica que esta lendo os comentarios iniciais do arquivo shx
	int head = 0; //flag que indica que esta lendo o cabecalho do shape
	int name = 0; //flag que indica que esta lendo os dados do shape
	
	long num; //num do shape
	char str_tmp[255]; //armazena a string de name de cada shape
	unsigned char buffer[255]; //armazena a sequencia de cmds de cada shape
	unsigned int buf_size = 0; //tamanho da sequencia de cmds
	
	file = fopen(path,"rb"); //abre o arquivo -> somente leitura, modo binario
	if(file == NULL) {
		//perror("Erro, nao foi possivel abrir o arquivo\n");
		return(NULL);
	}
	
	// cria a lista de shapes da shx_font
	shx_font = (shape *) malloc(sizeof(shape));
	if(shx_font == NULL) {
		//perror("Erro, nao foi possivel alocar memoria\n");
		return(NULL);
	}
	shx_font->next = NULL; //indica que a lista esta vazia
	
	while ((curr = (unsigned) fgetc(file)) != EOF){ //considera o byte lido como um inteiro nao negativo
		index++;
		if (!comment){
			//busca os comentarios no inicio do arquivo
			if (curr != 26){
				//printf("%c", curr); //imprime os comentarios
				continue;
			}
			else{ //termina quando encontra o valor 26 (1A hex)
				comment = 1; //sinaliza que esta pronto
				
				//espera que o nextimo campo tenha 6 bytes de comprimento (cabecalho do arquivo)
				next_index = index + 6;
				continue;
			}
		}
		//a partir deste ponto comeca a ler as definicoes de cada letra
		// as rotinas abaixo serao repetidas para cada letra
		else if (!head){
			//busca o cabecalho da letra
			if (index != next_index){
				buffer[buf_size] = curr; //armazena no buffer
				buf_size++;
			}
			else{ //termina quando atinge o comprimento de 4 bytes (ou 6, se for a definicao da shx_font)
				//verifica se o ultimo byte eh igual a zero, senao para por erro
				if (curr != 0){
					break; //<=erro
				}
				
				//o ultimo byte eh o comprimento do campo de dados da letra
				next_index = index + buffer[buf_size - 1];
				
				// combina os dois primeiros bytes como um inteiro, que eh a especificacao unicode
				num = ((buffer[1]&255) << 8)|(buffer[0]&255);
				
				//prepara para a leitura dos dados
				head = 1;
				name = 0;
				buf_size = 0;
			}
		}	
		else if (head){
			//busca os dados da letra
			if (index != next_index){
				if (!name){ //busca o name do shape
					if (curr != 0){ //o valor 00 indica o fim do name
						str_tmp[buf_size] = curr; //armazena na string temporaria
						buf_size++;
					}
					else{
						str_tmp[buf_size] = curr; // completa a string
						
						//prepara para leitura dos cmds de desenho
						buf_size = 0;
						name = 1;
					}
				}
				else{ //cmds de desenho
					buffer[buf_size] = curr; //armazena no buffer
					buf_size++;
				}
			}
			else{ //termina quando atinge o comprimento especificado
				//verifica se o ultimo byte eh igual a zero, senao para por erro
				if (curr != 0){
					break; //<=erro
				}
				//adiciona o shape a lista da shx_font
				shx_font_add(shx_font, num, str_tmp, buffer, buf_size);
				
				// prepara para a nextima letra
				next_index = index + 4; //cabecalho com 4 bytes de compr
				head = 0;
				buf_size = 0;
			}
		}
	}
	//printf ("\nbytes: %d\n", index);
	
	fclose(file); // fecha o arquivo	
	return(shx_font); // retorna a shx_font
}

graph_obj *shx_font_parse(shape *shx_font, int pool_idx, char *txt, double *w){
	double pre_x = 0;
	double pre_y = 0;
	double px = 0;
	double py = 0;
	int pen = 1;
	int exec = 0;
	graph_obj * line_list = NULL;
	double max_x = 0;
	double max_y = 0;
	double min_x = 0;
	double min_y = 0;
	double stack_x[50];
	double stack_y[50];
	int stk_size = 0;
	
	long index = 0;
	long next_index = 0;
	int cmd = 0; //sem cmd
	int coord_y = 0; //indica que eh a coordenada y
	int bypass = 0;
	int bulge_f = 0;
	double bulge = 0;
	int arc_f = 0;
	double tmp_scale = 1.0;
	double scale = 1.0;
	
	double length, center_x, center_y, ang_ini;
	int vector, octant, direction, num_oct, radius;
	
	int str_uni[255];
	int str_size;
	
	int i, j, ii;
	
	//double height, weigth;
	
	//cria a lista de retorno
	line_list = graph_new(pool_idx);
	/*
	line_list = (line_node *) malloc(sizeof(line_node));
	if (line_list){
		line_list->next = NULL;
	}*/
	
	//converte o texto em uma string unicode
	//str_size = mbstowcs(str_uni, txt, 255); //tamanho maximo de 255
	str_size = str_utf2cp(txt, str_uni, 255);
	//printf("tamanho str = %d\n", str_size);
	//printf("str = %s\n", str_uni);

	
	for(i = 0; i < str_size; i++){
		//procura a letra na shx_font
		shape *shp = shx_font_find(shx_font, (long) str_uni[i]);
		if(shp){
			//printf("\nCod. %d\n", shp->num);
		//}
		//else: i = [42, 'asterisco', [0, 2, 14, 8, 254, 251, 33, 1, 68, 2, 46, 1, 72, 2, 65, 1, 74, 2, 68, 1, 78, 2, 47, 14, 8, 252, 253]]
		
		next_index = 0;
		for(index = 0; index < shp->cmd_size; index++){
			j = shp->cmds[index];
			if (index == next_index){
				exec = 0;
				cmd = j;
				//printf("%d,", cmd);
				
				if(cmd > 14){
					//cmd imediato de movimento
					next_index = index + 1;
					//obtem o comprimento do nibble maior
					length = (cmd & 240)/16;
					//obtem o indice do vetor do nibble menor
					vector = cmd & 15;
					// o vetor eh obtido da consultando o indice na tabela
					px = length * shp_dir_x[vector];
					py = length * shp_dir_y[vector];
					//print length, vetor
					exec = 1;
				}
				else if(cmd == 1){
					//desenho ativado
					next_index = index + 1;
					if (bypass){
						bypass = 0;
						continue;
					}
					pen = 1;
					continue;
				}
				else if(cmd == 2){
					//desenho desligado
					next_index = index + 1;
					if (bypass){ 
						bypass = 0;
						continue;
					}
					pen = 0;
					continue;
				}
				else if(cmd == 3){
					//escala divide
					next_index = index + 2;
					continue;
				}
				else if(cmd == 4){
					//escala multiplica
					next_index = index + 2;
					continue;
				}
				else if(cmd == 5){
					//salva posicao current
					next_index = index + 1;
					if(bypass){ 
						bypass = 0;
						continue;
					}
					stack_x[stk_size] = pre_x;
					stack_y[stk_size] = pre_y;
					stk_size++;
					
					continue;
				}
				else if(cmd == 6){
					//restaura posicao current
					next_index = index + 1;
					if(bypass){ 
						bypass = 0;
						continue;
					}
					if(stk_size>0){
						stk_size--;
						pre_x = stack_x[stk_size];
						pre_y = stack_y[stk_size];
					}
					continue;
				}
				else if(cmd == 7){
					//subshape
					next_index = index + 3; //unicode pula 2 bytes
					if(bypass){ 
						bypass = 0;
						continue;
					}
					//
					continue;
				}
				else if(cmd == 8){
					//uma coordenada (x,y)
					next_index = index + 3;
					coord_y = 0;
					continue;
				}
				else if(cmd == 9){
					// sequencia de coordenadas (x,y), terminada em (0,0)
					next_index = index + 3;
					coord_y = 0;
					continue;
				}
				else if(cmd == 10){
					//arc por octante
					next_index = index + 3;
					continue;
				}
				else if(cmd == 11){
					//arc fracionario
					next_index = index + 6;
					if(bypass){ 
						bypass = 0;
						continue;
					}
					//
					continue;
				}
				else if(cmd == 12){
					//arc por bulge
					next_index = index + 4;
					if(bypass){ 
						bypass = 0;
						continue;
					}
					//
					continue;
				}
				else if(cmd == 13){
					//sequencia de arcs por bulge, terminada em (0,0)
					next_index = index + 3;
					if(bypass){ 
						bypass = 0;
						continue;
					}
					//
					continue;
				}
				else if(cmd == 14){
					// salta o nextimo cmd se eh um texto horizontal
					next_index = index + 1;
					bypass = 1;
					continue;
				}
			}
			else{
				//print cmd
				if(cmd == 3){
					if(abs(j) > 0){ tmp_scale = scale/j; }
					exec = 1;
				}
				if(cmd == 4){
					tmp_scale = scale*j;
					exec = 1;
				}
				if(cmd == 8){
					if(!coord_y){
						px = (double)((signed char) j);
						coord_y = 1;
						continue;
					}
					else{
						py = (double)((signed char) j);
						coord_y = 0;
						exec = 1;
					}
				}
				else if(cmd == 9){
					if(!coord_y){
						px = (double)((signed char) j);
						coord_y = 1;
						continue;
					}
					else{
						py = (double)((signed char) j);
						if(!((px==0) && (py==0))){
							coord_y = 0;
							next_index = index + 3;
							exec = 1;
							//print px, py
						}
					}
				}
				if(cmd == 10){
					if(!coord_y){
						radius = j;
						coord_y = 1;
					}
					else{
						//obtem o primeiro octante e o sentido do nibble maior
						octant = (j & 112)/16;
						direction = (j & 128)/16;
						if(direction){ direction = -1;}
						else{ direction =1;}
						
						//obtem a quantidade de octantes do nibble menor
						num_oct = j & 15;
						if(num_oct == 0){ num_oct = 8;} //circulo completo
						
						coord_y = 0;
						arc_f = 1;
						exec = 1;
					}
				}
				if(cmd == 12){
					if(!coord_y){
						px = (double)((signed char) j);
						coord_y = 1;
						continue;
					}
					else if(!bulge_f){
						py = (double)((signed char) j);
						bulge_f = 1;
					}
					else{
						bulge = (double)((signed char) j);
						coord_y = 0;
						bulge_f = 0;
						exec = 1;
					}
				}
				else if(cmd == 13){
					if(!coord_y){
						px = (double)((signed char) j);
						coord_y = 1;
						continue;
					}
					else if(!bulge_f){
						py = (double)((signed char) j);
						if(!((px==0) && (py==0))){
							next_index = index + 2;
							bulge_f = 1;
						}
						continue;
					}
					else{
						bulge = (double)((signed char) j);
						coord_y = 0;
						next_index = index + 3;
						exec = 1;
						bulge_f = 0;
					}
				}
			}
			if(exec){
				exec = 0;
				//print cmd
				if(bypass){ 
					bypass = 0;
					tmp_scale = scale;
					arc_f = 0;
				}
				else{
					if(scale != tmp_scale){ scale = tmp_scale; }
					else if(arc_f){
						arc_f = 0;
						center_x = pre_x - radius * shp_oct_x[octant];
						center_y = pre_y - radius * shp_oct_y[octant];
						ang_ini = octant * M_PI/4;
						for(ii=1; ii <= num_oct+1; ii++){
							px = center_x + radius * cos(2 * M_PI * i * direction/ 8 + ang_ini);
							py = center_y + radius * sin(2 * M_PI * i * direction/ 8 + ang_ini);
							//line_list.append(((pre_x,pre_y),(px,py)))
							line_add(line_list, pre_x, pre_y, 0.0, px, py, 0.0);
							pre_x=px;
							pre_y=py;
						}
						//print center_x, center_y, radius
						//print octant, num_oct
					}
					else{
						px *= scale;
						py *= scale;
						if(pen){
							//adiciona a linha na lista de retorno
							line_add(line_list, pre_x, pre_y, 0.0, pre_x+px, pre_y+py, 0.0);
							
							//calcula os valores maximo e minimo de cada coordenada
							/*
							if((pre_x + px) > max_x){ max_x = (pre_x + px); }
							if((pre_y + py) > max_y){ max_y = (pre_y + py); }
							if((pre_x + px) < min_x){ min_x = (pre_x + px); }
							if((pre_y + py) < min_y){ min_y = (pre_y + py); }*/
							//max_x = max([max_x, pre_x, pre_x+px])
							//max_y = max([max_y, pre_y, pre_y+py])
							
							//printf("%3.2f,%3.2f,%3.2f,%3.2f\n", pre_x, pre_y, px, py);
						}
						pre_x += px;
						pre_y += py;
						
						/*calcula os valores maximo e minimo de cada coordenada*/
						
						max_x = (max_x > pre_x) ? max_x : pre_x;
						max_y = (max_y > pre_y) ? max_y : pre_y;
						min_x = (min_x < pre_x) ? min_x : pre_x;
						min_y = (min_y < pre_y) ? min_y : pre_y;
						//print [pre_x,pre_y]
					}
				}
			}
		}
		}//temporario ate implementar o else
	}
	//calcula a altura e a largura do texto interpretado
	/*height = fabs(max_y - min_y);
	weigth = fabs(max_x - min_x);*/
	
	//printf("Altura = %3.2f, largura = %3.2f\n", height, weigth);
	
	//return line_list, max_x, max_y
	//if (line_list){
		//line_list->ext_max_x = max_x;
		//line_list->ext_max_y = max_y;
		//line_list->ext_min_x = min_x;
		//line_list->ext_min_y = min_y;
	//}
	
	if (w != NULL) *w = max_x - min_x;
	return(line_list);
}
/*
int main (){
	setlocale(LC_ALL,""); //seta a localidade como a current do computador para aceitar acentuacao
		
	shape *txt = shx_font_open("txt.shx"); //abre a shx_font txt
	//shx_font_print(txt); //exibe os cmds
	
	lin *teste = shx_font_parse(txt, "teste«");
	lin_exibe(teste);
	
	shx_font_free(txt); //desaloca a memoria
	free(txt);
	lin_free(teste);
	free(teste);
	
	return(0);
}*/