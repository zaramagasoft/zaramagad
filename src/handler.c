/* src/handler.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "common.h"
#include "metrics.h"
#include "commands.h"

// --- HILO 1: TELEMETRÍA (Envío cada 2s) ---
void* thread_telemetry(void* arg) {
    z_client_t *c = (z_client_t*)arg;
    char buffer[128];

    while(c->is_active) {
        if(c->metrics_requested) {
            float cpu = get_cpu_usage();
            float ram = get_ram_usage();
            snprintf(buffer, sizeof(buffer), "{ \"cpu\": %.2f, \"ram\": %.2f }\n", cpu, ram);
            
            // Enviamos por el socket de métricas
            if(write(c->fd_metrics, buffer, strlen(buffer)) <= 0) {
                c->is_active = false; // Cliente desconectado
            }
        }
        sleep(2); 
    }
    return NULL;
}

// --- HILO 3: EJECUTOR (Responde peticiones) ---
void* thread_executor(void* arg) {
    z_client_t *c = (z_client_t*)arg;
    char response[512];

    while(c->is_active) {
        pthread_mutex_lock(&c->mtx_cmd);
        // Espera a que el hilo RX le despierte
        pthread_cond_wait(&c->cond_cmd, &c->mtx_cmd);
        
        if (!c->is_active) {
            pthread_mutex_unlock(&c->mtx_cmd);
            break;
        }

        // Ejecutar el comando guardado en pending_cmd
        execute_system_command(c->pending_cmd, response);
        pthread_mutex_unlock(&c->mtx_cmd);

        // Enviar respuesta por el socket de CONTROL (con Mutex de envío)
        pthread_mutex_lock(&c->mtx_send);
        write(c->fd_control, response, strlen(response));
        write(c->fd_control, "\n", 1);
        pthread_mutex_unlock(&c->mtx_send);
    }
    return NULL;
}

// --- HILO 2: RECEPTOR (Escucha al cliente) ---
void* thread_receiver(void* arg) {
    z_client_t *c = (z_client_t*)arg;
    char buffer[256];

    while(c->is_active) {
        int bytes = read(c->fd_control, buffer, sizeof(buffer) - 1);
        if(bytes <= 0) {
            c->is_active = false;
            break;
        }
        buffer[bytes] = '\0';
        // Limpiar el salto de línea si existe
        strtok(buffer, "\n\r");

        if(strcmp(buffer, "START_METRICS") == 0) {
            c->metrics_requested = true;
            pthread_mutex_lock(&c->mtx_send);
            write(c->fd_control, "METRICS_ENABLED\n", 16);
            pthread_mutex_unlock(&c->mtx_send);
        } 
        else if(strcmp(buffer, "STOP_METRICS") == 0) {
            c->metrics_requested = false;
        } 
        else {
            // Es un comando para el sistema (ej: PING)
            pthread_mutex_lock(&c->mtx_cmd);
            strncpy(c->pending_cmd, buffer, sizeof(c->pending_cmd));
            pthread_cond_signal(&c->cond_cmd); // Despertar al ejecutor
            pthread_mutex_unlock(&c->mtx_cmd);
        }
    }
    // Despertar al ejecutor para que muera si el cliente se desconecta
    pthread_cond_signal(&c->cond_cmd);
    return NULL;
}

// --- FUNCIÓN MAESTRA (Lanzada por main.c) ---
void* start_client_handler(void* arg) {
    z_client_t *c = (z_client_t*)arg;

    pthread_create(&c->thread_tele, NULL, thread_telemetry, (void*)c);
    pthread_create(&c->thread_rx,   NULL, thread_receiver,  (void*)c);
    pthread_create(&c->thread_exec, NULL, thread_executor,  (void*)c);

    // Esperar a que los hilos terminen (cuando el cliente se va)
    pthread_join(c->thread_rx, NULL);
    pthread_join(c->thread_tele, NULL);
    pthread_join(c->thread_exec, NULL);

    printf("[Handler] Cliente %d desconectado. Limpiando...\n", c->id);
    close(c->fd_control);
    close(c->fd_metrics);
    free(c);
    return NULL;
}