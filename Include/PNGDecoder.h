#include "../zlib/zlib.h"
#include "LinkedList.h"
#include <stdint.h>

#pragma once
#define PNG_SIGNATURE 0xa1a0a0d474e5089

FILE *stream;
typedef struct chunk
{
    size_t length;
    size_t crc;
    char *type;
    unsigned char *data;
} chunk_t;

typedef struct chunk_type_IHDR
{
    unsigned int width;
    unsigned int height;
    char bit_depth;
    char color_type;
    char compression_method;
    char filter_method;
    char interlace_method;

} chunk_type_IHDR_t;
chunk_t *read_chunk();
chunk_type_IHDR_t *process_IHDR_data(chunk_t *IHDR_chunk);
unsigned char *process_IDAT_chunks(list_item_t *chunk_list, chunk_type_IHDR_t *IHDR_data);
int recon_a(unsigned char *recon, const int r, const int c, int stride, int bytes_per_pixel);
int recon_b(unsigned char *recon, const int r, const int c, int stride, int bytes_per_pixel);
int recon_c(unsigned char *recon, const int r, const int c, int stride, int bytes_per_pixel);
int paeth_predictor(const int a, const int b, const int c);

unsigned char *PNG_decode(char *path, int *width, int *height, int *channels)
{
    stream = malloc(sizeof(FILE));
    if (!stream)
    {
        puts("Failed to allocate memory on stream ptr");
        return NULL;
    }

    int res = fopen_s(&stream, path, "rb");
    size_t png_sign;
    if (res != 0)
    {
        free(stream);
        printf("Couldn't open file");
        return NULL;
    }

    fread(&png_sign, sizeof(size_t), 1, stream);
    if (png_sign != PNG_SIGNATURE)
    {
        printf("Invalid PNG Signature");
        free(stream);
        fclose(stream);
        return NULL;
    }

    list_item_t *chunk_list = NULL;
    while (1)
    {
        chunk_t *new_chunk = read_chunk();
        list_append_ptr(&chunk_list, new_chunk);
        if (!strcmp(new_chunk->type, "IEND"))
        {
            break;
        }
    }

    chunk_type_IHDR_t *IHDR_data = process_IHDR_data(((chunk_t *)chunk_list->node.val));
    unsigned char *pixels = process_IDAT_chunks(chunk_list, IHDR_data);

    *width = IHDR_data->width;
    *height = IHDR_data->height;
    *channels = 4;
    fclose(stream);
    return pixels;
}

uint32_t lil_endian_to_big(uint32_t num)
{
    uint32_t res =
        ((num & 0x000000FF) << 24) |
        ((num & 0x0000FF00) << 8) |
        ((num & 0x00FF0000) >> 8) |
        ((num & 0xFF000000) >> 24);
    return res;
}

chunk_t *read_chunk()
{
    /*
       Memory allocation
    */
    chunk_t *cur_chunk = malloc(sizeof(chunk_t));
    if (!cur_chunk)
    {

        puts("Failed to allocate memory for the chunk");
        return NULL;
    }

    unsigned char *bytes_buffer = calloc(4, 1);
    if (!bytes_buffer)
    {
        free(cur_chunk);
        puts("Failed to allocate memory for the bytes buffer");
        return NULL;
    }
    /*
        Reading the length
    */
    fread(bytes_buffer, 4, 1, stream);
    cur_chunk->length = lil_endian_to_big(*(int *)bytes_buffer);
    /*
        Allocating memory for the type and reading it from the stream
    */
    cur_chunk->type = calloc(4, 1);
    if (!cur_chunk->type)
    {
        free(bytes_buffer);
        free(cur_chunk);
        puts("Failed to allocate memory for the chunk_tpye");
        return NULL;
    }
    fread(bytes_buffer, 4, 1, stream);
    memcpy(cur_chunk->type, bytes_buffer, 4);

    /*
        Allocating memory for the data_buffer and the chunk_data and reading it from the stream
    */
    cur_chunk->data = calloc(cur_chunk->length, 1);
    if (!cur_chunk->data)
    {
        free(bytes_buffer);
        free(cur_chunk->type);
        free(cur_chunk);
        puts("Failed to allocate memory for the chunk_data");
        return NULL;
    }
    unsigned char *data_buffer = malloc(cur_chunk->length);
    if (!data_buffer)
    {
        free(bytes_buffer);
        free(cur_chunk->type);
        free(cur_chunk->data);
        free(cur_chunk);
        puts("Failed to allocate memory for the chunk_data");
        return NULL;
    }

    fread(data_buffer, 1, cur_chunk->length, stream);
    memcpy(cur_chunk->data, data_buffer, cur_chunk->length);
    /*
        calculating CRC32
    */
    fread(bytes_buffer, 4, 1, stream);
    unsigned int expected_crc32 = lil_endian_to_big(*(int *)bytes_buffer);
    unsigned int actual_crc32 = crc32(crc32(0, (Bytef *)cur_chunk->type, 4), (Bytef *)cur_chunk->data, cur_chunk->length);
    if (expected_crc32 != actual_crc32)
    {
        free(bytes_buffer);
        free(cur_chunk->type);
        free(cur_chunk);
        puts("Chunk checksum failed");
        return NULL;
    }
    cur_chunk->crc = actual_crc32;
    free(bytes_buffer);
    return cur_chunk;
}

chunk_type_IHDR_t *process_IHDR_data(chunk_t *IHDR_chunk)
{
    /*
        Allocating memory for chunk_type_IHDR
    */

    chunk_type_IHDR_t *IHDR_data = malloc(sizeof(chunk_type_IHDR_t));
    if (!IHDR_data)
    {
        puts("Failed to allocate memory for IHDR_data");
        return NULL;
    }
    char *byte_buffer = calloc(4, 1);
    if (!byte_buffer)
    {
        free(IHDR_data);
        puts("Failed to allocate memory for byte_buffer");
        return NULL;
    }

    /*
        Reading the IHDR_DATA
    */
    memcpy(byte_buffer, IHDR_chunk->data, 4);
    IHDR_data->width = lil_endian_to_big(*(unsigned int *)byte_buffer);
    memcpy(byte_buffer, &IHDR_chunk->data[4], 4);
    IHDR_data->height = lil_endian_to_big(*(unsigned int *)byte_buffer);
    IHDR_data->bit_depth = IHDR_chunk->data[8];
    IHDR_data->color_type = IHDR_chunk->data[9];
    IHDR_data->compression_method = IHDR_chunk->data[10];
    IHDR_data->filter_method = IHDR_chunk->data[11];
    IHDR_data->interlace_method = IHDR_chunk->data[12];
    if (IHDR_data->bit_depth != 8)
    {
        puts("we only support a bit depth of 8");
        goto free_on_error;
    }
    if (IHDR_data->color_type != 6)
    {

        puts("we only support truecolor with alpha");
        goto free_on_error;
    }
    if (IHDR_data->compression_method != 0)
    {
        puts("Invalid compression method");
        goto free_on_error;
    }
    if (IHDR_data->filter_method != 0)
    {
        puts("Invalid filter method");
        goto free_on_error;
    }
    if (IHDR_data->interlace_method != 0)
    {
        puts("we only support no interlacing");
        goto free_on_error;
    }
    free(byte_buffer);
    return IHDR_data;

free_on_error:
    free(IHDR_data);
    free(byte_buffer);
    return NULL;
}

unsigned char *process_IDAT_chunks(list_item_t *chunk_list, chunk_type_IHDR_t *IHDR_data)
{
    chunk_t *IDAT_data = malloc(sizeof(chunk_t));
    IDAT_data->data = malloc(0);
    if (!IDAT_data)
    {
        puts("failed to allocate memory for IDAT");
    }
    IDAT_data->length = 0;
    list_node_t *cur_node = &chunk_list->node;
    for (size_t i = 0; i < list_len((list_node_t **)chunk_list); i++)
    {
        if (!strcmp(((chunk_t *)cur_node->val)->type, "IDAT"))
        {
            void *new_data = realloc(IDAT_data->data, IDAT_data->length + ((chunk_t *)cur_node->val)->length);
            if(new_data == NULL)
            {
                puts("Failed to realloc");
                free(IDAT_data);
                return NULL;
            }
            IDAT_data->data = new_data;
            memcpy(&IDAT_data->data[IDAT_data->length], ((chunk_t *)cur_node->val)->data, ((chunk_t *)cur_node->val)->length );
            IDAT_data->length += ((chunk_t *)cur_node->val)->length;
        }
        cur_node = cur_node->next;
    }
    size_t bytes_per_pixel = 4;
    size_t stride = IHDR_data->width * bytes_per_pixel;

    unsigned long total_uncompressed_size = IHDR_data->height * (1 + stride);
    unsigned char *uncompress_data = calloc(total_uncompressed_size, 1);
    size_t source_len = compressBound(IDAT_data->length);
    int uncompress_res = uncompress(uncompress_data, &total_uncompressed_size, IDAT_data->data, source_len);

    if (uncompress_res != 0)
    {
        free(IDAT_data->data);
        free(IDAT_data);
        printf("Couldn't uncompress");
    }

    unsigned char *recon = calloc(IHDR_data->height * stride, 1);
    if (!recon)
    {
        free(IDAT_data->data);
        free(IDAT_data);
        puts("Failed to allocate memory for recon");
    }
    int i = 0;
    int filter_type = 0;
    int filt_x = 0;
    int recon_x = 0;
    int pixel_pos = 0;
    for (size_t y = 0; y < IHDR_data->height; y++)
    {
        filter_type = uncompress_data[i];
        i += 1;
        for (size_t x = 0; x < stride; x++)
        {
            filt_x = uncompress_data[i];
            i += 1;
            if (filter_type == 0)
            {
                recon_x = filt_x;
            }
            else if (filter_type == 1)
            {
                recon_x = filt_x + recon_a(recon, y, x, stride, bytes_per_pixel);
            }
            else if (filter_type == 2)
            {
                recon_x = filt_x + recon_b(recon, y, x, stride, bytes_per_pixel);
            }
            else if (filter_type == 3)
            {
                recon_x = filt_x + (recon_a(recon, y, x, stride, bytes_per_pixel) + recon_b(recon, y, x, stride, bytes_per_pixel)) / 2;
            }
            else if (filter_type == 4)
            {
                recon_x = filt_x + paeth_predictor(recon_a(recon, y, x, stride, bytes_per_pixel),
                                                   recon_b(recon, y, x, stride, bytes_per_pixel), recon_c(recon, y, x, stride, bytes_per_pixel));
            }
            recon[pixel_pos] = recon_x;
            pixel_pos += 1;
        }
    }


    return recon;
}

int paeth_predictor(int a, int b, int c)
{
    int p = a + b + c;
    int pa = abs(p - a);
    int pb = abs(p - b);
    int pc = abs(p - c);
    int pr;
    if (pa <= pb && pa <= pc)
    {
        pr = a;
    }
    else if (pb <= pc)
    {
        pr = b;
    }
    else
    {
        pr = c;
    }
    return pr;
}

int recon_a(unsigned char *recon, const int r, const int c, int stride, int bytes_per_pixel)
{
    if (c >= bytes_per_pixel)
        return recon[r * stride + c - bytes_per_pixel];
    return 0;
}
int recon_b(unsigned char *recon, const int r, const int c, int stride, int bytes_per_pixel)
{
    if (r > 0)
        return recon[(r - 1) * stride + c];
    return 0;
}
int recon_c(unsigned char *recon, const int r, const int c, int stride, int bytes_per_pixel)
{
    if (r > 0 && c >= bytes_per_pixel)
        return recon[(r - 1) * stride + c - bytes_per_pixel];
    return 0;
}
