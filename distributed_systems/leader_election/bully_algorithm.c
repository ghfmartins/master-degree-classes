#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define NUM_PROC 5
#define BASE_PORT 5000
#define BUFFER_SIZE 256

typedef struct {
    int id;
    int port;
    bool ativo;
    bool is_lider;
} Processo;

Processo processos[NUM_PROC];
int meu_id;
int lider_atual = -1;

void enviar_mensagem(int destino_id, const char *mensagem) {
    int sockfd;
    struct sockaddr_in serv_addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) return;

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(processos[destino_id].port);
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) >= 0) {
        send(sockfd, mensagem, strlen(mensagem), 0);
    }
    close(sockfd);
}

void anunciar_lider(int id_lider) {
    char mensagem[BUFFER_SIZE];
    snprintf(mensagem, sizeof(mensagem), "LIDER:%d", id_lider);
    for (int i = 0; i < NUM_PROC; i++) {
        if (processos[i].ativo && processos[i].id != meu_id) {
            enviar_mensagem(i, mensagem);
        }
    }
}

void *eleicao(void *arg) {
    int id_chamador = (int)(long)arg;
    printf("🔔 Processo %d iniciou eleição.\n", id_chamador);

    bool alguem_respondeu = false;
    for (int i = id_chamador; i < NUM_PROC; i++) {
        if (processos[i].ativo && processos[i].id > id_chamador) {
            enviar_mensagem(i, "ELEICAO");
            alguem_respondeu = true;
        }
    }

    if (!alguem_respondeu) {
        processos[id_chamador - 1].is_lider = true;
        lider_atual = id_chamador;
        printf("👑 Processo %d se tornou líder!\n", id_chamador);
        anunciar_lider(id_chamador); // <-- NOVO: envia anúncio do líder
    }

    return NULL;
}

void *escutar_mensagens(void *arg) {
    Processo *proc = (Processo *)arg;
    int sockfd, newsockfd;
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t clilen;
    char buffer[BUFFER_SIZE];

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(proc->port);

    bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    listen(sockfd, 5);
    clilen = sizeof(cli_addr);

    while (1) {
        newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
        memset(buffer, 0, BUFFER_SIZE);
        read(newsockfd, buffer, BUFFER_SIZE);

        printf("📥 Processo %d recebeu: %s\n", proc->id, buffer);

        if (strncmp(buffer, "ELEICAO", 7) == 0) {
            if (proc->ativo && proc->id > meu_id) {
                printf("📩 Processo %d responde à eleição.\n", proc->id);
                enviar_mensagem(meu_id - 1, "OK");
                pthread_t t;
                pthread_create(&t, NULL, eleicao, (void *)(long)proc->id);
            }
        } else if (strncmp(buffer, "LIDER:", 6) == 0) {
            int novo_lider = atoi(buffer + 6);
            lider_atual = novo_lider;
            printf("🧠 Processo %d reconhece que o novo líder é o Processo %d\n", meu_id, novo_lider);
        }

        close(newsockfd);
    }

    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Uso: %s <id do processo>\n", argv[0]);
        return 1;
    }

    meu_id = atoi(argv[1]);

    for (int i = 0; i < NUM_PROC; i++) {
        processos[i].id = i + 1;
        processos[i].port = BASE_PORT + i;
        processos[i].ativo = true;
        processos[i].is_lider = false;
    }

    pthread_t escuta;
    pthread_create(&escuta, NULL, escutar_mensagens, &processos[meu_id - 1]);

    sleep(2);
    pthread_t inicio;
    pthread_create(&inicio, NULL, eleicao, (void *)(long)meu_id);

    while (1) sleep(1);
    return 0;
}
