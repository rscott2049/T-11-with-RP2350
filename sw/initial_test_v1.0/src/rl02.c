/**
 * @file rl02.c
 * @brief DEC RL11 / RL02 Controller State Machine
 */

#include <string.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/dma.h"

#include "rl02.h"
//#include "t11_bus_interface.h"
#include "rl02_storage.h"
#include "t11_defines.h"

// Shared 64KB memory
extern uint16_t emu_mem[32768];
// Signals to main from ISR
extern volatile uint32_t rl11_cs_hit;
extern volatile uint32_t rl11_cs_go;

// To send interrupt to T-11
extern PIO pio;
extern uint sm_iack;

#define T11_SHARED_MEM_SIZE (64 * 1024)
uint8_t *g_t11_shared_memory = (uint8_t *)emu_mem;

//static rl11_controller_t s_controller;
//static rl02_storage_t    s_storage;

rl11_controller_t s_controller;
rl02_storage_t    s_storage;


// Captured by ISR RL command registers
uint32_t input_cs;
uint32_t input_ba;
uint32_t input_da;
uint32_t input_mp;

// Saved copy of RL command registers
uint32_t saved_cs;
uint32_t saved_ba;
uint32_t saved_da;
uint32_t saved_mp;
uint32_t saved_mp1;
uint32_t saved_mp2;



uint32_t rl11_irq_cmd; // Filled in by rl11_emu_init()

void rl11_init(rl11_controller_t* controller, void* storage_handle) {
  memset(controller, 0, sizeof(*controller));
  controller->storage_handle = storage_handle;
    
  for (int i = 0; i < RL11_MAX_DRIVES; i++) {
    controller->drives[i].current_cylinder = 0;
    controller->drives[i].current_head = 0;
    controller->drives[i].current_sector = 0;

    // Ready drives that have storage mounted
    if (controller->have_file[i]) {
      controller->drives[i].attached = true;
      controller->drives[i].status_word = RL02_STATUS_READY_WORD;
    }
  }
    
  emu_mem[(RL11_BASE + RLCS_OFFSET)/2] = RLCS_CRDY | RLCS_DRDY;
  emu_mem[(RL11_BASE + RLBA_OFFSET)/2] = 0;
  emu_mem[(RL11_BASE + RLDA_OFFSET)/2] = 0;
  emu_mem[(RL11_BASE + RLMP_OFFSET)/2] = 0;
  
}


void rl11_reset(rl11_controller_t* controller) {
  emu_mem[(RL11_BASE + RLCS_OFFSET)/2] = RLCS_CRDY | RLCS_DRDY;
  emu_mem[(RL11_BASE + RLBA_OFFSET)/2] = 0;
  emu_mem[(RL11_BASE + RLDA_OFFSET)/2] = 0;
  emu_mem[(RL11_BASE + RLMP_OFFSET)/2] = 0;
}

void t11_assert_interrupt(void) {
  if (saved_cs & RLCS_IE) {
    pio_sm_put(pio, sm_iack, rl11_irq_cmd);
  }
}

// Taken from simh pdp11_rl.c:
/* CRC16 as implemented by the DEC 9401 chip */
static uint16_t calcCRC (const int32_t wc, const uint16_t *data)
{
    uint32_t  crc, j, d;
    int32_t   i;

    crc = 0;
    for (i = 0; i < wc; i++) {
        d = *data++;
        /* cribbed from KG11-A */
        for (j = 0; j < 16; j++) {
            crc = (crc & ~01) | ((crc & 01) ^ (d & 01));
            crc = (crc & 01) ? (crc >> 1) ^ 0120001 : crc >> 1;
            d >>= 1;
        }
    }
    return (uint16_t)crc;
}

static const char * const funcname[] = {
    "NOP", "WCK", "GSTA", "SEEK",
    "RHDR", "WT", "RD", "RNOHDR", "SPECIAL",
};

uint32_t exec_count = 0;
rl02_drive_t* drive;
volatile uint32_t last_cs;

void rl11_execute_operation(void) {

  // Copy inputs to working
  // CRDY is still zero, so T11 shouldn't be changing contents
#if 1
  //  if (input_cs & 0x80) printf("I"); else printf("\\");
  saved_cs = input_cs;
  saved_ba = input_ba;
  saved_da = input_da;
  saved_mp = input_mp;
#endif

  //  printf("cs: %06o\n", saved_cs);
  uint8_t fn = (saved_cs & RLCS_FN_MASK) >> RLCS_FN_SHIFT;
  uint8_t drive_idx = (saved_cs & RLCS_DS_MASK) >> RLCS_DS_SHIFT;

#if 0
  if (drive_idx >= RL11_MAX_DRIVES) {
    emu_mem[(RL11_BASE + RLCS_OFFSET)/2] |= (RLCS_OPI | RLCS_ERR | RLCS_CRDY);
    emu_mem[(RL11_BASE + RLBA_OFFSET)/2] = saved_ba;
    emu_mem[(RL11_BASE + RLDA_OFFSET)/2] = saved_da;
    emu_mem[(RL11_BASE + RLMP_OFFSET)/2] = saved_mp;

    t11_assert_interrupt();
    return;
  }
#endif    

  rl02_drive_t* drive = &s_controller.drives[drive_idx];

#if 0
  printf("%d fn: %d %s\n", exec_count, fn, funcname[fn]);
  printf("cs: %06o\n", saved_cs);
  printf("da: %06o\n", saved_da);
  printf("ba: %06o\n", saved_ba);
  printf("mp: %06o %06o %06o\n", saved_mp, saved_mp1, saved_mp2);
  printf("cyl: %d\n", drive->current_cylinder);
  printf("head: %d\n", drive->current_head);
  printf("sect: %d\n", drive->current_sector);
#endif

  switch (fn) {
  case RL_FN_NOP: {
    /*
    extern uint32_t bus_op_dma_chan;
    extern uint32_t bus_op_cp_dma_chan;
    extern uint32_t cmd_fetch_dma_chan;
    extern uint32_t cmd_exe_dma_chan;
    extern uint32_t la_dma_chan;
    extern void reset_dma();
    extern volatile uint32_t bus_op_cmd;


      dma_channel_abort(bus_op_dma_chan);
      dma_channel_abort(bus_op_cp_dma_chan);
      dma_channel_abort(cmd_fetch_dma_chan);
      dma_channel_abort(cmd_exe_dma_chan);
      dma_channel_abort(la_dma_chan);
    */

    //    reset_dma();


    //volatile uint32_t save_cmd_before = bus_op_cmd;
    //saved_cs = -1;
    // At 3.75 MHz
    //sleep_us(174);
    // At 7.3 MHz
    //sleep_us(94);

#if 0
    uint32_t cmd_save[8];
    for (int i = 0; i < 8; i++) {
      cmd_save[i] = *(uint32_t *)(0x20030300 + (i*4));
    }

    volatile uint32_t save_cmd_after;
    for (int i = 0; i < 5000; i++) {
      save_cmd_after = bus_op_cmd;
      if (save_cmd_after != 0x20030000) {
	printf("cmd count: %d\n", i);
	break;
      }
    }
#endif

#if 0
    printf("before\n");
    for (int i = 0; i < 8; i++) {
      printf("add/data: %08x %08x\n",
	     0x20030300 + (i * 4), *(uint32_t *)(0x20030300 + i*4));
    }

    printf("after\n");
    for (int i = 0; i < 8; i++) {
      printf("add/data: %08x %08x\n",
	     0x20030300 + (i * 4), *(uint32_t *)(0x20030300 + (i*4)));
    }
#endif

    // printf("cs: %06o\n", saved_cs);
    //  printf("drive index: %d\n", drive_idx);
    //  printf("addr cs: %08x\n", (uint32_t)&saved_cs);
    //printf("cs: %06o\n", saved_cs);
    //  rl02_drive_t* drive = &s_controller.drives[drive_idx];
    //printf("cs: %06o\n", saved_cs);
#if 0
    printf("bus op before/after: %08x %08x\n", save_cmd_before, save_cmd_after);
    printf("DMA busy: ");
    if (dma_channel_is_busy(bus_op_dma_chan)) printf("bus_op_dma_chan\n");
    if (dma_channel_is_busy(bus_op_cp_dma_chan)) printf("bus_op_cp_dma_chan\n");
    if (dma_channel_is_busy(cmd_fetch_dma_chan)) printf("cmd_fetch_dma_chan\n");
    if (dma_channel_is_busy(cmd_exe_dma_chan)) printf("cmd_exe_dma_chan\n");
    if (dma_channel_is_busy(la_dma_chan)) printf("la_dma_chan\n");


    printf("drive: %08x size: %08x\n", (uint32_t)drive, sizeof(drive));
    printf("drive type size: %d\n", sizeof(rl02_drive_t));
    printf("addr cs: %08x %08x\n", (uint32_t)&saved_cs, saved_cs);
    printf("addr drive: %08x %08x\n", (uint32_t)&drive, (uint32_t)drive);

    drive = &s_controller.drives[drive_idx];
#endif

    saved_cs = 0112201;
    saved_da = 0000006;
    saved_ba = 0002000;

    break;
  }
        
  case RL_FN_GET_STATUS: {
    // Only update status if the GS bit is set in the DA reg
    if (!(saved_da & RLDA_GS_RESET)) {
      // Set error bits for incomplete command received
      saved_cs = RLCS_ERR | RLCS_OPI;
      break;
    }

    // Handle reset request
    if (saved_da & RLDA_GS_RESET) {
      drive->status_word &= ~(RLDS_ERR);
    }

    // Return drive state
    saved_mp = drive->status_word;
    // Current head selected
    saved_mp |= (drive->current_head) ? RLDS_HS : 0;
    // Always an RL02
    saved_mp |= RLDS_DT;
    // Not write protected
    saved_mp &= ~RLDS_WL;

    // Hack for now - always return lock on, brush home and heads over disk
    saved_mp |= RLDS_LOCK | RLDS_BH | RLDS_HO;

    saved_mp1 = saved_mp;
    saved_mp2 = saved_mp1;

    break;
  }
        
  case RL_FN_SEEK: {
    uint16_t da = saved_da;
    uint16_t diff = (da & RLDA_SEEK_DIFF_MASK) >> RLDA_SEEK_DIFF_SHIFT;
    bool dir_inward = (da & RLDA_SEEK_DIR_IN) != 0;
    uint8_t head = (da & RLDA_SEEK_HEAD) ? 1 : 0;
            
    drive->current_head = head;
            
    if (dir_inward) {
      if (drive->current_cylinder + diff >= RL02_CYLINDERS) {
	drive->current_cylinder = RL02_CYLINDERS - 1;
	saved_cs |= (RLCS_OPI | RLCS_ERR);
      } else {
	drive->current_cylinder += diff;
      }
    } else {
      if (drive->current_cylinder < diff) {
	drive->current_cylinder = 0;
      } else {
	drive->current_cylinder -= diff;
      }
    }
    //printf("seek cyl: %d\n", drive->current_cylinder);
    break;
  }

    /*
      The Read Header command reads the first header found on the
      selected drive and stores the three header words in the FIFO RAM. The
      first word, WD 1, includes the cylinder address, the head selected,
      and the sector address. The second word, WD2, is all zeros. The third
      word, WD3, has the header CRC information. These words can be read
      from the FIFO RAM buffer by consecutive read MPR instructions. Three
      read MPR instructions are needed to read three FIFO words. Reading the
      first header word provides enough head positioning information to
      permit software computation of the cylin- der difference for another
      Seek command to a new track address.
    */

  case RL_FN_READ_HEADER: {
    uint16_t hdr[2];

    //printf("rhdr cyl: %d\n", drive->current_cylinder);
    // Calculate track number
    hdr[0] = (drive->current_cylinder << 7) | 
      (drive->current_head << 6) | 
      (drive->current_sector);
    //printf("rhdr trk: %d\n", hdr[0]);
    hdr[1] = 0;

    saved_mp = hdr[0];
    saved_mp1 = hdr[1];
    saved_mp2 = calcCRC (2, &hdr[0]);
    // Simulate rotation:
    drive->current_sector++;
    if (drive->current_sector >= RL02_SECTORS_PER_TRACK) {
      drive->current_sector = 0;
    }

    break;
  }
        
  case RL_FN_READ_DATA:
  case RL_FN_READ_DATA_NOC: {

    // Read Data: Direct transfer from SD image (f_read) into T11 memory
    uint16_t cyl = (saved_da & RLDA_RW_CYL_MASK) >> RLDA_RW_CYL_SHIFT;
    uint8_t head = (saved_da & RLDA_RW_HEAD) ? 1 : 0;
    uint8_t sector = (saved_da & RLDA_RW_SECTOR_MASK);
            
    uint32_t t11_addr = saved_ba;
    int16_t words_left = (int16_t)saved_mp;
    //printf("left: %d\n", words_left);
    uint16_t total_words = (uint16_t)(-words_left);
    if (total_words == 0) total_words = RL02_WORDS_PER_SECTOR;
            
    while (words_left < 0) {
      if (sector >= RL02_SECTORS_PER_TRACK || cyl >= RL02_CYLINDERS) {
	saved_cs |= (RLCS_OPI | RLCS_ERR);
	break;
      }
                
      uint16_t words_this_sector = (-words_left > RL02_WORDS_PER_SECTOR) ?
	RL02_WORDS_PER_SECTOR : (-words_left);
      uint32_t bytes_this_sector = (uint32_t)words_this_sector * 2;
                
      /* Bounds check against 64KB shared memory */
      if (t11_addr + bytes_this_sector > T11_SHARED_MEM_SIZE) {
	saved_cs |= (RLCS_NXM | RLCS_ERR);
	break;
      }
                
      uint32_t lba = rl02_chs_to_lba(cyl, head, sector);
                
#if 0
      printf("read: %04d to: %08x, size %d %04x\n", lba,
	     (uint32_t)&g_t11_shared_memory[t11_addr],
	     bytes_this_sector, bytes_this_sector);
#endif

      /* DIRECT TRANSFER: FatFS f_read directly into g_t11_shared_memory */
      if (!rl02_storage_read_direct(&s_storage, drive_idx, lba, 
				    &g_t11_shared_memory[t11_addr], bytes_this_sector)) {
	saved_cs |= (RLCS_CRC | RLCS_ERR);
	break;
      }
                
      words_left += words_this_sector;
      t11_addr += bytes_this_sector;
      sector++;
                
      if (sector >= RL02_SECTORS_PER_TRACK) {
	sector = 0;
	head++;
	if (head >= RL02_HEADS) {
	  head = 0;
	  cyl++;
	}
      }
    }
            
    saved_mp = (uint16_t)words_left;
    saved_ba = (uint16_t)(t11_addr & 0xFFFF);
    saved_da = (cyl << RLDA_RW_CYL_SHIFT) | (head << 6) | sector;
    drive->current_cylinder = cyl;
    drive->current_head = head;
    drive->current_sector = sector;
    break;
  }
  case RL_FN_WRITE_DATA: {
    /* Write Data: Direct transfer from 64KB shared memory to SD image (f_write) */
    uint16_t cyl = (saved_da & RLDA_RW_CYL_MASK) >> RLDA_RW_CYL_SHIFT;
    uint8_t head = (saved_da & RLDA_RW_HEAD) ? 1 : 0;
    uint8_t sector = (saved_da & RLDA_RW_SECTOR_MASK);
            
    if (drive->write_protected) {
      saved_cs |= (RLCS_DE | RLCS_ERR);
      break;
    }
            
    uint32_t t11_addr = saved_ba;
    int16_t words_left = (int16_t)saved_mp;
            
    while (words_left < 0) {
      if (sector >= RL02_SECTORS_PER_TRACK || cyl >= RL02_CYLINDERS) {
	saved_cs |= (RLCS_OPI | RLCS_ERR);
	break;
      }
                
      uint16_t words_this_sector = (-words_left > RL02_WORDS_PER_SECTOR) ?
	RL02_WORDS_PER_SECTOR : (-words_left);
      uint32_t bytes_this_sector = (uint32_t)words_this_sector * 2;
                
      if (t11_addr + bytes_this_sector > T11_SHARED_MEM_SIZE) {
	saved_cs |= (RLCS_NXM | RLCS_ERR);
	break;
      }
                
      uint32_t lba = rl02_chs_to_lba(cyl, head, sector);
                
      /* DIRECT TRANSFER: FatFS f_write directly from g_t11_shared_memory */
      if (!rl02_storage_write_direct(&s_storage, drive_idx, lba,
				     &g_t11_shared_memory[t11_addr], bytes_this_sector)) {
	saved_cs |= (RLCS_DE | RLCS_ERR);
	break;
      }
                
      words_left += words_this_sector;
      t11_addr += bytes_this_sector;
      sector++;
                
      if (sector >= RL02_SECTORS_PER_TRACK) {
	sector = 0;
	head++;
	if (head >= RL02_HEADS) {
	  head = 0;
	  cyl++;
	}
      }
    }
            
    saved_mp = (uint16_t)words_left;
    saved_ba = (uint16_t)(t11_addr & 0xFFFF);
    saved_da = (cyl << RLDA_RW_CYL_SHIFT) | (head << 6) | sector;
    drive->current_cylinder = cyl;
    drive->current_head = head;
    drive->current_sector = sector;

    break;
  }
        
  case RL_FN_WRITE_CHECK: {
    break;
  }
        
  default:
    saved_cs |= (RLCS_OPI | RLCS_ERR);
    break;
  }
    
  saved_cs |= (RLCS_CRDY | RLCS_DRDY);

#if 0
  printf("%d done\n", exec_count);
  printf("cs: %06o\n", saved_cs);
  printf("da: %06o\n", saved_da);
  printf("ba: %06o\n", saved_ba);
  printf("mp: %06o %06o %06o\n", saved_mp, saved_mp1, saved_mp2);
  printf("cyl: %d\n", drive->current_cylinder);
  printf("head: %d\n", drive->current_head);
  printf("sect: %d\n", drive->current_sector);
#endif
  exec_count++;
  if (exec_count > 10) while(1);
  //if (exec_count > 4) while(1);

  last_cs = saved_cs;

  sleep_us(20);


  // Done with command
  emu_mem[(RL11_BASE + RLBA_OFFSET)/2] = saved_ba;
  emu_mem[(RL11_BASE + RLDA_OFFSET)/2] = saved_da;
  emu_mem[(RL11_BASE + RLMP_OFFSET)/2] = saved_mp;
  emu_mem[(RL11_BASE + RLCS_OFFSET)/2] = saved_cs;
  t11_assert_interrupt();

}


// Setup RL emulated state, attach drives, etc.
void rl11_emu_init() {

  // Open emulated disk storage
  //rl02_storage_init(&s_storage);
  s_controller.have_file[0] = rl02_storage_mount(&s_storage, 0, "rl02_0.dsk");
  s_controller.have_file[1] = rl02_storage_mount(&s_storage, 1, "rl02_1.dsk");
  s_controller.have_file[2] = false;
  s_controller.have_file[3] = false;

  // Setup hardware state
  rl11_init(&s_controller, &s_storage);

  // Fill in irq_cmd
  rl11_irq_cmd = (RL11_VEC << 16) | pio_encode_set(pio_pins, T11_CP_5);

}

// Runs at ISR level, so be quick!
void rl11_emu(uint32_t addr, uint32_t wr_data) {
  
  // Four words of RL11 registers
  uint32_t reg_sel = addr & 0x7;

  uint32_t wr = (~addr & T11_RD_MASK);

  // Emulate the MPR register read
  if (wr == 0) {
    if (reg_sel == RLMP_OFFSET) {
      emu_mem[(RL11_BASE + RLMP_OFFSET)/2] = saved_mp1;
      saved_mp1 = saved_mp2;
    }
  } else {
    if (reg_sel == RLCS_OFFSET) {
#if 0
      //      if ((wr_data & 0x80) == 0x80) {
      if ((last_cs & 0x80) == 0x80) {
	printf("D");
      } else {
	printf("_");
      }

      if ((emu_mem[(RL11_BASE + RLCS_OFFSET)/2] & 0x80) == 0x80) {
	printf("M");
      } else {
	printf("_");
      }

      if ((saved_cs & 0x80) == 0x80) {
	printf("S");
      } else {
	printf("_");
      }

      printf("|");
#endif
      // Commands are only executed while CSR_DONE bit is cleared by sw
#if 0

      if (rl11_cs_hit == 1) {
	//	printf("%06o %04x %06o %04x\n",
	//     emu_mem[(RL11_BASE + RLCS_OFFSET)/2],
	//     emu_mem[(RL11_BASE + RLCS_OFFSET)/2],
	//     wr_data & 0xffff, wr_data & 0xffff);
	printf("B");
	//while(1);
      }
#endif

      rl11_cs_hit = 1;

      // If we're writing a zero to CRDY, new command
      if ((wr_data & RLCS_CRDY) == 0) {
	//printf("C");
	// Save registers
#if 1
	input_cs = wr_data & 0xffff;
	input_ba = emu_mem[(RL11_BASE + RLBA_OFFSET)/2];
	input_da = emu_mem[(RL11_BASE + RLDA_OFFSET)/2];
	input_mp = emu_mem[(RL11_BASE + RLMP_OFFSET)/2];
#else
	saved_cs = wr_data & 0xffff;
	saved_ba = emu_mem[(RL11_BASE + RLBA_OFFSET)/2];
	saved_da = emu_mem[(RL11_BASE + RLDA_OFFSET)/2];
	saved_mp = emu_mem[(RL11_BASE + RLMP_OFFSET)/2];
#endif

	// Clear error bits
	emu_mem[(RL11_BASE + RLCS_OFFSET)/2] &= ~(RLCS_ALL_ERRORS);

	// In theory, should zero out emu_mem, as RL11 "locks out" access to regs
#if 0
	emu_mem[(RL11_BASE + RLCS_OFFSET)/2] = 0;
	emu_mem[(RL11_BASE + RLBA_OFFSET)/2] = 0;
	emu_mem[(RL11_BASE + RLDA_OFFSET)/2] = 0;
	emu_mem[(RL11_BASE + RLMP_OFFSET)/2] = 0;
#endif    
	// Signal mainline that we have RL work to do
	rl11_cs_go = 1;
      } else {
	//printf("_");
      }
    }
  }
}

// Read block zero into emulated memory starting at location 0
uint32_t rl11_load_0(uint32_t drive_idx) {

  if (!rl02_storage_read_direct(&s_storage, drive_idx, 0, 
				&g_t11_shared_memory[0], 512)) {
    printf("Failed to read block zero from drive %d\n", drive_idx);
    return 1;
  }

  //printf("Boot block loaded: chksum: %ld\n",
  //	 cksum((uint8_t *)(&emu_mem[0]), 512));

  return 0;
}
