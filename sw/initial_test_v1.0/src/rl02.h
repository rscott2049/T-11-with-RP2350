/**
 * @file rl02.h
 * @brief DEC RL11 / RLV12 Controller and RL02 Cartridge Subsystem Definitions
 * 
 * Standard DEC Hardware Reference: EK-RL012-UG
 * PDP-11 Unibus Base Address: 01774400 (octal)
 * PDP-11 Interrupt Vector:    0160 (octal), BR5 priority
 */

#ifndef RL02_H
#define RL02_H

#include <stdint.h>
#include <stdbool.h>

#define RL11_BASE 0174400
#define RLCS_OFFSET 0
#define RLBA_OFFSET 2
#define RLDA_OFFSET 4
#define RLMP_OFFSET 6
#define RL11_VEC    0160

/* RL11 Memory-Mapped I/O Register Offsets (Word Addresses) */
#define RL11_REG_RLCS           0       /* Control Status Register (+0, 01774400) */
#define RL11_REG_RLBA           1       /* Bus Address Register    (+2, 01774402) */
#define RL11_REG_RLDA           2       /* Disk Address Register   (+4, 01774404) */
#define RL11_REG_RLMP           3       /* Multipurpose Register   (+6, 01774406) */

#define RL11_INT_VECTOR         0160    /* Standard PDP-11 interrupt vector (octal) */

/* RLCS - Control Status Register Bitfield (01774400) */
#define RLCS_DRDY               (1U << 0)   /* Bit 0: Drive Ready (Read-Only) */
#define RLCS_FN_MASK            (7U << 1)   /* Bits 1-3: Function Code */
#define RLCS_FN_SHIFT           1
#define RLCS_BA16               (1U << 4)   /* Bit 4: Bus Address Bit 16 */
#define RLCS_BA17               (1U << 5)   /* Bit 5: Bus Address Bit 17 */
#define RLCS_BA_EXT_MASK        (3U << 4)
#define RLCS_IE                 (1U << 6)   /* Bit 6: Interrupt Enable */
#define RLCS_CRDY               (1U << 7)   /* Bit 7: Controller Ready (0=Busy, 1=Ready) */
#define RLCS_DS_MASK            (3U << 8)   /* Bits 8-9: Drive Select */
#define RLCS_DS_SHIFT           8
#define RLCS_OPI                (1U << 10)  /* Bit 10: Operation Incomplete */
#define RLCS_CRC                (1U << 11)  /* Bit 11: Data/Header CRC Error */
#define RLCS_DLT                (1U << 12)  /* Bit 12: Data Late */
#define RLCS_NXM                (1U << 13)  /* Bit 13: Non-Existent Memory */
#define RLCS_DE                 (1U << 14)  /* Bit 14: Drive Error */
#define RLCS_ERR                (1U << 15)  /* Bit 15: Composite Error */

#define RLCS_WRITABLE_MASK      (RLCS_FN_MASK | RLCS_BA_EXT_MASK | RLCS_IE | RLCS_CRDY | RLCS_DS_MASK)
#define RLCS_ALL_ERRORS         (RLCS_OPI | RLCS_CRC | RLCS_DLT | RLCS_NXM | RLCS_DE | RLCS_ERR)



/* Function Codes (FN bits 1-3) */
typedef enum {
    RL_FN_NOP           = 0,    /* 000: No-Op / Maintenance */
    RL_FN_WRITE_CHECK   = 1,    /* 001: Write Check */
    RL_FN_GET_STATUS    = 2,    /* 010: Get Status */
    RL_FN_SEEK          = 3,    /* 011: Seek */
    RL_FN_READ_HEADER   = 4,    /* 100: Read Header */
    RL_FN_WRITE_DATA    = 5,    /* 101: Write Data */
    RL_FN_READ_DATA     = 6,    /* 110: Read Data */
    RL_FN_READ_DATA_NOC = 7     /* 111: Read Data Without Header Check */
} rl_function_t;

/* RLDA - Disk Address Register Bitfields (01774404) */
#define RLDA_RW_SECTOR_MASK     0x3FU       /* Bits 0-5: Sector Address (0-39) */
#define RLDA_RW_HEAD            (1U << 6)   /* Bit 6: Head Select (0 or 1) */
#define RLDA_RW_CYL_MASK        (0x1FFU << 7) /* Bits 7-15: Cylinder (0-511) */
#define RLDA_RW_CYL_SHIFT       7

#define RLDA_SEEK_MARKER        (1U << 0)
#define RLDA_SEEK_DIR_IN        (1U << 1)   /* 1=Inward, 0=Outward */
#define RLDA_SEEK_HEAD          (1U << 4)
#define RLDA_SEEK_DIFF_MASK     (0x1FFU << 7)
#define RLDA_SEEK_DIFF_SHIFT    7

#define RLDA_GS_MARKER          (1U << 0)
#define RLDA_GS_STATUS          (1U << 1)
#define RLDA_GS_RESET           (1U << 3)

/* RLMP - Drive Status Register (returned during Get Status) */
#define RLDS_STA_MASK           0x07U
#define RLDS_BH                 (1U << 3)
#define RLDS_HO                 (1U << 4)
#define RLDS_CO                 (1U << 5)
#define RLDS_HS                 (1U << 6)
#define RLDS_DT                 (1U << 7)   /* 1 = RL02, 0 = RL01 */
#define RLDS_DSE                (1U << 8)
#define RLDS_VC                 (1U << 9)
#define RLDS_WGE                (1U << 10)
#define RLDS_SPE                (1U << 11)
#define RLDS_SKTO               (1U << 12)
#define RLDS_WL                 (1U << 13)
#define RLDS_CHE                (1U << 14)
#define RLDS_WDE                (1U << 15)

#define RLDS_M_STATE    (07)
#define RLDS_LOAD       (0)                             /* no cartridge */
#define RLDS_SPIN       (1)                             /* spin-up */
#define RLDS_BRUSH      (2)                             /* brush cycle *! */
#define RLDS_HLOAD      (3)                             /* load heads */
#define RLDS_SEEK       (4)                             /* drive seeking * */
#define RLDS_LOCK       (5)                             /* lock on * */
#define RLDS_UNL        (6)                             /* unload heads */
#define RLDS_DOWN       (7)                             /* spin-down */


#define RLDS_ERR        (RLDS_WDE|RLDS_CHE|RLDS_SKTO|RLDS_SPE|RLDS_WGE| \
                         RLDS_VC|RLDS_DSE)             /* errors bits */


#define RL02_STATUS_READY_WORD  (RLDS_DT | RLDS_HO | RLDS_BH | 0x05U)

/* DEC RL02 Physical & Logical Disk Geometry */
#define RL02_CYLINDERS          512     /* 512 cylinders per cartridge */
#define RL02_HEADS              2       /* 2 heads/surfaces (Head 0, Head 1) */
#define RL02_SECTORS_PER_TRACK  40      /* 40 sectors per track (0 - 39) */
#define RL02_BYTES_PER_SECTOR   256     /* 256 bytes (128 words) per sector */
#define RL02_WORDS_PER_SECTOR   128     /* 128 16-bit words per sector */

#define RL02_TOTAL_SECTORS      (RL02_CYLINDERS * RL02_HEADS * RL02_SECTORS_PER_TRACK) /* 40,960 */
#define RL02_TOTAL_BYTES        (RL02_TOTAL_SECTORS * RL02_BYTES_PER_SECTOR)           /* 10,485,760 bytes (~10.48 MB) */

#define RL11_MAX_DRIVES         4       /* RL11 supports up to 4 disk drives */

typedef struct {
    bool        attached;
    bool        write_protected;
    uint16_t    current_cylinder;
    uint8_t     current_head;
    uint8_t     current_sector;
    uint16_t    status_word;
} rl02_drive_t;

typedef struct {
  rl02_drive_t drives[RL11_MAX_DRIVES];
  bool         have_file[RL11_MAX_DRIVES];
  uint8_t      selected_drive;
  uint16_t     header_word_count;
  void*        storage_handle;
} rl11_controller_t;

void rl11_init(rl11_controller_t* controller, void* storage_handle);
void rl11_reset(rl11_controller_t* controller);
//void rl11_process_pending_command(rl11_controller_t* controller);
void rl11_execute_operation(void);

void rl11_emu_init();
void rl11_emu(uint32_t io_addr, uint32_t io_data);
uint32_t rl11_load_0(uint32_t drive_idx);

#endif /* RL02_H */
