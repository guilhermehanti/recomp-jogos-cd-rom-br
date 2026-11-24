#include <stdint.h>
#include <stdio.h>

/* NE 34:140D?14FC. LES accesses the four arrays via the resource object.
   Host arguments are the selected entries, already read and endian-decoded. */
static uint32_t pic_size(uint16_t mode, uint16_t flags,
                         uint32_t begin, uint32_t end,
                         uint32_t explicit_size, uint16_t width, uint16_t height)
{
    unsigned shift = (2u * mode + 2u) & 31u;
    unsigned current = (flags >> shift) & 3u;
    unsigned baseline = (flags >> (mode == 2 ? 4 : 2)) & 3u;
    if (current == baseline) return end - begin;
    if (baseline == 2) return explicit_size;
    uint16_t stride = (uint16_t)((width + 3u) & 0xfffcu);
    return (uint32_t)((int32_t)(int16_t)stride * (int32_t)(int16_t)height);
}

int main(void)
{
    unsigned mode, flags, begin, end, stored, width, height;
    int fields;
    while ((fields = scanf("%u %u %u %u %u %u %u", &mode, &flags, &begin,
                           &end, &stored, &width, &height)) == 7)
        printf("%u\n", pic_size((uint16_t)mode, (uint16_t)flags, begin, end,
                                stored, (uint16_t)width, (uint16_t)height));
    return fields == EOF ? 0 : 1;
}
