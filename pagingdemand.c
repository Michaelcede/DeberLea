#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<unistd.h>
#include<string.h>
#include<errno.h>
#include<stdint.h>

const int FRAMES = 256;
const int TABLA_PAGINAS_ENTRADAS = 256;
const int TAM_PAGINA = 256;
const int MASCARA = 255;
const int TAM_MEMORIA_FISICA = 256 * 256;
const int FRAMES_ENTRADAS = 256;
const char *BACKING = "BACKING_STORE.bin";

int main(int argc, char* argv[]){

  errno = 0;

	// Validacion de cantidad de argumentos
  if(argc < 3){
    errno = EINVAL;
    perror("Argumentos");
    printf("Uso del programa: %s <ruta archivo direcciones logicas> <ruta archivo de salida>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

	// Archivos
	FILE *direcciones_logicas, *archivo_salida;
	int backing_store;
	
	//Variables 
	uint32_t direccion_logica, direccion_fisica;
	uint8_t no_pagina, offset;
  char valor;

	// Para leer el archivo
	char *linea = NULL;
	size_t len = 0;
	ssize_t bytes_leidos;

	// Tabla de paginas 
	int tabla_paginas[TABLA_PAGINAS_ENTRADAS];
	memset(tabla_paginas, -1, TABLA_PAGINAS_ENTRADAS * sizeof(int));

	int frames_libres[FRAMES_ENTRADAS];
	memset(frames_libres, -1, FRAMES_ENTRADAS * sizeof(int));

	// Memoria fisica de 256x256 bytes
	char memoria_fisica[TAM_MEMORIA_FISICA]; 

	// Abrir archivos
	archivo_salida = fopen(argv[2], "w");

	direcciones_logicas = fopen(argv[1], "r");
  if(direcciones_logicas == NULL){
    perror(argv[1]);
    exit(EXIT_FAILURE);
  }

	backing_store = open(BACKING, O_RDONLY);
  if(backing_store == -1){
    perror(BACKING);
    exit(EXIT_FAILURE);
  }

	// Leer linea por linea el archivo de direcciones
	while((bytes_leidos = getline(&linea, &len, direcciones_logicas)) != -1){ 

    direccion_logica = (uint32_t)atoi(linea);
    offset = direccion_logica & MASCARA;
    no_pagina = (direccion_logica >> 8) & MASCARA;


    if(tabla_paginas[no_pagina] != -1){

      // Se encuentra la pagina cargada
      direccion_fisica = (tabla_paginas[no_pagina] * TAM_PAGINA) + offset;
      valor = memoria_fisica[direccion_fisica];

    }else{

      // Fallo de pagina
      int i;
      for(i = 0; i < FRAMES_ENTRADAS; i++){
        if(frames_libres[i] == -1) break;
      }

      tabla_paginas[no_pagina] = i;
      frames_libres[i] = 1;

      if(lseek(backing_store, no_pagina * TAM_PAGINA, SEEK_SET) == -1){
        perror("Moviendo cursor");
        exit(EXIT_FAILURE);
      }

      for(int j = 0; j < TAM_PAGINA; j++)
        read(backing_store, &memoria_fisica[(i * TAM_PAGINA) + j], sizeof(char));

      direccion_fisica = (i * TAM_PAGINA) + offset;
      valor = memoria_fisica[direccion_fisica];

    }

		printf("Virtual address: %u Physical address: %u Value: %d\n", direccion_logica, direccion_fisica, valor);
    fprintf(archivo_salida, "Virtual address: %u Physical address: %u Value: %d\n", direccion_logica, direccion_fisica, valor);

	}

	fclose(archivo_salida);
	fclose(direcciones_logicas);

	close(backing_store);

	return EXIT_SUCCESS;
}

