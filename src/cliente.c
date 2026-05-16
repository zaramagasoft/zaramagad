#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <pthread.h>

#define SOCKET_METRICS "/tmp/z_metrics.sock"
#define SOCKET_CONTROL "/tmp/z_control.sock"

// Hilo para escuchar métricas en segundo plano
void* listen_metrics(void* arg) {
    int fd = *(int*)arg;
    char buffer[256];
    printf("[Cliente] Escuchando métricas...\n");
    while (1) {
        int bytes = read(fd, buffer, sizeof(buffer)-1);
        if (bytes > 0) {
            buffer[bytes] = '\0';
            printf("\r\033[K[MÉTRICAS] %s", buffer); // Sobreescribe la línea para que no llene la pantalla
            fflush(stdout);
        } else break;
    }
    return NULL;
}

int connect_socket(const char* path) {
    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, path, sizeof(addr.sun_path)-1);
    if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("Error de conexión");
        return -1;
    }
    return fd;
}

int main() {
    printf("--- Test de Cliente ZaramagaOS ---\n");

    // 1. Conectar a ambos canales
    int fd_ctrl = connect_socket(SOCKET_CONTROL);
    int fd_metr = connect_socket(SOCKET_METRICS);

    if (fd_ctrl == -1 || fd_metr == -1) exit(1);

    // 2. Lanzar hilo para ver las métricas sin bloquear el menú
    pthread_t th;
    pthread_create(&th, NULL, listen_metrics, &fd_metr);

    // 3. Simular interacción del usuario
    char cmd[256];
    printf("[Cliente] Escribe comandos (ej: START_METRICS, uname -a, uptime, STOP_METRICS)\n");
    
    while (1) {
        printf("\n> ");
        fgets(cmd, sizeof(cmd), stdin);
        write(fd_ctrl, cmd, strlen(cmd));

        // Leer respuesta del comando (Control)
        char res[1024];
        int b = read(fd_ctrl, res, sizeof(res)-1);
        if (b > 0) {
            res[b] = '\0';
            printf("[Respuesta Server]: %s\n", res);
        }
    }

    return 0;
}