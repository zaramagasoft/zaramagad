/* src/metrics.c */
#include "metrics.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

float get_cpu_usage() {
    long double a[4], b[4], loadavg;
    FILE *fp;

    fp = fopen("/proc/stat", "r");
    fscanf(fp, "%*s %Lf %Lf %Lf %Lf", &a[0], &a[1], &a[2], &a[3]);
    fclose(fp);
    sleep(1); // Necesitamos dos muestras para calcular la diferencia

    fp = fopen("/proc/stat", "r");
    fscanf(fp, "%*s %Lf %Lf %Lf %Lf", &b[0], &b[1], &b[2], &b[3]);
    fclose(fp);

    loadavg = ((b[0]+b[1]+b[2]) - (a[0]+a[1]+a[2])) / ((b[0]+b[1]+b[2]+b[3]) - (a[0]+a[1]+a[2]+a[3]));
    return (float)(loadavg * 100);
}

float get_ram_usage() {
    FILE *fp = fopen("/proc/meminfo", "r");
    long total, free;
    if (!fp) return 0.0f;

    fscanf(fp, "MemTotal: %ld kB\nMemFree: %ld kB", &total, &free);
    fclose(fp);

    return 100.0f * (1.0f - ((float)free / (float)total));
}