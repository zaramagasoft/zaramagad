/* include/common.h */
#ifndef COMMON_H
#define COMMON_H

#include <pthread.h>
#include <stdbool.h>

// Configuración del sistema ZaramagaOS
#define MAX_CLIENTS 4
#define SOCKET_METRICS "/tmp/z_metrics.sock"
#define SOCKET_CONTROL "/tmp/z_control.sock"

extern int clientes_activos;
extern pthread_mutex_t clientes_mtx;

// Estructura maestra que representa a un cliente en zaramagad
typedef struct {
    int id;                  // ID único del cliente (0-3)
    int fd_metrics;          // Socket exclusivo de telemetría (Server -> Cliente)
    int fd_control;          // Socket exclusivo de comandos (Bidireccional)
    
    pthread_t thread_tele;   // Hilo 1: Envío de métricas cada 2s
    pthread_t thread_rx;     // Hilo 2: Escucha peticiones del cliente
    pthread_t thread_exec;   // Hilo 3: Ejecuta comandos y responde
    
    bool metrics_requested;  // Flag: ¿El cliente activó las métricas?
    bool is_active;          // Control de vida del cliente
    
    char pending_cmd[256];   // Buffer para pasar el comando del RX al Exec
    pthread_mutex_t mtx_cmd; // Protege el acceso al buffer de comando
    pthread_cond_t cond_cmd; // Señal para avisar al Ejecutor que hay trabajo
    
    pthread_mutex_t mtx_send; // Protege el socket de control (evita colisión RX/Exec)
} z_client_t;

// Prototipos globales para que el compilador esté feliz
void* start_client_handler(void* arg); // Definido en handler.c

#endif /* COMMON_H */