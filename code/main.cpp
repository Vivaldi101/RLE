#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>

#define MAX_SYMBOL_COUNT 255

#define EXTRA_FILE_BYTES size_t
#define NUM_EXTRA_FILE_BYTES sizeof(EXTRA_FILE_BYTES)
#define internal static
#define persistent static
#define global static


// typedefs for all ints
typedef int8_t		s8;
typedef int16_t		s16;
typedef int32_t		s32;
typedef int64_t		s64;

typedef uint8_t		u8;
typedef uint16_t	u16;
typedef uint32_t	u32;
typedef uint64_t	u64;

typedef unsigned char byte;

// 32 bit bool for dword alignment
typedef u32 b32;

// typedefs for single and double floating points
typedef float	r32;
typedef double	r64;

typedef struct 
{
    u32 size;
    byte *data;
} file_t;

    internal file_t
get_file(char *file_name)
{
    file_t result = {0};
    FILE *fp = fopen(file_name, "rb");

    if(fp) 
    {
        fseek(fp, 0, SEEK_END);
        s64 size = ftell(fp);
        assert((size > 0u) && (size < (1ll << 32ll)));
        result.size = (u32)size;
        fseek(fp, 0, SEEK_SET);
        result.data = (byte *)malloc(result.size);
        fread(result.data, sizeof(byte), result.size, fp);
        fclose(fp);
    } 
    else 
    {
        fprintf(stderr, "Unable to read file %s\n", file_name);
    }

    return result;
}

    internal u32
get_max_compressed_file_size(u32 size)
{
    u32 result = 256 + size * 2;

    return result;
}

    internal void 
copy(byte *src, byte *dst)
{
    while(*dst++ = *src++) ;
}

    internal size_t
encode_literals(byte **out_data, byte *literal_buffer, u32 literal_count)
{
    size_t result = 0;
    assert(literal_count <= MAX_SYMBOL_COUNT);

    *((*out_data)++) = 0x00;
    *((*out_data)++) = (u8)literal_count;
    result++;
    result++;
    for(u32 i = 0; i < literal_count; i++)

    {
        *((*out_data)++) = literal_buffer[i];
        result++;
        literal_buffer[i] = 0;
    }

    return result;
}

    internal size_t
encode_run(byte **out_data, byte* literal_buffer, u32 run_length, byte run_symbol)
{
    size_t result = 0;
    assert(run_length <= MAX_SYMBOL_COUNT);
    *((*out_data)++) = 0xFF;
    *((*out_data)++) = (u8)run_length;
    *((*out_data)++) = run_symbol;
    result += 3;

    return result;
}

    internal size_t
rle_compress(size_t in_size, byte *in_data, byte *out_data)
{
    size_t result = 0;
    byte *const begin_in_data = in_data;
    byte *const end_in_data = begin_in_data + in_size;
    byte literal_buffer[MAX_SYMBOL_COUNT] = {0};
    u32 literal_count = 0;
    while(in_data < end_in_data)
    {
        u32 end_of_run = 0;
        byte first_symbol = *in_data;
        while((first_symbol == *(in_data + end_of_run)) && (end_of_run < MAX_SYMBOL_COUNT))
        {
            end_of_run++;
        }
        if(end_of_run > 1)
        {
            if(literal_count)
            {
                result += encode_literals(&out_data, literal_buffer, literal_count);
                literal_count = 0;
            }

            result += encode_run(&out_data, literal_buffer, end_of_run, first_symbol);
            in_data += end_of_run;
        }
        else
        {
            if(literal_count < MAX_SYMBOL_COUNT)
            {
                literal_buffer[literal_count++] = *in_data++;
            }
            else
            {
                result += encode_literals(&out_data, literal_buffer, literal_count);
                literal_count = 0;
            }
        }
    }

    result += encode_literals(&out_data, literal_buffer, literal_count);

    return result;
}

    internal void
rle_decompress(size_t in_size, byte *in_data, byte *out_data)
{
    byte *const begin_in_data = in_data;
    while(in_data < (begin_in_data + in_size))
    {
        u8 run_code = *in_data++;
        u8 run_length = *in_data++;
        // Decode runs
        if(run_code == 0xFF)
        {
            byte run_symbol = *in_data++;
            for(s32 i = 0; i < run_length; i++)
            {
                *out_data++ = run_symbol;
            }
        }
        else
        {
            for(s32 i = 0; i < run_length; i++)
            {
                *out_data++ = *in_data++;
            }
        }
    }
}

    internal void
open_file_and_write_data(char *file_name, size_t write_size, byte *data)
{
    FILE *fp = fopen(file_name, "wb");
    if(fp)
    {
        fwrite(data, sizeof(byte), write_size, fp);
        fclose(fp);
    }
    else
    {
        fprintf(stderr, "Unable to write to file: %s\n", file_name);
    }
}

    internal void
replace_filename_suffix(char *filename, char *new_suffix)
{
    char *p = filename;
    for(; *p != '.'; p++) ;
    while(*p++ = *new_suffix++) ;
}

    internal char *
get_filename_suffix(char *filename)
{
    char *result = NULL;
    char *p = filename;
    for(; *p != '.'; p++) ;
    result = p;

    return result;
}

    internal u32
get_num_string_length_bytes(char *string)
{
    u32 result = 0;
    while(*string++)
    {
        result++;
    }

    return result;
}

    int 
main(int argc, char **argv)
{
    char *program = argv[0];
    if(argc == 3) 
    {
        char *cmd = argv[1];
        char *filename = argv[2];

        size_t result_out_size = 0;
        byte *result_out_buffer = NULL;

        file_t file = get_file(filename);
        if((strcmp(cmd, "compress") == 0) || (strcmp(cmd, "co") == 0)) 
        {
            u32 total_allocation_size = 0;
            char compressed_filename[256] = {0};
            copy((byte *)filename, (byte *)compressed_filename);
            char *filename_suffix = get_filename_suffix(compressed_filename);
            u32 max_out_size = get_max_compressed_file_size(file.size);
            u32 filename_suffix_size = get_num_string_length_bytes(filename_suffix);
            assert(filename_suffix_size > 0);
            total_allocation_size = max_out_size + NUM_EXTRA_FILE_BYTES + filename_suffix_size + sizeof(u32);

            byte *out_buffer = (byte *)malloc(total_allocation_size);

            memset(out_buffer, 0, total_allocation_size);

            *(EXTRA_FILE_BYTES *)out_buffer = file.size;
            out_buffer += NUM_EXTRA_FILE_BYTES;
            *(u32 *)out_buffer = filename_suffix_size;
            out_buffer += sizeof(u32);
            for(u32 i = 0; i < filename_suffix_size; i++)
            {
                *out_buffer++ = (byte)(*filename_suffix++);
            }
            size_t compressed_size = rle_compress(file.size, file.data, out_buffer);
            result_out_size = compressed_size + NUM_EXTRA_FILE_BYTES + filename_suffix_size + sizeof(u32);

            result_out_buffer = out_buffer - (NUM_EXTRA_FILE_BYTES + sizeof(u32) + filename_suffix_size);

            replace_filename_suffix(compressed_filename, ".rle");
            open_file_and_write_data(compressed_filename, result_out_size, result_out_buffer);
            free(result_out_buffer);

            fprintf(stdout, "File %s compressed to: %.1f%% of the original size\n", filename, ((r32)(compressed_size + NUM_EXTRA_FILE_BYTES) / (r32)file.size) * 100.0f);
        } 
        else if((strcmp(cmd, "decompress") == 0) || (strcmp(cmd, "dc") == 0)) 
        {
            char decompressed_filename[256] = {0};
            if((file.size >= NUM_EXTRA_FILE_BYTES) && (strcmp((get_filename_suffix(filename) + 1), "rle")) == 0) 
            {
                size_t decompressed_file_size = *(EXTRA_FILE_BYTES *)file.data;
                u32 filename_suffix_size = *(u32 *)(file.data + NUM_EXTRA_FILE_BYTES);
                char *decompressed_filename_suffix = (char *)(file.data + NUM_EXTRA_FILE_BYTES + filename_suffix_size);
                byte *out_buffer = (byte *)malloc(decompressed_file_size);
                memset(out_buffer, 0, decompressed_file_size);

                rle_decompress(file.size - (NUM_EXTRA_FILE_BYTES + sizeof(u32) + filename_suffix_size), file.data + NUM_EXTRA_FILE_BYTES + sizeof(u32) + filename_suffix_size, out_buffer);

                result_out_size = decompressed_file_size;
                result_out_buffer = out_buffer;

                copy((byte *)filename, (byte *)decompressed_filename);
                replace_filename_suffix(decompressed_filename, decompressed_filename_suffix);
                open_file_and_write_data(decompressed_filename, result_out_size, result_out_buffer);
                free(result_out_buffer);
            } 
            else 
            {
                fprintf(stderr, "Invalid file: %s\nFile must be a valid rle compressed file with .rle suffix", filename);
            }
        } 
        else 
        {
            fprintf(stderr, "Unknown command: %s\n", cmd);
            fprintf(stderr, "Known commands: compress|decompress\n");
        }
    } 
    else 
    {
        fprintf(stderr, "Invalid usage: %s %s\n", program, argv[1]);
        fprintf(stderr, "Valid usage: %s compress|decompress <filename>", program);
    }

    return 0;
}
