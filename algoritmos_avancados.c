#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// --- Configuração da Tabela Hash ---
#define HASH_SIZE 10
#define MAX_PISTA_LEN 50
#define MAX_SUSPEITO_LEN 50

// --- 1. Árvore Binária (Mapa da Mansão) ---
typedef struct Sala {
    char nome[50];
    char pista[MAX_PISTA_LEN]; // Pista encontrada nesta sala (se houver)
    struct Sala *esquerda;
    struct Sala *direita;
} Sala;

// --- 2. Árvore de Busca (Inventário de Pistas) ---
typedef struct PistaNode {
    char pista[MAX_PISTA_LEN];
    struct PistaNode *esquerda;
    struct PistaNode *direita;
} PistaNode;

// --- 3. Tabela Hash (Vínculo Pista -> Suspeito) ---
typedef struct SuspeitoNode {
    char pista[MAX_PISTA_LEN];
    char suspeito[MAX_SUSPEITO_LEN];
    struct SuspeitoNode *next;
} SuspeitoNode;

SuspeitoNode *hashTable[HASH_SIZE];
PistaNode *inventarioBST = NULL; // Raiz da BST de pistas

// Variável global para rastrear o suspeito mais citado
char culpadoProvisorio[MAX_SUSPEITO_LEN] = "Nenhum";
int maxCitacoes = 0;

Sala* criarSala(const char* nome, const char* pista) {
    Sala* novaSala = (Sala*)malloc(sizeof(Sala));
    if (novaSala == NULL) {
        perror("Erro ao alocar memória para Sala");
        exit(EXIT_FAILURE);
    }
    strncpy(novaSala->nome, nome, 49);
    novaSala->nome[49] = '\0';
    strncpy(novaSala->pista, pista, MAX_PISTA_LEN - 1);
    novaSala->pista[MAX_PISTA_LEN - 1] = '\0';
    novaSala->esquerda = NULL;
    novaSala->direita = NULL;
    return novaSala;
}

// Montagem estática da Árvore Binária
Sala* montarMapa() {
    Sala *hall = criarSala("Hall de Entrada", "");
    
    // Nível 1
    hall->esquerda = criarSala("Sala de Jantar", "Pista: Chave Estranha");
    hall->direita = criarSala("Biblioteca", "");
    
    // Nível 2 - Esquerda
    hall->esquerda->esquerda = criarSala("Cozinha", "");
    hall->esquerda->direita = criarSala("Quarto de Hóspedes", "Pista: Bilhete Rasgado");
    
    // Nível 2 - Direita
    hall->direita->esquerda = criarSala("Escritório", "Pista: Mapa Manchado");
    hall->direita->direita = criarSala("Jardim de Inverno", "");

    // Nível 3 - Finais (Nó-Folha)
    hall->esquerda->direita->esquerda = criarSala("Banheiro Principal", "Pista: Fio de Cabelo");
    
    return hall;
}

PistaNode* criarPistaNode(const char* pista) {
    PistaNode* novo = (PistaNode*)malloc(sizeof(PistaNode));
    if (novo == NULL) {
        perror("Erro ao alocar memória para PistaNode");
        exit(EXIT_FAILURE);
    }
    strncpy(novo->pista, pista, MAX_PISTA_LEN - 1);
    novo->pista[MAX_PISTA_LEN - 1] = '\0';
    novo->esquerda = novo->direita = NULL;
    return novo;
}

PistaNode* inserirPistaBST(PistaNode* raiz, const char* pista) {
    if (raiz == NULL) {
        return criarPistaNode(pista);
    }

    if (strcmp(pista, raiz->pista) < 0) {
        raiz->esquerda = inserirPistaBST(raiz->esquerda, pista);
    } else if (strcmp(pista, raiz->pista) > 0) {
        raiz->direita = inserirPistaBST(raiz->direita, pista);
    }
    // Ignora se a pista já existe
    return raiz;
}

void exibirPistasEmOrdem(PistaNode* raiz) {
    if (raiz != NULL) {
        exibirPistasEmOrdem(raiz->esquerda);
        printf("- %s\n", raiz->pista);
        exibirPistasEmOrdem(raiz->direita);
    }
}

// Função de espalhamento simples: soma ASCII do primeiro caractere
unsigned int hash(const char *key) {
    if (!key || *key == '\0') return 0;
    return (unsigned int)(*key) % HASH_SIZE;
}

void inserirNaHash(const char *pista, const char *suspeito) {
    unsigned int index = hash(pista);
    SuspeitoNode *novo = (SuspeitoNode*)malloc(sizeof(SuspeitoNode));
    if (novo == NULL) {
        perror("Erro ao alocar memória para SuspeitoNode");
        exit(EXIT_FAILURE);
    }

    strncpy(novo->pista, pista, MAX_PISTA_LEN - 1);
    novo->pista[MAX_PISTA_LEN - 1] = '\0';
    strncpy(novo->suspeito, suspeito, MAX_SUSPEITO_LEN - 1);
    novo->suspeito[MAX_SUSPEITO_LEN - 1] = '\0';

    // Inserção com encadeamento
    novo->next = hashTable[index];
    hashTable[index] = novo;

    printf("\n[DEDUÇÃO ADICIONADA] Pista '%s' associada ao suspeito: %s\n", pista, suspeito);
}

void preencherHashInicial() {
    // Associações iniciais de Pista -> Suspeito para simulação
    inserirNaHash("Chave Estranha", "Mordomo");
    inserirNaHash("Bilhete Rasgado", "Cozinheira");
    inserirNaHash("Mapa Manchado", "Dama de Companhia");
    inserirNaHash("Fio de Cabelo", "Mordomo");
}

void analisarSuspeitos() {
    int contadores[HASH_SIZE] = {0}; // Usado para contagem simples
    char *suspeitosUnicos[HASH_SIZE]; // Array para armazenar nomes únicos

    // Inicializa o array de suspeitos únicos
    for(int i = 0; i < HASH_SIZE; i++) {
        suspeitosUnicos[i] = NULL;
    }

    int numSuspeitos = 0;

    printf("\n--- Análise de Evidências (Tabela Hash) ---\n");
    
    for (int i = 0; i < HASH_SIZE; i++) {
        SuspeitoNode *current = hashTable[i];
        while (current != NULL) {
            printf("[Hash Slot %d] Pista: %s -> Suspeito: %s\n", i, current->pista, current->suspeito);
            
            // Lógica para Contagem e Determinação do Culpado
            int found = 0;
            for(int j = 0; j < numSuspeitos; j++) {
                if(strcmp(current->suspeito, suspeitosUnicos[j]) == 0) {
                    contadores[j]++;
                    found = 1;
                    break;
                }
            }

            if(!found && numSuspeitos < HASH_SIZE) {
                // Adiciona novo suspeito único
                suspeitosUnicos[numSuspeitos] = current->suspeito; // Armazena o ponteiro para a string
                contadores[numSuspeitos] = 1;
                numSuspeitos++;
            }

            current = current->next;
        }
    }

    // Determina o suspeito mais citado
    maxCitacoes = 0;
    strncpy(culpadoProvisorio, "Nenhum", MAX_SUSPEITO_LEN - 1);
    for(int i = 0; i < numSuspeitos; i++) {
        if (contadores[i] > maxCitacoes) {
            maxCitacoes = contadores[i];
            strncpy(culpadoProvisorio, suspeitosUnicos[i], MAX_SUSPEITO_LEN - 1);
        }
    }

    printf("\n*** SUSPEITO MAIS CITADO ***\n");
    printf("Suspeito: **%s** com %d citações.\n", culpadoProvisorio, maxCitacoes);
    printf("É hora de resolver o mistério!\n");
}

void explorarSalas(Sala *atual) {
    if (atual == NULL) {
        printf("Fim do caminho. Retornando ao último ponto de decisão.\n");
        return;
    }
    
    printf("\n--- Você está em: **%s** ---\n", atual->nome);

    // Verifica e coleta pista
    if (strlen(atual->pista) > 0) {
        printf("🔎 Pista encontrada! \"%s\"\n", atual->pista);
        
        // 2. Armazena a pista na BST
        inventarioBST = inserirPistaBST(inventarioBST, atual->pista);
        
        // Simulação de associação com suspeito após encontrar a pista
        char suspeitoAssociado[MAX_SUSPEITO_LEN];
        if (strcmp(atual->pista, "Chave Estranha") == 0) {
            strcpy(suspeitoAssociado, "Mordomo");
        } else if (strcmp(atual->pista, "Bilhete Rasgado") == 0) {
            strcpy(suspeitoAssociado, "Cozinheira");
        } else if (strcmp(atual->pista, "Mapa Manchado") == 0) {
            strcpy(suspeitoAssociado, "Dama de Companhia");
        } else if (strcmp(atual->pista, "Fio de Cabelo") == 0) {
            strcpy(suspeitoAssociado, "Mordomo");
        } else {
             strcpy(suspeitoAssociado, "Jardineiro");
        }
        
        // 3. Adiciona a associação na Tabela Hash
        inserirNaHash(atual->pista, suspeitoAssociado);
        
        // Limpa a pista para não ser coletada novamente
        strcpy(atual->pista, "");
    }

    if (atual->esquerda == NULL && atual->direita == NULL) {
        printf("É um Beco Sem Saída. Você deve retornar.\n");
        return;
    }

    char escolha;
    do {
        printf("\nOpções:\n");
        if (atual->esquerda) printf("   [e] Esquerda -> %s\n", atual->esquerda->nome);
        if (atual->direita) printf("   [d] Direita -> %s\n", atual->direita->nome);
        printf("   [i] Mostrar Inventário (Pistas Coletadas)\n");
        printf("   [s] Sair e Analisar Evidências\n");
        printf("Sua escolha: ");
        scanf(" %c", &escolha);
        
        switch (escolha) {
            case 'e':
                if (atual->esquerda) {
                    explorarSalas(atual->esquerda);
                    return; // Retorna após a exploração recursiva
                } else {
                    printf("Caminho esquerdo bloqueado.\n");
                }
                break;
            case 'd':
                if (atual->direita) {
                    explorarSalas(atual->direita);
                    return; // Retorna após a exploração recursiva
                } else {
                    printf("Caminho direito bloqueado.\n");
                }
                break;
            case 'i':
                printf("\n--- INVENTÁRIO DE PISTAS (BST Ordenada) ---\n");
                if (inventarioBST == NULL) {
                    printf("O inventário está vazio.\n");
                } else {
                    exibirPistasEmOrdem(inventarioBST);
                }
                break;
            case 's':
                printf("Saindo da exploração...\n");
                return;
            default:
                printf("Opção inválida. Tente novamente.\n");
        }
    } while (escolha != 's');
}

// Função de Liberação de Memória (para evitar vazamentos)
void freeBST(PistaNode *raiz) {
    if (raiz) {
        freeBST(raiz->esquerda);
        freeBST(raiz->direita);
        free(raiz);
    }
}

void freeHash() {
    for (int i = 0; i < HASH_SIZE; i++) {
        SuspeitoNode *current = hashTable[i];
        while (current != NULL) {
            SuspeitoNode *temp = current;
            current = current->next;
            free(temp);
        }
        hashTable[i] = NULL;
    }
}

void freeSala(Sala *sala) {
    if (sala) {
        freeSala(sala->esquerda);
        freeSala(sala->direita);
        free(sala);
    }
}


int main() {
    printf("==========================================\n");
    printf("   BEM-VINDO AO DETECTIVE QUEST (Nível Mestre) \n");
    printf("==========================================\n");
    
    // Inicialização da Tabela Hash
    for (int i = 0; i < HASH_SIZE; i++) {
        hashTable[i] = NULL;
    }
    
    // 1. Monta o mapa (Árvore Binária)
    Sala *mapaRaiz = montarMapa();
    
    // Inicia a exploração
    explorarSalas(mapaRaiz);
    
    // 3. Análise Final
    printf("\n\n##########################################\n");
    printf("  FIM DA EXPLORAÇÃO: RESOLVENDO O MISTÉRIO \n");
    printf("##########################################\n");
    
    analisarSuspeitos();

    // Liberação de memória
    freeSala(mapaRaiz);
    freeBST(inventarioBST);
    freeHash();
    
    return 0;
}