/*
  Taken from simh: 
  pdp11_rl.c: RL11 (RLV12) cartridge disk simulator

   Copyright (c) 1993-2013, Robert M Supnik
*/

#define SIMH_RL_BOOT_START      02000                           /* start */
#define SIMH_RL_BOOT_ENTRY      (SIMH_RL_BOOT_START + 002)              /* entry */
#define SIMH_RL_BOOT_UNIT       (SIMH_RL_BOOT_START + 010)              /* unit number */
#define SIMH_RL_BOOT_CSR        (SIMH_RL_BOOT_START + 020)              /* CSR */
#define SIMH_RL_BOOT_LEN        (sizeof (simh_rl02_boot_rom) / sizeof (uint16_t))

static const uint16_t simh_rl02_boot_rom[] = {
    0042114,                        /* "LD" */
    0012706, SIMH_RL_BOOT_START,    /* MOV #boot_start, SP */
    0012700, 0000000,               /* MOV #unit, R0 */
    0010003,                        /* MOV R0, R3 */
    0000303,                        /* SWAB R3 */
    0012701, 0174400,               /* MOV #RLCS, R1        ; csr */
    0012761, 0000013, 0000004,      /* MOV #13, 4(R1)       ; clr err */
    0052703, 0000004,               /* BIS #4, R3           ; unit+gstat */
    0010311,                        /* MOV R3, (R1)         ; issue cmd */
    0105711,                        /* TSTB (R1)            ; wait */
    0100376,                        /* BPL .-2 */
    0105003,                        /* CLRB R3 */
    0052703, 0000010,               /* BIS #10, R3          ; unit+rdhdr */
    0010311,                        /* MOV R3, (R1)         ; issue cmd */
    0105711,                        /* TSTB (R1)            ; wait */
    0100376,                        /* BPL .-2 */
    0016102, 0000006,               /* MOV 6(R1), R2        ; get hdr */
    0042702, 0000077,               /* BIC #77, R2          ; clr sector */
    0005202,                        /* INC R2               ; magic bit */
    0010261, 0000004,               /* MOV R2, 4(R1)        ; seek to 0 */
    0105003,                        /* CLRB R3 */
    0052703, 0000006,               /* BIS #6, R3           ; unit+seek */
    0010311,                        /* MOV R3, (R1)         ; issue cmd */
    0105711,                        /* TSTB (R1)            ; wait */
    0100376,                        /* BPL .-2 */
    0005061, 0000002,               /* CLR 2(R1)            ; clr ba */
    0005061, 0000004,               /* CLR 4(R1)            ; clr da */
    0012761, 0177000, 0000006,      /* MOV #-512., 6(R1)    ; set wc */
    0105003,                        /* CLRB R3 */
    0052703, 0000014,               /* BIS #14, R3          ; unit+read */
    0010311,                        /* MOV R3, (R1)         ; issue cmd */
    0105711,                        /* TSTB (R1)            ; wait */
    0100376,                        /* BPL .-2 */
    0042711, 0000377,               /* BIC #377, (R1) */
    0005002,                        /* CLR R2 */
    0005003,                        /* CLR R3 */
    0012704, SIMH_RL_BOOT_START+020,        /* MOV #START+20, R4 */
    0005005,                        /* CLR R5 */
    0005007                         /* CLR PC */
    };

