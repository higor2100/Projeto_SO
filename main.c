#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>


//**************************
// static const char *filepath = "/file";
// static const char *filename = "file";
// static const char *filecontent = "I'm the content of the only file available there\n";

// static int getattr_callback(const char *path, struct stat *stbuf) {
//   memset(stbuf, 0, sizeof(struct stat));

//   if (strcmp(path, "/") == 0) {
//     stbuf->st_mode = S_IFDIR | 0755;
//     stbuf->st_nlink = 2;
//     return 0;
//   }

//   if (strcmp(path, filepath) == 0) {
//     stbuf->st_mode = S_IFREG | 0777;
//     stbuf->st_nlink = 1;
//     stbuf->st_size = strlen(filecontent);
//     return 0;
//   }

//   return -ENOENT;
// }

// static int readdir_callback(const char *path, void *buf, fuse_fill_dir_t filler,
//     off_t offset, struct fuse_file_info *fi) {
//   (void) offset;
//   (void) fi;

//   filler(buf, ".", NULL, 0);
//   filler(buf, "..", NULL, 0);

//   filler(buf, filename, NULL, 0);

//   return 0;
// }

// static int open_callback(const char *path, struct fuse_file_info *fi) {
//   return 0;
// }

// static int read_callback(const char *path, char *buf, size_t size, off_t offset,
//     struct fuse_file_info *fi) {

//   if (strcmp(path, filepath) == 0) {
//     size_t len = strlen(filecontent);
//     if (offset >= len) {
//       return 0;
//     }

//     if (offset + size > len) {
//       memcpy(buf, filecontent + offset, len - offset);
//       return len - offset;
//     }

//     memcpy(buf, filecontent + offset, size);
//     return size;
//   }

//   return -ENOENT;
// }

// static struct fuse_operations fuse_example_operations = {
//   .getattr = getattr_callback,
//   .open = open_callback,
//   .read = read_callback,
//   .readdir = readdir_callback,
// };

//**************************
int get_imagem_para_deslocamento(FILE* bmp_deslocamento) {
	fseek(bmp_deslocamento,10,0);
	int deslocamento;
	deslocamento=(int)fgetc(bmp_deslocamento);
	return deslocamento;
}

int get_mensagem_length(FILE *fp) {
	fseek(fp, 0L, SEEK_END);
	int tamanho = ftell(fp);
	fseek(fp, 0L, SEEK_SET);
	return(tamanho);
}
int get_bit(char o_byte,int outro_bit) {
	return((o_byte>>8-outro_bit)&1);
}

int main(int argc,char** argv) {



	unsigned char tabela_mascarada[] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };

	FILE *id_arquivo;
	FILE *id_msg;
	FILE *id_msg_escondida;

	if(argc!=5) {
		printf("*** Esteganografia por substituição *** \n Uso:%s [-e] [-d] <arquivo de origem> <arquivo de destino> <arquivo de texto> \n e: Adicionar texto à imagem \n -d: Obter texto de imagem \n",argv[0]);
		exit(1);
	}

	int modo;
	if(!strcmp(argv[1],"-e"))
		modo=1;
	else if(!strcmp(argv[1],"-d"))
		modo=0;
	else {
		printf("*** Esteganografia por substituição *** \nUso:%s <modo> <arquivo de origem> <arquivo de destino> <arquivo de texto> \nModo - e = criptografar \n d = descriptografar \n",argv[0]);
		exit(1);
	}

	/* MANUSEIO DE ABERTURA DE ARQUIVO E ERROS */
	id_arquivo=fopen(argv[2],"r");
	if (id_arquivo == NULL) {
		fprintf(stderr, "Não é possível abrir o arquivo de entrada %s \n",argv[2]);
		exit(1);
	}

	id_msg_escondida=fopen(argv[3],"w");
	if (id_msg_escondida== NULL) {
		fprintf(stderr, "Não é possível criar arquivo de saída %s\n",argv[3]);
		exit(1);
	}
	

	int c=0;

	/* Gera arquivo com o mesmo cabeçalho. Copie os primeiros 128 bytes */
	char tmp_sig_cpy;
	int deslocamento=get_imagem_para_deslocamento(id_arquivo);

	rewind(id_arquivo);

	for(int i=0;i<deslocamento;i++) {
		tmp_sig_cpy=fgetc(id_arquivo);
		fputc(tmp_sig_cpy,id_msg_escondida);
		c++;
	}
	/*Arquivo criado como .bmp */

	char arq_buffer; 			// Variável temporária para um byte do arquivo
	char msg_buffer;		// Buffer temporário para um byte de mensagem

	if(modo) {
		id_msg=fopen(argv[4],"r");
		if (id_msg== NULL) {
			fprintf(stderr, "Não é possível abrir o arquivo de entrada de texto %s\n",argv[4]);
			exit(1);
		}
		int msg_escondida_length=get_mensagem_length(id_msg);


	/* 
	Depois que o deslocamento foi lido e o cabeçalho do arquivo foi escrito como está para a imagem virgem - o comprimento da mensagem oculta é escrito como o primeiro byte. Esse comprimento é então usado ao descriptografar o texto da imagem.
	*/
		fputc(msg_escondida_length,id_msg_escondida);
		c++;
		do {
			int pedaco_msg;
			if(!feof(id_msg)) {		
				msg_buffer=fgetc(id_msg);
				for(int i=1;i<=8;i++) {  // Faça isso para cada bit em cada byte da imagem virgem

					arq_buffer=fgetc(id_arquivo);
					c++;
					int arq_byte = arq_buffer & 1; // E COM 1 PARA OBTER O VALOR DO BIT. E FAZ 0 SE O BIT FOR 0 OU 1 SE FOR 1

					pedaco_msg=get_bit(msg_buffer,i);
					//pedaco_msg=tabela_mascarada[i] & msg_buffer;
					if(arq_byte==pedaco_msg) {
						fputc(arq_buffer,id_msg_escondida);
					}
					else {
						if(arq_byte==0)
							arq_buffer = (arq_buffer | 1);
						else
							arq_buffer = (arq_buffer & ~1);
						// lógica para inverter o bit de arq_buffer e colocá-lo em um arquivo com putc ()
						fputc(arq_buffer,id_msg_escondida);
					}
				}
			}
			else {
				tmp_sig_cpy=fgetc(id_arquivo);
				fputc(tmp_sig_cpy,id_msg_escondida);
				c++;
			}
		} while(!feof(id_arquivo));	
		fclose(id_msg);	
	}
	else {
		id_msg=fopen(argv[4],"w");
		if (id_msg== NULL) {
			fprintf(stderr,"Não é possível abrir o arquivo de entrada de texto%s\n",argv[4]);
			exit(1);
		}
	
		/* Grab BIT de todos os bytes para o comprimento especificado em fgetc */
		int msg_length=fgetc(id_arquivo);
		for(int i=0;i<msg_length;i++) {
			char temp_ch='\0';
			for( int j=0;j<8;j++) {
				temp_ch=temp_ch<<1;
				arq_buffer=fgetc(id_arquivo);
				int arq_byte = arq_buffer & 1; 
				temp_ch|=arq_byte;
			}
			fputc(temp_ch,id_msg);
		}
		fclose(id_msg);	
	}

	/* Limpe antes de sair */
	fclose(id_arquivo);
	fclose(id_msg_escondida);
	// return fuse_main(argc, argv, &fuse_example_operations, NULL);

}