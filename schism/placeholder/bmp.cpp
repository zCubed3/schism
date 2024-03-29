#include "bmp.hpp"

#include <cstdio>

void scWriteBMP(const char* path, uint16_t width, uint16_t height, char* rgb) {
    FILE* file = fopen(path, "wb");

    if (file == NULL)
        return;

    scBMPHeader_t header;
    scBMPInfo_t info;

    header.ident = 'B' | 'M' << 8;
    header.offset = sizeof(header) + sizeof(info);
    header.size = width * height * 3;

    info.size = sizeof(info);
    info.width = width;
    info.height = height;
    info.bits = 24;
    info.compression = 0;
    info.image_size = 0;
    info.x_per_m = 100;
    info.y_per_m = 100;
    info.color_usage = 0;
    info.importance = 0;

    fwrite(&header, sizeof(header), 1, file);
    fwrite(&info, sizeof(info), 1, file);

    fwrite(rgb, width * height * 3, 1, file);

    fclose(file);
}