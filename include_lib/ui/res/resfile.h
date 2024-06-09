#ifndef RESFILE_H
#define RESFILE_H


#include "typedef.h"

#define FILE_TYPE_JPEG 	5

struct image_file {
    u8 format;
    u8 compress;
    u16 data_crc;
    u16 width;
    u16 height;
    u32 offset;
    u32 len;
};

int open_resfile(const char *name);
void close_resfile();

int res_file_version_compare(int res_ver);

int open_str_file(const char *name);
void close_str_file();
int str_file_version_compare(int str_ver);

int open_style_file(const char *name);

int open_image_by_id(struct image_file *f, int id, int page);
int read_image_data(struct image_file *f, u8 *data, int len);
u32 image_decode(const void *pSour, void *pDest, u32 SourLen, u32 DestLen, u8 compress);
int open_string_pic(struct image_file *file, int id);
int read_str_data(struct image_file *f, u8 *data, int len);
int load_pallet_table(int id, u32 *data);
int ui_language_set(int language);
int ui_language_get();

#endif
