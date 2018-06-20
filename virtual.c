#include <stdio.h>
//#include <conio.h> // windows
#include <ncurses.h> //linux
#include <string.h>
#include <stdlib.h>

#define RAM_SIZE 6
#define DISK_SIZE 4096
#define AMOUNT_OF_TABLES 32
#define FRAMES 4

int RAM[RAM_SIZE][2];
int DISK[DISK_SIZE][2];
int PAGE_TABLES[AMOUNT_OF_TABLES][3];
int LEAST_USED[RAM_SIZE] = {0, 0, 0, 0, 0, 0};
int PAGE_FAULT_COUNT = 0;
int CHANCE[RAM_SIZE] = {0, 0, 0, 0, 0, 0};
int BIT[RAM_SIZE] = {0, 0, 0, 0, 0, 0};
int OLDEST[RAM_SIZE] = {0, 0, 0, 0, 0, 0};

int ARG_P;
int ARG_M;

int ALG_TYPE;

const char *get_filename_ext(const char *filename) {
  const char *dot = strrchr(filename, '.');
  if(!dot || dot == filename) return "";
  return dot + 1;
}

void check_parameters(char *argv[], int x) {
  int i;
  for (i = 2; i < x; i++) {
    if (strcmp(argv[i], "-p") == 0) {
      ARG_P = 1;
    }
    else if (strcmp(argv[i], "-m") == 0) {
      ARG_M = 1;

    }
    else {
      printf("Opcao %s invalida!\n", argv[i]);
      exit(0);
    }
  }
}

void show_ram() {
  int i, j;
  printf("\n ________________\n");
  printf("|      RAM       |\n");
  printf("------------------\n");
  printf("|POS | P |  PAG  |\n");
  for (i = 0; i < RAM_SIZE; i++) {
    if(RAM[i][0] != -1) {
      if (i > 9) {
        printf("| %d | %d |   %d   |\n", i, RAM[i][0], RAM[i][1]);
      }
      else {
        printf("| %d  | %d |   %d   |\n", i, RAM[i][0], RAM[i][1]);
      }
    }
  }
  printf("|________________|\n");
  //  system("pause");
}
void show_pages_table(int p_id) {
  int i;
  printf("\n ____________________ \n");
  printf("| TABELA DE PAGS P%d  |\n", p_id);
  printf("|--------------------|\n");
  printf("| P |  PAG  |   HD?  |\n");
  for (i = 0; i < AMOUNT_OF_TABLES; i++) {
    if (PAGE_TABLES[i][0] == p_id) {
      printf("| %d |   %d   |   %d    |\n", PAGE_TABLES[i][0], PAGE_TABLES[i][1], PAGE_TABLES[i][2]);
    }
  }
  printf("|____________________|\n\n");
}

int leastRecentlyUsed() {
  int i, smallest, LEAST_USED[0], LRU = 0;

  for(i = 1; i < RAM_SIZE; ++i){
    if(LEAST_USED[i] < smallest){
      smallest = LEAST_USED[i];
      LRU = i;
    }
  }
  return LRU;
}

int firstIn() {
  int i, FI = 0, maior;
  maior = OLDEST[0];
  for(i = 0; i < RAM_SIZE; i++) {
    if(OLDEST[i] > maior) {
      maior = OLDEST[i];
      FI = i;
    }
  }

  return FI;
}

void create_process(int p_id, char p_op, int p_size, char* p_data_type) {
  int amount_of_pages;
  int pages[4096];
  int i, j, k;

  if (p_size % FRAMES == 0) {
    amount_of_pages = p_size/FRAMES;
  }
  else {
    amount_of_pages = p_size/FRAMES +1;
  }

  for (i = 0; i < amount_of_pages; i++) {
    pages[i] = i;
  }

  for (i = 0, j = 0; i < DISK_SIZE; i++) {
    if (DISK[i][0] == -1) { // acha uma posição vazia
      if(j < amount_of_pages) {
        DISK[i][0] = p_id;
        DISK[i][1] = pages[j];
        j++;
      }
      else {
        break;
      }
    }
  }

  // for (i = 0, j = 0; i < DISK_SIZE; i++) {
  //   if (DISK[i][0] != -1) { // acha uma posição vazia
  //   }
  // }

  //atualizar tabela de paginas (dizer que esta no HD)
  j = 0;
  for (k = 0; k < AMOUNT_OF_TABLES; k++) {
    if (PAGE_TABLES[k][0] == -1) {
      if (j < amount_of_pages) {
        PAGE_TABLES[k][0] = p_id;
        PAGE_TABLES[k][1] = pages[j];
        PAGE_TABLES[k][2] = 1; // 1 = está no disco 0 = está na memoria
        j++;
      }
    }
    else {
      continue;
    }
  }
  show_pages_table(p_id);
}

void search_process(int p_id, char* p_address) {
  int i, j, k, l, ok, fi, count=0, smallest, LRU;
  long page;

  page = strtoul(p_address, 0, 0);
  if (page % FRAMES == 0) {
    page = page/FRAMES;
  }
  else {
    if(page/FRAMES == 0) {
      page = 0;
    }
    else {
      page = page/FRAMES +1;
    }
  }

  for (i = 0; i < AMOUNT_OF_TABLES; i++) {
    if (PAGE_TABLES[i][0] == p_id && PAGE_TABLES[i][1] == page && PAGE_TABLES[i][2] == 1) {
      printf("page fault\n");

      for (k = 0; k < DISK_SIZE; k++) {
        if (DISK[k][0] == p_id && DISK[k][1] == page) {
          DISK[k][0] = -1;
          DISK[k][1] = -1;
          break;
        }
        else {
          continue;
        }
      }

      if (ALG_TYPE == 1) {
        for (j = 0; j < RAM_SIZE; j++) {
          if (RAM[j][0] == -1) {
            count++;
            PAGE_FAULT_COUNT++;
            LEAST_USED[j] = count;

            RAM[j][0] = p_id;
            RAM[j][1] = page;

            PAGE_TABLES[i][2] = 0;

            break;
          }

        }

        //RAM cheia
        if (PAGE_TABLES[i][2] == 1) {
          LRU = leastRecentlyUsed();
          count++;
          PAGE_FAULT_COUNT++;

          RAM[LRU][0] = p_id;
          RAM[LRU][1] = page;

          LEAST_USED[LRU] = count;

          PAGE_TABLES[i][2] = 0;

        }
      }
      else if (ALG_TYPE = 2) {
        for (j = 0; j < RAM_SIZE; j++) {
          if (RAM[j][0] == -1) {
            count++;
            PAGE_FAULT_COUNT++;
            LEAST_USED[j] = count;

            RAM[j][0] = p_id;
            RAM[j][1] = page;

            PAGE_TABLES[i][2] = 0;

            break;
          }

        }

        //RAM cheia
        if (PAGE_TABLES[i][2] == 1) {

          ok = 0;
          while (ok == 0) {
            LRU = leastRecentlyUsed();
            count++;
            if (BIT[LRU] == 0) {
              PAGE_FAULT_COUNT++;
              LEAST_USED[LRU] = count;
              ok = 1;
              RAM[LRU][0] = p_id;
              RAM[LRU][1] = page;
              PAGE_TABLES[i][2] = 0;
              break;
            }
            else {
              BIT[LRU] = 0;
              LEAST_USED[LRU] = count;
            }
          }

        }
      }
      else if (ALG_TYPE = 3) {

        for (j = 0; j < RAM_SIZE; j++) {
          if (RAM[j][0] == -1) {
            PAGE_FAULT_COUNT++;
            OLDEST[j] = 0;

            RAM[j][0] = p_id;
            RAM[j][1] = page;

            PAGE_TABLES[i][2] = 0;

            break;
          }
        }

        //RAM cheia
        if (PAGE_TABLES[i][2] == 1) {

          ok = 0;
          while (ok == 0) {
            fi = firstIn();
            if (CHANCE[fi] == 0) {
              PAGE_FAULT_COUNT++;
              OLDEST[fi] = 0;
              ok = 1;
              RAM[LRU][0] = p_id;
              RAM[LRU][1] = page;

              PAGE_TABLES[i][2] = 0;

              break;
            }
            else {
              CHANCE[fi] = 0;
              OLDEST[fi] = 0;
            }
          }

        }
        for (j = 0; j < RAM_SIZE; j++) {
          OLDEST[j]++;
        }
      }


    }

    else {

//      for (i = 0; i < RAM_SIZE; i++) {
        if (PAGE_TABLES[i][0] == p_id && PAGE_TABLES[i][1] == page && PAGE_TABLES[i][2] == 0) {
          printf("page hit\n");
          if (ALG_TYPE == 1) {
            count++;
            LEAST_USED[i] = count;
            break;
          }
          else if (ALG_TYPE == 2) {
            count++;
            BIT[i] = 1;
            LEAST_USED[i] = count;
            break;
          }
          else if (ALG_TYPE == 3) {

            CHANCE[i] = 1;
            break;
          }

        }

      //}

    }

  }
  show_ram();
  show_pages_table(p_id);
}

void initialize_disk() {
  int i;
  for (i = 0; i < DISK_SIZE; i++) {
    DISK[i][0] = -1;
  }
}

void initialize_ram() {
  int i;
  for (i = 0; i < RAM_SIZE; i++) {
    RAM[i][0] = -1;
    RAM[i][1] = -1;
  }
}

void initialize_page_tables() {
  int i;
  for (i = 0; i < AMOUNT_OF_TABLES; i++) {
    PAGE_TABLES[i][0] = -1;
  }
}

void main( int argc, char *argv[]) {
  initialize_disk();
  initialize_ram();
  initialize_page_tables();



  FILE *arq;
  char line[13];
  char file_name[50];
  char file_extension[5];
  char *result;
  char p_op;
  char size[5], p_data_type[2];
  int i, j, k, p_id, p_size;

  printf("Qual algoritmo? 1- LRU 2- LRU BIT 3- SECOND CHANCE :)\n");
  scanf("%d", &ALG_TYPE);

  if (argc == 1) {
    printf("Nenhum argumento oferecido!\n");
    printf("Execucao: 'nome-do-arquivo.txt [-p, -b, -m, -s]'\n");
    exit(0);
  }
  else if( argc == 2) {
    strcpy(file_extension, get_filename_ext(argv[1]));
    if (strcmp(file_extension, "txt") != 0) {
      printf("Extensao nao suportada.\n");
      exit(0);
    }
  }
  else if (argc == 3) {
    check_parameters(argv, 3);
  }
  else if (argc == 4) {
    check_parameters(argv, 4);
  }
  else if (argc == 5) {
    check_parameters(argv, 5);
  }
  else if (argc == 6) {
    check_parameters(argv, 6);
  }
  else {
    printf("Muitos argumentos!\n");
    exit(0);
  }

  strcpy(file_name, argv[1]);
  arq = fopen(file_name, "r");
  if (arq == NULL) {
    printf("Problemas na abertura do arquivo\n");
    return;
  }
  i = 1;
  while (fgets(line, sizeof line, arq) != NULL) {
    char p_address[6] = {' ', ' ', ' ', ' ', ' ', ' '};
    p_id = line[1] - '0'; //converter char em int
    p_op = line[3];
    if(p_op == 'C') {

      for (j = 5, k = 0; j < strlen(line); j++, k++) {
        size[k] = line[j];
        if (line[j] == ' ') {
          break;
        }
      }

      p_size = atoi(size);

      for (j+=1, k = 0; j < strlen(line); j++, k++) {
        p_data_type[k] = line[j];
      }
      printf("%d %c %d %s", p_id, p_op, p_size, p_data_type);
      create_process(p_id, p_op, p_size, p_data_type);
      printf("----------------------------------------------------------\n");

    }

    else if(p_op == 'R') {

      for (j = 5, k = 0; j < strlen(line); j++, k++) {
        if (line[j] != 10 || line[j] != '\n') {
          p_address[k] = line[j];
        } else { break; }
      }
      printf("%d %c %s\n", p_id, p_op, p_address);
      search_process(p_id, p_address);
      printf("----------------------------------------------------------\n");

    }

    else if(p_op == 'W') {

      for (j = 5, k = 0; j < strlen(line); j++, k++) {
        if (line[j] != 10 || line[j] != '\n') {
          p_address[k] = line[j];
        } else { break; }
      }
      printf("%d %c %s\n", p_id, p_op, p_address);
      search_process(p_id, p_address);
      printf("----------------------------------------------------------\n");

    }

  }
  fclose(arq);

  printf("\nTotal de page faults: %d\n", PAGE_FAULT_COUNT);

  if (ARG_P == 1) {
    printf("\nTamanho das páginas e quadros de memória: %d\n", FRAMES);
  }
  if(ARG_M == 1) {
    printf("\nTamanho da memória física: %d\n", RAM_SIZE);
  }
}
