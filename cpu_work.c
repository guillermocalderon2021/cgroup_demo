#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

static uint64_t parse_u64(const char *s) {
    char *end = NULL;
    errno = 0;
    unsigned long long value = strtoull(s, &end, 10);
    if (errno != 0 || end == s || *end != '\0' || value == 0) {
        fprintf(stderr, "Uso: cpu_work <iteraciones>\n");
        exit(EXIT_FAILURE);
    }
    return (uint64_t)value;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <iteraciones>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const uint64_t iterations = parse_u64(argv[1]);
    uint64_t x = UINT64_C(0x9e3779b97f4a7c15);

    for (uint64_t i = 0; i < iterations; ++i) {
        /* Dependencia entre iteraciones: el trabajo no puede reducirse a una suma simple. */
        x ^= x << 13;
        x ^= x >> 7;
        x ^= x << 17;
        x += i * UINT64_C(0x9e3779b97f4a7c15);
    }

    printf("iterations=%" PRIu64 " checksum=%" PRIu64 "\n", iterations, x);
    return EXIT_SUCCESS;
}
