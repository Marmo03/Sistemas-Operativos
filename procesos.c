#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>

// --- Función de tarea CPU-bound ---
void cpu_task(int n, const char *name) {
    for (int i = 0; i < n; i++) {
        if (i % (n/5 + 1) == 0) {
            printf("[%s] paso %d (pid=%d)\n", name, i, getpid());
            usleep(50000); // pequeña pausa simulada
        }
    }
    printf("[%s] terminado (pid=%d)\n", name, getpid());
}

int main() {
    int n; 
    printf("Ingrese cantidad de procesos a crear: ");
    scanf("%d", &n);

    // (1) reservar memoria dinámica para un arreglo de PIDs (pid_t) y otro de cargas (int)
    pid_t *pids = malloc(n * sizeof(pid_t));
    int *cargas = malloc(n * sizeof(int));
    if (pids == NULL || cargas == NULL) {
        perror("Error al reservar memoria");
        exit(1);
    }

    // (2) inicializar las cargas en el arreglo "cargas"
    for (int i = 0; i < n; i++) {
        cargas[i] = (i + 1) * 5000000;  // carga creciente
    }

    // --- Crear procesos hijos ---
    for (int i = 0; i < n; i++) {
        // (3) llamar a fork() y guardar el PID en pids[i]
        pid_t pid = fork();
        if (pid < 0) {
            perror("Error al crear proceso");
            exit(1);
        }
        pids[i] = pid;

        if (pid == 0) { // (4) es un hijo
            // usar nice() para variar prioridades
            if (i % 2 == 0) {
                nice(5);  // menor prioridad
            } else {
                nice(0);  // prioridad normal
            }

            // Construir nombre de proceso
            char nombre[32];
            sprintf(nombre, "Proc-%d", i + 1);

            // Ejecutar tarea
            cpu_task(cargas[i], nombre);
            exit(0); // hijo termina aquí
        }
    }

    // --- Esperar a todos los hijos ---
    for (int i = 0; i < n; i++) {
        // (5) usar waitpid() para esperar a pids[i]
        waitpid(pids[i], NULL, 0);
    }

    // (6) liberar la memoria de pids y cargas
    free(pids);
    free(cargas);

    printf("Todos los procesos terminaron.\n");
    return 0;
}
