//
// Created by alb on 18/5/26.
//
// common.c
#include "../include/common.h"

int clientes_activos = 0;
pthread_mutex_t clientes_mtx = PTHREAD_MUTEX_INITIALIZER;