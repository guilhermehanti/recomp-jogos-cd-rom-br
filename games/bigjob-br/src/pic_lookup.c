#include <stdint.h>
#include <stdio.h>

static uint32_t pic_size(const uint32_t *offsets, uint16_t index)
{
    /* NE 34:147B?1499. Table is already in host order; DX:AX = end - start. */
    return offsets[index + 1u] - offsets[index];
}

static int pic_lookup(const uint32_t *offsets, uint16_t count, int index,
                      uint32_t *start, uint32_t *length)
{
    /* Bounds check supplied by the host harness, outside the lifted block. */
    if (index < 0 || index >= count) return 0;
    *start = offsets[index];
    *length = pic_size(offsets, (uint16_t)index);
    return 1;
}

int main(void)
{
    uint32_t offsets[16384], start, length;
    unsigned count;
    while (scanf("%u", &count) == 1 && count && count < 16384) {
        for (unsigned i=0; i<=count; ++i)
            if (scanf("%u", &offsets[i]) != 1) return 1;
        for (int i=-1; i<=(int)count; ++i) {
            if (pic_lookup(offsets, (uint16_t)count, i, &start, &length))
                printf("%u %u\n", start, length);
            else
                puts("invalid");
        }
    }
    return feof(stdin) ? 0 : 1;
}
