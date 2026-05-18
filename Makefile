# Configuración
CC = gcc
CFLAGS = -Wall -Wextra -Iinclude -pthread
BINDIR = bin

# Archivos del SERVIDOR (zaramagad)
# NOTA: No incluimos cliente.c aquí
SRCS_SERV = src/main.c src/network.c src/handler.c src/metrics.c src/commands.c src/common.c
OBJS_SERV = $(SRCS_SERV:.c=.o)
TARGET_SERV = $(BINDIR)/zaramagad

# Archivos del CLIENTE (test)
SRC_CLI = src/cliente.c
TARGET_CLI = $(BINDIR)/z_client_test

# Regla principal
all: $(BINDIR) $(TARGET_SERV) $(TARGET_CLI)

$(BINDIR):
	mkdir -p $(BINDIR)

# Compilar el Servidor
$(TARGET_SERV): $(OBJS_SERV)
	$(CC) $(CFLAGS) -o $@ $^
	@echo "--- [zaramagad] Servidor compilado con éxito ---"

# Compilar el Cliente (por separado)
$(TARGET_CLI): $(SRC_CLI)
	$(CC) $(CFLAGS) $< -o $@
	@echo "--- [z_client_test] Cliente de prueba compilado ---"

# Regla genérica para objetos
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f src/*.o $(TARGET_SERV) $(TARGET_CLI)
	rm -f /tmp/z_metrics.sock /tmp/z_control.sock
	@echo "--- Limpieza completada ---"