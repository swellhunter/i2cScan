.include "tn85def.inc"
.org 0
       rjmp   reset          ; Reset Handler
       rjmp   EXT_INT0       ; IRQ0 Handler
       rjmp   PCINT0H        ; PCINT0 Handler - note 'H'
       rjmp   TIM1_COMPA     ; Timer1 CompareA Handler
       rjmp   TIM1_OVF       ; Timer1 Overflow Handler
       rjmp   TIM0_OVF       ; Timer0 Overflow Handler
       rjmp   EE_RDY         ; EEPROM Ready Handler
       rjmp   ANA_COMP       ; Analog Comparator Handler
       rjmp   ADC            ; ADC Conversion Handler
       rjmp   TIM1_COMPB     ; Timer1 CompareB Handler
       rjmp   TIM0_COMPA     ;
       rjmp   TIM0_COMPB     ;
       rjmp   WDT            ;
       rjmp   USI_START      ;
       rjmp   USI_OVF        ;

.org 15
       ;-------------------------------------------------
       ; Unless we used a crowbar, or the device is
       ; just powered up, all sorts of things are not
       ; initialised. For simple projects this is not
       ; important but for everything else we fix SP
       ; and call off fluffy.
       ;-------------------------------------------------

reset: ldi     r16, low (RAMEND)
       out     SPL, r16      ; Set Stack Pointer to top of RAM
       ldi     r16, high(RAMEND)
       out     SPH, r16      ; Tiny85 has also SPH, only just..

       cli                   ; Disable interrupts
       rcall   WDT_off       ; Disable WDT
       sei                   ; Enable interrupts

       ldi     r16,0b00000011
       out     ddrb,r16      ; PB0, PB1 as output
       ldi     r16,1
       out     portb,r16     ; the LED is on, dude

       ;-------------------------------------------------
       ; We are going to toggle the LED 7 times which
       ; means off and three more flashes for a total of
       ; four. This should be an odd number or every 2nd
       ; restart is a complementary pattern with the LED
       ; on after we "trap the sparx" with rjmp PC.
       ;-------------------------------------------------

       ldi     r16,7
loop:  rcall   halfcycle
       dec     r16
       brne    loop

       ldi     r16, 0b11011110 ;1s
;      ldi     r16, 0b11111001 ;8s
       out     WDTCR, r16
       rcall   init1
;      rjmp    reset
       rjmp    PC            ; forever

data:  .db 0xAA, 0x20, 0, 0, 0xAA, 0x25, "Hello World", 0x0D

init1: push    r15
       in      r15, SREG
       push    r0
       push    r16
       push    r30
       push    r31
       ldi     r16,18                ; 18 bytes of data
       ldi     ZH,high(data << 1)
       ldi     ZL,low (data << 1)
msg:   lpm     r0,Z+
       ; call the serial routine here
       dec     r16
       brne    msg
       pop     r31
       pop     r30
       pop     r16
       pop     r0
       out     SREG, r15   
       pop     r15
       ret

WDT_off:       ; from the datasheet, largely   
       push    r15
       in      r15, SREG           ;stash it.
       push    r16
       wdr
       ldi     r16, (0<<WDRF)
       out     MCUSR, r16      ; Clear WDRF in MCUSR
       ; Write logical one to WDCE and WDE
       ; Keep old prescaler setting to prevent
       ; unintentional Watchdog Reset
       in      r16, WDTCR
       ori     r16, (1<<WDCE)|(1<<WDE)
       out     WDTCR, r16
       ldi     r16, (0<<WDE) ; Turn off WDT
       out     WDTCR, r16
       pop     r16
       out     SREG, r15      ; restore it 
       pop     r15
       ret

halfcycle:
       push    r15
       in      r15, SREG           ;stash it.
       push    r24
       push    r25
       clr     r24
       clr     r25
       sbiw    r24,1         ; slight wait
       brne    PC-1
       sbi     pinb,pinb0    ; toggle the LED state
       pop     r25
       pop     r24
       out     SREG, r15      ; restore it 
       pop     r15
       ret

EXT_INT0:      ; IRQ0 Handler
       reti

PCINT0H:       ; PCINT0 Handler
       reti

TIM1_COMPA:    ; Timer1 CompareA Handler
       reti

TIM1_OVF:      ; Timer1 Overflow Handler
       reti

TIM0_OVF:      ; Timer0 Overflow Handler
       reti

EE_RDY:        ; EEPROM Ready Handler
       reti

ANA_COMP:      ; Analog Comparator Handler
       reti

ADC:           ; ADC Conversion Handler
       reti

TIM1_COMPB:    ; Timer1 CompareB Handler
       reti

TIM0_COMPA:    ;
       reti

TIM0_COMPB:    ;
       reti

WDT:           ;
       reti

USI_START:     ;
       reti

USI_OVF:       ;
       reti
