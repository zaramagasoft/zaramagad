#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <pthread.h>

#define SOCKET_METRICS "/tmp/z_metrics.sock"
#define SOCKET_CONTROL "/tmp/z_control.sock"

/* // Hilo para escuchar métricas en segundo plano
void *listen_metrics(void *arg)
{
    int fd = *(int *)arg;
    char buffer[256];
    printf("[Cliente] Hilo de métricas iniciado...\n");
    while (1)
    {
        int bytes = read(fd, buffer, sizeof(buffer) - 1);
        if (bytes > 0)
        {
            buffer[bytes] = '\0';
            // Imprime las métricas de forma limpia
            printf("\r\033[K[MÉTRICAS] %s", buffer); 
            fflush(stdout);
        }
        else {
            break; // Si el socket se cierra, salimos del hilo
        }
    }
    return NULL;
}
 */
int connect_socket(const char *path) {
    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, path, sizeof(addr.sun_path) - 1);
    
    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("Error de conexión");
        return -1;
    }
    return fd;
}

// Envía un comando único y espera su respuesta sin bloquear el futuro de la UI
int ejecuta(const char *command)
{
    printf("--- Conectando a zaramagad ---\n");

    // 1. Conectar a ambos canales (Obligatorio para el protocolo del servidor)
    int fd_ctrl = connect_socket(SOCKET_CONTROL);
    usleep(100000);
    int fd_metr = connect_socket(SOCKET_METRICS);
    usleep(100000);
    if (fd_ctrl == -1 || fd_metr == -1) {
        fprintf(stderr, "Error: El servidor zaramagad no está corriendo.\n");
        return -1;
    }

    
    
    // 3. Enviar el comando que hemos recibido por parámetro
    printf("[Cliente] Mandando comando: %s\n", command);
    if (write(fd_ctrl, command, strlen(command)) <= 0) {
        perror("Error al escribir en socket");
        close(fd_ctrl);
       
        return -1;
    }

    // 4. Leer la respuesta única del servidor
    char res[1024];
    int b = read(fd_ctrl, res, sizeof(res) - 1);
    if (b > 0) {
        res[b] = '\0';
        printf("\n[Respuesta Server]:\n%s\n", res);
    } else {
        printf("[Cliente] No se recibió respuesta del comando.\n");
    }

    // Dejamos un margen pequeño para ver si llega el JSON de métricas si fuera "START_METRICS"
    sleep(1); 

    // 5. Limpieza al terminar la acción
    close(fd_ctrl);
    
    return 0;
}

int main()
{
    char command[256];
    
    // Prueba 1: Pedir información del sistema
    strcpy(command, "uname -a\n");
    ejecuta(command);
    
    printf("\n--- Fin de la ejecución de prueba ---\n");
    return 0;
}