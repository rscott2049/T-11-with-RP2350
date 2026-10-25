/**
 * @file rl02_storage.c
 * @brief SD Card Storage Implementation using no-OS-FatFS-SD-SDIO-SPI-RPi-Pico
 */

#include "rl02_storage.h"
#include <stdio.h>
#include <string.h>

#if 0
#if !__has_include("ff.h")
FRESULT f_mount(FATFS* fs, const char* path, uint8_t opt) { (void)fs; (void)path; (void)opt; return FR_OK; }
FRESULT f_open(FIL* fp, const char* path, uint8_t mode) { (void)fp; (void)path; (void)mode; return FR_OK; }
FRESULT f_close(FIL* fp) { (void)fp; return FR_OK; }
FRESULT f_sync(FIL* fp) { (void)fp; return FR_OK; }
FRESULT f_lseek(FIL* fp, FSIZE_t ofs) { (void)fp; (void)ofs; return FR_OK; }
FRESULT f_read(FIL* fp, void* buff, UINT btr, UINT* br) {
    (void)fp;
    memset(buff, 0, btr);
    *br = btr;
    return FR_OK;
}
FRESULT f_write(FIL* fp, const void* buff, UINT btw, UINT* bw) {
    (void)fp; (void)buff;
    *bw = btw;
    return FR_OK;
}
#endif
#endif

bool rl02_storage_init(rl02_storage_t* storage) {
    memset(storage, 0, sizeof(*storage));
    
    /* Mount SD Card root filesystem using FatFS */
    FRESULT fr = f_mount(&storage->fatfs, "", 1);
    if (fr != FR_OK) {
        printf("[STORAGE] SD card mount warning (code %d).", fr);
    } else {
        printf("[STORAGE] FatFS SD card filesystem mounted successfully.\n");
    }
    return true;
}

bool rl02_storage_mount(rl02_storage_t* storage, uint8_t drive_idx, const char* filename) {
    if (drive_idx >= RL11_MAX_DRIVES) return false;
    
    strncpy(storage->image_filenames[drive_idx], filename, sizeof(storage->image_filenames[drive_idx]) - 1);
    
    FRESULT fr = f_open(&storage->fil_handles[drive_idx], filename, FA_READ | FA_WRITE);
    if (fr == FR_OK) {
        storage->is_mounted[drive_idx] = true;
        printf("[STORAGE] Drive %d attached to SD image: %s (FatFS)\n", drive_idx, filename);

	// Maks sure size is correct
	FSIZE_t size = f_size(&storage->fil_handles[drive_idx]);
	if (size != RL02_TOTAL_BYTES) {
	  printf("[STORAGE] SD image: %s exp/act/delta size: %d %d: %d\n",
		 filename, RL02_TOTAL_BYTES, size, RL02_TOTAL_BYTES - size);
	  storage->is_mounted[drive_idx] = false;
	  return false;
	}

        return true;
    }
    
    printf("[STORAGE] Failed to open %s (code %d).\n", filename, fr, drive_idx);
    storage->is_mounted[drive_idx] = false;
    return false;
}

void rl02_storage_unmount(rl02_storage_t* storage, uint8_t drive_idx) {
    if (drive_idx >= RL11_MAX_DRIVES || !storage->is_mounted[drive_idx]) return;
    
    f_sync(&storage->fil_handles[drive_idx]);
    f_close(&storage->fil_handles[drive_idx]);
    storage->is_mounted[drive_idx] = false;
    printf("[STORAGE] Drive %d unmounted.\n", drive_idx);
}

bool rl02_storage_read_direct(rl02_storage_t* storage, uint8_t drive,
                             uint32_t lba, uint8_t* shared_mem_dest, uint32_t byte_count) {
    if (drive >= RL11_MAX_DRIVES || !storage->is_mounted[drive]) return false;
    if (lba >= RL02_TOTAL_SECTORS) return false;
    
    FIL* fp = &storage->fil_handles[drive];
    FSIZE_t byte_offset = (FSIZE_t)lba * RL02_BYTES_PER_SECTOR;
    
    FRESULT fr = f_lseek(fp, byte_offset);
    if (fr != FR_OK) {
        printf("[STORAGE] f_lseek error: %d at offset %lu\n", fr, (unsigned long)byte_offset);
        return false;
    }
    
    /* DIRECT READ: streams bytes directly into 64KB shared memory */
    UINT bytes_read = 0;
    fr = f_read(fp, shared_mem_dest, (UINT)byte_count, &bytes_read);
    if (fr != FR_OK || bytes_read != byte_count) {
        printf("[STORAGE] f_read error: code %d, read %u / %lu bytes\n",
               fr, bytes_read, (unsigned long)byte_count);
        return false;
    }
    
    return true;
}

bool rl02_storage_write_direct(rl02_storage_t* storage, uint8_t drive,
                              uint32_t lba, const uint8_t* shared_mem_src, uint32_t byte_count) {
    if (drive >= RL11_MAX_DRIVES || !storage->is_mounted[drive]) return false;
    if (lba >= RL02_TOTAL_SECTORS) return false;
    
    FIL* fp = &storage->fil_handles[drive];
    FSIZE_t byte_offset = (FSIZE_t)lba * RL02_BYTES_PER_SECTOR;
    
    FRESULT fr = f_lseek(fp, byte_offset);
    if (fr != FR_OK) {
        printf("[STORAGE] f_lseek error: %d at offset %lu\n", fr, (unsigned long)byte_offset);
        return false;
    }
    
    /* DIRECT WRITE: streams bytes directly from 64KB shared memory to SD */
    UINT bytes_written = 0;
    fr = f_write(fp, shared_mem_src, (UINT)byte_count, &bytes_written);
    if (fr != FR_OK || bytes_written != byte_count) {
        printf("[STORAGE] f_write error: code %d, written %u / %lu bytes\n",
               fr, bytes_written, (unsigned long)byte_count);
        return false;
    }
    
    f_sync(fp);
    return true;
}
