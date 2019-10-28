/*******************************************************************************
 * wdt_x85.h : redefine wdt_enable()   https://github.com/swellhunter/blinkwdt 
 * 
 * See WDTCR in full datasheet.
 * also avr/wdt.h from Atmel with Studio 7 for Windows.
 * also bigdanzblog : 
 * https://bigdanzblog.wordpress.com/2015/07/20/resetting-rebooting-attiny85-with-watchdog-timer-wdt/
 * 
 * WDIF WDIE WDP3 WDCE WDE  WDP2 WDP1 WDP0            refer "Watchdog Timer Control Register"
 *   1    1    0    1    1    0    0    0             WDPn's are prescaler bits.       
 *  
 *******************************************************************************/
#ifdef wdt_enable
   #undef wdt_enable
#endif

// Alrighty then. ATTiny85 does not have the XMEGA CCP register.
// There is no "good" wdt_enable() defined in avr/wdt.h 
// See WDTCR section in datasheet for the 'x' values.
// bigdanzblog is simpler, 0xD8|WDTO_value, but need to add 0x18 to 4S,8S.
// R16 is assumed pushed for stack frame. 
// AVR Studio 7 toolchain code was the prototype for this style.

//    in R16,SREG                          ; stash SREG
//    cli                                  ; disable interrupts
//    wdr                                  ; watchdog reset
//    out WDTCR, 0b11011000 | 0b00x00xxx   ; supply WDTO value but mind 8 and 9
//    out SREG,  R26 ; restore SREG        ; put SREG back

#define wdt_enable(value) \
__asm__ __volatile__ ( \
    "in __tmp_reg__,__SREG__" "\n\t"  \
    "cli" "\n\t"  \
    "wdr" "\n\t"  \
    "out %[WDTREG],%[WDVALUE]" "\n\t"  \
    "out __SREG__,__tmp_reg__" "\n\t"  \
    : /* no outputs */  \
    : [WDTREG] "I" (_SFR_IO_ADDR(_WD_CONTROL_REG)), \
      [WDVALUE] "r" ((uint8_t)(0xD8 \
      | (value & 0x08 ? _WD_PS3_MASK : 0x00) \
      | _BV(WDE) | (value & 0x07) )) \
    : "r16" \
)
