#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX 1048576

// Estrutura para representar buracos de memória
typedef struct {
    int start; // Endereço inicial do buraco
    int end;   // Endereço final do buraco
} MemoryHole;

// Estrutura para representar blocos de memória alocados
typedef struct {
    int start;      // Endereço inicial do bloco
    int end;        // Endereço final do bloco
    char process[10]; // Nome do processo que alocou o bloco
} MemoryBlock;

MemoryHole holes[MAX];  // Array para armazenar buracos de memória
MemoryBlock blocks[MAX]; // Array para armazenar blocos de memória alocados
int num_holes = 0;  // Número atual de buracos de memória
int num_blocks = 0; // Número atual de blocos de memória alocados

// Função para solicitar um bloco contíguo de memória
void request_memory(char process[], int size, char strategy) {
    int best_index = -1;

    // Encontrar buraco de acordo com a estratégia
    for (int i = 0; i < num_holes; i++) {
        int hole_size = holes[i].end - holes[i].start + 1;
        if (hole_size >= size) {
            if (strategy == 'F' || // Primeiro ajuste
                (strategy == 'B' && (best_index == -1 || hole_size < holes[best_index].end - holes[best_index].start + 1)) || // Melhor ajuste
                (strategy == 'W' && (best_index == -1 || hole_size > holes[best_index].end - holes[best_index].start + 1))) { // Pior ajuste
                best_index = i;
                if (strategy == 'F') break; // Para no primeiro ajuste encontrado
            }
        }
    }

    if (best_index == -1) {
        printf("Erro: Memória insuficiente para alocar %d bytes para o processo %s\n", size, process);
        return;
    }

    // Alocar memória
    int start = holes[best_index].start;
    int end = start + size - 1;

    // Atualizar buraco
    if (end == holes[best_index].end) {
        // Remover buraco se totalmente utilizado
        for (int i = best_index; i < num_holes - 1; i++) {
            holes[i] = holes[i + 1];
        }
        num_holes--;
    } else {
        holes[best_index].start = end + 1; // Atualizar início do buraco restante
    }

    // Registrar alocação
    MemoryBlock block = {start, end, ""};
    strcpy(block.process, process);
    blocks[num_blocks++] = block;

    printf("Memória alocada: %s [%d:%d]\n", process, start, end);
}

// Função para liberar um bloco contíguo de memória
void release_memory(char process[]) {
    int block_index = -1;

    // Encontrar o bloco alocado para o processo
    for (int i = 0; i < num_blocks; i++) {
        if (strcmp(blocks[i].process, process) == 0) {
            block_index = i;
            break;
        }
    }

    if (block_index == -1) {
        printf("Erro: Processo %s não encontrado.\n", process);
        return;
    }

    // Liberar memória
    MemoryHole new_hole = {blocks[block_index].start, blocks[block_index].end};
    blocks[block_index] = blocks[--num_blocks]; // Remover bloco e atualizar lista

    // Combinar com buracos adjacentes
    for (int i = 0; i < num_holes; i++) {
        if (holes[i].end + 1 == new_hole.start) {
            new_hole.start = holes[i].start;
            for (int j = i; j < num_holes - 1; j++) {
                holes[j] = holes[j + 1];
            }
            num_holes--;
            i--;
        } else if (holes[i].start == new_hole.end + 1) {
            new_hole.end = holes[i].end;
            for (int j = i; j < num_holes - 1; j++) {
                holes[j] = holes[j + 1];
            }
            num_holes--;
            i--;
        }
    }

    holes[num_holes++] = new_hole;
    printf("Memória liberada: %s [%d:%d]\n", process, new_hole.start, new_hole.end);
}

// Função para compactar buracos de memória
void compact_memory() {
    int current_address = 0;

    // Mover todos os blocos de memória para o início
    for (int i = 0; i < num_blocks; i++) {
        int block_size = blocks[i].end - blocks[i].start + 1;
        blocks[i].start = current_address;
        blocks[i].end = current_address + block_size - 1;
        current_address += block_size;
    }

    // Atualizar lista de buracos
    num_holes = 1;
    holes[0].start = current_address;
    holes[0].end = MAX - 1;

    printf("Memória compactada.\n");
}

// Função para relatar o status da memória
void status_report() {
    printf("Status da memória:\n");

    // Relatar blocos alocados
    for (int i = 0; i < num_blocks; i++) {
        printf("Endereços [%d:%d] Processo %s\n", blocks[i].start, blocks[i].end, blocks[i].process);
    }

    // Relatar buracos de memória
    for (int i = 0; i < num_holes; i++) {
        printf("Endereços [%d:%d] Não utilizados\n", holes[i].start, holes[i].end);
    }
}

int main(int argc, char *argv[]) {
    
    if (argc != 2) {
        printf("Uso: ./alocador <tamanho_da_memoria>\n");
        return 1;
    }

    int memory_size = atoi(argv[1]);

    if (memory_size > MAX) {
        printf("Tamanho de memória excede o máximo permitido (%d bytes).\n", MAX);
        return 1;
    }

    // Inicializar buraco de memória
    MemoryHole initial_hole = {0, memory_size - 1};
    holes[num_holes++] = initial_hole;

    char command[50]; // Buffer para armazenar o comando

    while (1) {
        printf("alocador> ");
        fgets(command, 50, stdin);
        if (strncmp(command, "RQ", 2) == 0) {
            char process[10];
            int size;
            char strategy;
            sscanf(command + 3, "%s %d %c", process, &size, &strategy);
            request_memory(process, size, strategy);
        } else if (strncmp(command, "RL", 2) == 0) {
            char process[10];
            sscanf(command + 3, "%s", process);
            release_memory(process);
        } else if (strncmp(command, "C", 1) == 0) {
            compact_memory();
        } else if (strncmp(command, "STAT", 4) == 0) {
            status_report();
        } else if (strncmp(command, "X", 1) == 0) {
            // Sair do programa
            break;
        } else {
            printf("Comando inválido.\n");
        }
    }

    return 0;
}
