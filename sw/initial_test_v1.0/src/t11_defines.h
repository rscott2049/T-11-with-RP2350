/**
 * @file t11_defines.h
 * @brief RP2350 pin utilization for T11 I/O and memory emulation
 * 
 */

#ifndef T11_DEFINES_H
#define T11_DEFINES_H

#include <stdint.h>
#include <stdbool.h>


// T-11 +5v enable GPIO
#define VCC_EN_PIN 17
// T-11 Bus Clear
#define T11_BCLR_PIN 18
// T-11 clock
#define T11_CLK_PIN 35
// T-11 RAS/CAS/PI/SEL/CP/VEC
#define T11_SEL1_PIN 36
#define T11_SEL0_PIN 37
#define T11_RWHB_PIN 38
#define T11_RWLB_PIN 39
#define T11_RAS_PIN 40
#define T11_CAS_PIN 41
#define T11_PI_PIN  42
//#define T11_VEC_PIN 47
#define T11_HLT_PIN 47
#define T11_CP0_PIN 46
#define T11_CP1_PIN 45
#define T11_CP2_PIN 44
#define T11_CP3_PIN 43

#define T11_BUS_STATE_LSB T11_SEL1_PIN
// 7 pins for bus state
#define T11_BUS_STATE_MASK (((1ll << 7) - 1) << T11_BUS_STATE_LSB)
// Bus state masks
#define T11_BUS_STATE_RW 0xc0
#define T11_BUS_STATE_RWHB 0x40
#define T11_BUS_STATE_RWLB 0x80
#define T11_BUS_STATE_FETCH 0x02
#define T11_BUS_STATE_RAS (T11_RAS_PIN - T11_BUS_STATE_LSB)


// DAL defines
#define T11_DAL_LSB 19
#define T11_DAL_MASK (0xffffll << T11_DAL_LSB)
#define T11_DAL0_PIN 19
#define T11_DAL15_PIN 34

// Read/write defines
#define T11_RD_MASK (0x3l << (T11_RWHB_PIN - T11_DAL_LSB))

// T-11 PUP mode register:
// <0> - 1 select processor clock
// <1> - 1 select standard microcycle
// <8> - 0 select normal read/write
// <9> - 1 select static memory
// <10> - 1 (N/A, since we're static)
// <11> - 0 select 16-bit bus
// <12> - 1 select user mode
// <15:13> - 0b111 for 0o172000 start address
// <15:13> - 0b110 for 0o173000 start address
// <15:13> - 0b101 for 0o000000 start address

#define T11_START 6
#if (T11_START == 7)
#define T11_ROM_BASE 0172000
#elif (T11_START == 6)
#define T11_ROM_BASE 0173000
#elif (T11_START == 5)
#define T11_ROM_BASE 0
#endif

#define T11_MODE ((T11_START << 13) | (1 << 12) | (1 << 9) | 3)
//#define T11_MODE ((5 << 13) | (1 << 12) | (1 << 9) | 3)
//#define T11_MODE ((6 << 13) | (1 << 12) | (1 << 9) | 3)
//#define T11_MODE ((7 << 13) | (1 << 12) | (1 << 9) | 3)


// T-11 Coded Priority bits
// These are reversed from the data sheet, as the HW is bit reversed
#define T11_CP_4 0b0111
#define T11_CP_5 0b1101
#define T11_CP_6 0b1110
#define T11_CP_7 0b0000

#endif // T11_DEFINES_H
