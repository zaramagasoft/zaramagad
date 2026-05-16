/* src/network.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "../include/network.h"

int create_unix_socket(const char *path) {
    int fd;
    struct sockaddr_un addr;

    // 1. Crear el socket
    if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("Error al crear socket");
        return -1;
    }

    // 2. Limpiar el archivo anterior si existe (evita el error 'Address already in use')
    unlink(path);

    // 3. Configurar la dirección
    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, path, sizeof(addr.sun_path) - 1);

    // 4. Vincular el socket a la ruta
    if (bind(fd, (struct sockaddr *)&addr, sizeof(struct sockaddr_un)) == -1) {
        perror("Error en bind");
        close(fd);
        return -1;
    }

    // 5. Poner en escucha
    if (listen(fd, 5) == -1) {
        perror("Error en listen");
        close(fd);
        return -1;
    }

    printf("[zaramagad] Escuchando en %s\n", path);
    return fd;
}