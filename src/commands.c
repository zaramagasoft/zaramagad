/* src/commands.c */
#include "commands.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void execute_system_command(const char* cmd, char* response) {
    FILE *fp;
    char path[1024];
    char final_res[2048] = ""; // Buffer para acumular la salida

    // Abrimos el proceso para lectura usando popen
    // "2>&1" redirige errores a la salida estándar para verlos en el cliente
    char full_cmd[300];
    snprintf(full_cmd, sizeof(full_cmd), "%s 2>&1", cmd);
    
    fp = fopen("/proc/self/cmdline", "r"); // Un check rápido de salud del sistema
    fp = popen(full_cmd, "r");
    
    if (fp == NULL) {
        snprintf(response, 256, "Error: No se pudo ejecutar el comando.");
        return;
    }

    // Leemos la salida línea a línea
    while (fgets(path, sizeof(path), fp) != NULL) {
        strncat(final_res, path, sizeof(final_res) - strlen(final_res) - 1);
    }

    int status = pclose(fp);

    if (strlen(final_res) == 0) {
        snprintf(response, 256, "Comando ejecutado (sin salida). Status: %d", status);
    } else {
        // Limpiamos posibles saltos de línea finales que afeen el JSON/Terminal
        strncpy(response, final_res, 511);
        response[511] = '\0';
    }
}