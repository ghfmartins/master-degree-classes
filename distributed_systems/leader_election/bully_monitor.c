#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <time.h>

#define MAX_PROCESSES 5
#define BASE_PORT 5000
#define BUF_SIZE 128

int process_id;
int leader_id = -1;
bool election_in_progress = false;
bool waiting_for_ok = false;
bool received_ok = false;

// ==== Prototipação ====
void send_message(int target_id, const char* msg);
void broadcast(const char* msg);
void* listener_thread(void* arg);
void initiate_election();

// ==== Envio de mensagens via socket ====
void send_message(int target_id, const char* msg) {
    int sock;
    struct sockaddr_in addr;
    char buffer[BUF_SIZE];

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return;

    addr.sin_family = AF_INET;
    addr.sin_port = htons(BASE_PORT + target_id);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        close(sock);
        return;
    }

    snprintf(buffer, sizeof(buffer), "%d:%s", process_id, msg);
    send(sock, buffer, strlen(buffer), 0);
    close(sock);
}

void broadcast(const char* msg) {
    for (int i = 1; i <= MAX_PROCESSES; i++) {
        if (i != process_id) {
            send_message(i, msg);
        }
    }
}

// ==== Thread que escuta mensagens ====
void* listener_thread(void* arg) {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    char buffer[BUF_SIZE];
    socklen_t addr_len = sizeof(client_addr);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(BASE_PORT + process_id);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    listen(server_fd, 10);

    while (1) {
        client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &addr_len);
        memset(buffer, 0, BUF_SIZE);
        recv(client_fd, buffer, BUF_SIZE, 0);

        int sender_id;
        char msg[BUF_SIZE];
        sscanf(buffer, "%d:%s", &sender_id, msg);

        if (strcmp(msg, "ELECTION") == 0) {
            printf("📨 Recebi ELECTION de %d\n", sender_id);

            // Se eu sou o líder, ignoro a eleição
            if (leader_id == process_id) {
                printf("👑 Eu sou o líder atual. Ignorando ELECTION de %d.\n", sender_id);
            } else {
                send_message(sender_id, "OK");

                if (!election_in_progress) {
                    sleep(1);
                    initiate_election(); // responder também inicia nova eleição
                }
            }

        } else if (strcmp(msg, "OK") == 0) {
            printf("✅ Recebi OK de %d\n", sender_id);
            received_ok = true;

        } else if (strcmp(msg, "LEADER") == 0) {
            printf("👑 Novo líder anunciado: %d\n", sender_id);
            leader_id = sender_id;
            election_in_progress = false;
            waiting_for_ok = false;
            received_ok = false;
        }

        close(client_fd);
    }

    return NULL;
}

// ==== Função que inicia a eleição ====
void initiate_election() {
    if (election_in_progress) return;

    election_in_progress = true;
    waiting_for_ok = true;
    received_ok = false;

    printf("🔔 Processo %d iniciou eleição\n", process_id);

    for (int i = process_id + 1; i <= MAX_PROCESSES; i++) {
        send_message(i, "ELECTION");
    }

    // Espera por OK com timeout
    time_t start = time(NULL);
    while (time(NULL) - start < 3) {
        if (received_ok) break;
        sleep(1);
    }

    waiting_for_ok = false;

    if (!received_ok) {
        printf("👑 Processo %d se declarou LÍDER\n", process_id);
        leader_id = process_id;
        broadcast("LEADER");
        election_in_progress = false;
    } else {
        printf("⌛ Processo %d aguardando o novo líder...\n", process_id);
        // espera por anúncio do líder vindo de outro processo
    }
}

// ==== Função principal ====
int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Uso: %s <ID>\n", argv[0]);
        return 1;
    }

    process_id = atoi(argv[1]);

    pthread_t listener;
    pthread_create(&listener, NULL, listener_thread, NULL);

    sleep(2);  // Espera todos iniciarem

    if (process_id == MAX_PROCESSES) {
        leader_id = process_id;
        printf("👑 Processo %d iniciou como LÍDER\n", process_id);
        broadcast("LEADER");
    }

    // Monitor do líder
    while (1) {
        sleep(5);

        if (leader_id == process_id) continue;

        int sock = socket(AF_INET, SOCK_STREAM, 0);
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(BASE_PORT + leader_id);
        addr.sin_addr.s_addr = inet_addr("127.0.0.1");

        if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
            printf("⚠️ Falha ao contactar líder %d! Iniciando nova eleição...\n", leader_id);
            initiate_election();
        } else {
            close(sock);
        }
    }

    return 0;
}
