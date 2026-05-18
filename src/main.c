/* src/main.c */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include "common.h"
#include "network.h"

// Array global para gestionar nuestros 4 clientes
z_client_t *clientes[MAX_CLIENTS] = {NULL};

int main() {
    printf("--- [zaramagad] Demonio de ZaramagaOS iniciado ---\n");

    int srv_metrics = create_unix_socket(SOCKET_METRICS);
    int srv_control = create_unix_socket(SOCKET_CONTROL);

    if (srv_metrics == -1 || srv_control == -1) exit(EXIT_FAILURE);

    printf("[Main] Esperando clientes (Máximo %d)...\n", MAX_CLIENTS);

    int client_id_counter = 0;

    while (client_id_counter < MAX_CLIENTS) {
        // 1. Aceptar conexión de Control
        int fd_c = accept(srv_control, NULL, NULL);
        if (fd_c == -1) continue;

        // 2. Aceptar conexión de Métricas
        // (El cliente debe conectar a ambos para ser validado)
        int fd_m = accept(srv_metrics, NULL, NULL);
        if (fd_m == -1) {
            close(fd_c);
            continue;
        }

        printf("[Main] Cliente %d conectado (Control FD: %d, Metrics FD: %d)\n", 
               client_id_counter, fd_c, fd_m);

        // 3. Reservar memoria para el "objeto" cliente
        z_client_t *nuevo_cliente = malloc(sizeof(z_client_t));
        memset(nuevo_cliente, 0, sizeof(z_client_t));

        // 4. Inicializar datos
        nuevo_cliente->id = client_id_counter;
        nuevo_cliente->fd_control = fd_c;
        nuevo_cliente->fd_metrics = fd_m;
        nuevo_cliente->is_active = true;
        nuevo_cliente->metrics_requested = false; // Por defecto apagado
        
        pthread_mutex_init(&nuevo_cliente->mtx_cmd, NULL);
        pthread_mutex_init(&nuevo_cliente->mtx_send, NULL);
        pthread_cond_init(&nuevo_cliente->cond_cmd, NULL);

        // 5. Lanzar el gestor (esto disparará los 3 hilos en handler.c)
        pthread_t thread_master;
        pthread_create(&thread_master, NULL, start_client_handler, (void*)nuevo_cliente);
        pthread_detach(thread_master); // Nos olvidamos de él, se gestiona solo
        pthread_mutex_lock(&clientes_mtx);
        clientes_activos++;
        pthread_mutex_unlock(&clientes_mtx);
        // clientes[client_id_counter] = nuevo_cliente;
        // client_id_counter++;
        printf("[Main] Clientes conectados %d \n", clientes_activos);

    }

    printf("[Main] Límite de clientes alcanzado. Servidor lleno.\n");
    
    // Bucle infinito para que el main no muera
    while(1) sleep(10); 

    return 0;
}