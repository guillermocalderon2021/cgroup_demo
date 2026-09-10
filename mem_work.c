#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MIB (1024UL * 1024UL)
#define CHUNK_MIB 4UL

struct block {
    void *ptr;
    size_t size;
};

static unsigned long parse_ulong(const char *s, const char *name) {
    char *end = NULL;
    errno = 0;
    unsigned long value = strtoul(s, &end, 10);
    if (errno != 0 || end == s || *end != '\0' || value == 0) {
        fprintf(stderr, "%s debe ser un entero positivo\n", name);
        exit(EXIT_FAILURE);
    }
    return value;
}

static void sleep_ms(unsigned long ms) {
    struct timespec ts = {
        .tv_sec = (time_t)(ms / 1000UL),
        .tv_nsec = (long)((ms % 1000UL) * 1000000UL)
    };

    while (nanosleep(&ts, &ts) == -1 && errno == EINTR) {
        /* continuar durmiendo el tiempo restante */
    }
}

int main(int argc, char **argv) {
    if (argc < 2 || argc > 3) {
        fprintf(stderr, "Uso: %s <objetivo_MiB> [pausa_ms]\n", argv[0]);
        return EXIT_FAILURE;
    }

    const unsigned long target_mib = parse_ulong(argv[1], "objetivo_MiB");
    const unsigned long pause_ms = (argc == 3) ? parse_ulong(argv[2], "pausa_ms") : 200UL;
    const size_t nblocks = (target_mib + CHUNK_MIB - 1UL) / CHUNK_MIB;

    struct block *blocks = calloc(nblocks, sizeof(*blocks));
    if (blocks == NULL) {
        perror("calloc");
        return EXIT_FAILURE;
    }

    setvbuf(stdout, NULL, _IOLBF, 0);

    size_t allocated_mib = 0;
    for (size_t i = 0; i < nblocks; ++i) {
        const size_t remaining_mib = target_mib - allocated_mib;
        const size_t this_mib = remaining_mib < CHUNK_MIB ? remaining_mib : CHUNK_MIB;
        const size_t this_bytes = this_mib * MIB;

        void *p = malloc(this_bytes);
        if (p == NULL) {
            perror("malloc");
            fprintf(stderr, "allocation_failed_after=%zu MiB\n", allocated_mib);
            return EXIT_FAILURE;
        }

        /* Escribir el bloque fuerza el uso efectivo de sus páginas de memoria. */
        memset(p, 0xA5, this_bytes);
        blocks[i].ptr = p;
        blocks[i].size = this_bytes;
        allocated_mib += this_mib;

        printf("allocated=%zu MiB\n", allocated_mib);
        sleep_ms(pause_ms);
    }

    printf("completed target=%lu MiB allocated=%zu MiB\n", target_mib, allocated_mib);

    for (size_t i = 0; i < nblocks; ++i) {
        free(blocks[i].ptr);
    }
    free(blocks);
    return EXIT_SUCCESS;
}
