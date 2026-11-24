#ifndef CDLAUNCHER_SHA256_H
#define CDLAUNCHER_SHA256_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint32_t state[8];
    uint64_t bitlen;
    uint8_t data[64];
    size_t datalen;
} Sha256Ctx;

void sha256_init(Sha256Ctx *ctx);
void sha256_update(Sha256Ctx *ctx, const uint8_t *data, size_t len);
void sha256_final(Sha256Ctx *ctx, uint8_t hash[32]);
void sha256_to_hex(const uint8_t hash[32], char out_hex[65]);
bool sha256_file_w(const wchar_t *path, char out_hex[65]);

#endif
