/**
 * @file rl02_storage.h
 * @brief SD Card Block Storage Driver using no-OS-FatFS-SD-SDIO-SPI-RPi-Pico
 */

#ifndef RL02_STORAGE_H
#define RL02_STORAGE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "rl02.h"
#include "ff.h"

#if 0
#if __has_include("ff.h")
#include "ff.h"
#else
typedef uint32_t FSIZE_t;
typedef uint32_t UINT;
typedef enum {
    FR_OK = 0,
    FR_DISK_ERR,
    FR_INT_ERR,
    FR_NOT_READY,
    FR_NO_FILE,
    FR_NO_PATH,
    FR_INVALID_NAME,
    FR_DENIED,
    FR_EXIST,
    FR_INVALID_OBJECT,
    FR_WRITE_PROTECTED,
    FR_INVALID_DRIVE,
    FR_NOT_ENABLED,
    FR_NO_FILESYSTEM,
    FR_MKFS_ABORTED,
    FR_TIMEOUT,
    FR_LOCKED,
    FR_NOT_ENOUGH_CORE,
    FR_TOO_MANY_OPEN_FILES,
    FR_INVALID_PARAMETER
} FRESULT;

#define FA_READ     0x01
#define FA_WRITE    0x02
#define FA_OPEN_ALWAYS   0x10

typedef struct {
    void* obj;
    uint32_t fsize;
    uint32_t fptr;
} FIL;

typedef struct {
    uint8_t fs_type;
} FATFS;

FRESULT f_mount(FATFS* fs, const char* path, uint8_t opt);
FRESULT f_open(FIL* fp, const char* path, uint8_t mode);
FRESULT f_close(FIL* fp);
FRESULT f_read(FIL* fp, void* buff, UINT btr, UINT* br);
FRESULT f_write(FIL* fp, const void* buff, UINT btw, UINT* bw);
FRESULT f_lseek(FIL* fp, FSIZE_t ofs);
FRESULT f_sync(FIL* fp);
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    FATFS fatfs;
    FIL   fil_handles[RL11_MAX_DRIVES];
    bool  is_mounted[RL11_MAX_DRIVES];
    char  image_filenames[RL11_MAX_DRIVES][64];
} rl02_storage_t;

bool rl02_storage_init(rl02_storage_t* storage);
bool rl02_storage_mount(rl02_storage_t* storage, uint8_t drive_idx, const char* filename);
void rl02_storage_unmount(rl02_storage_t* storage, uint8_t drive_idx);

bool rl02_storage_read_direct(rl02_storage_t* storage, uint8_t drive,
                             uint32_t lba, uint8_t* shared_mem_dest, uint32_t byte_count);

bool rl02_storage_write_direct(rl02_storage_t* storage, uint8_t drive,
                              uint32_t lba, const uint8_t* shared_mem_src, uint32_t byte_count);

static inline uint32_t rl02_chs_to_lba(uint16_t cylinder, uint8_t head, uint8_t sector) {
    return ((uint32_t)cylinder * RL02_HEADS + head) * RL02_SECTORS_PER_TRACK + sector;
}

#ifdef __cplusplus
}
#endif



#endif /* RL02_STORAGE_H */
