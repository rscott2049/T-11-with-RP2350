//; written by Folkert van Heusden <folkert@komputilo.nl>
//; (C) 2026, MIT license
//; GCC AS version  
	.text
// T-11 starts at 0x0
	
	. = 0	
	// Here when reset or powerup
pup:
	jmp init

       // Here when halt instruction executed
        . = 4
restart:
	jmp halted
//	br restart

	// Illegal instruction trap
	. = 010
	.word $ill_isr
	.word 0300

        . = 01000
init:
        mov $init, sp
        jmp start

run_counter:
        .word 0
run_counter_mw:
        .word 0
run_counter_hw:
        .word 0

kw11l_counter:
        .word 0
kw11l_temp:
        .word 0
        .word 0

        .word 0
        .word 0
        .word 0
        .word 0
        .word 0
scratch:
        .word 0
        .word 0
        .word 0
        .word 0
        .word 0
scratch2:
        .word 0
        .word 0
        .word 0
        .word 0
        .word 0

	.align 1, 0
hex_chars: .ascii "0123456789abcdef"
	.align 1, 0
value_buffer: .asciz "________________________"
	.align 1, 0
text_hello: .asciz "Benchmark starting...\r\n"
	.align 1, 0
text_go: .asciz "Go!\r\n"
	.align 1, 0
text_finished: .asciz "Finished: "
	.align 1, 0
text_crlf: .asciz "\r\n"
	.align 1, 0
text_ill: .asciz "Illegal instruction\r\n"
	.align 1, 0
text_halted: .asciz "Halt instruction\r\n"
	// Pad to next word
	.align 1, 0

trap_isr:
        rti

kw11l_isr:
	mov	r0, kw11l_temp
        mov	R1, kw11l_temp+2
//        movb    $0x41, 0177566       // Send character through 'TBP'

        MOV     $0177546, R0      // KW11-L CSR
        MOV     (R0), R1          // read CSR
        BIC     $0200, R1         // clear DONE bit (bit 7)
        MOV     R1, (R0)          // write back to acknowledge
        mov 	kw11l_temp,R0
        mov 	kw11l_temp+2,R1

        inc     kw11l_counter
        rti

start:
        mov $text_hello, R2
        JSR PC,PRINT

        // setup KW11-L ISR
        MOV     $kw11l_isr, 0100      //  ISR address
        MOV     $0300,      0102      //  PSW (IPL 6)
        mov     $0177546, R0
        MOV     $0101, (R0)            // enable KW11-L interrupts

        // setup TRAP vector
        MOV     $trap_isr, 034
        MOV     $0300,     036
        // setup CPU errors vector
        MOV     $trap_isr, 004
        MOV     $0300,     006
        // setup illegal & reserved instructions vector
        //MOV     $trap_isr, 010
	MOV     $ill_isr, 010
        MOV     $0300,     012

//        spl 5
        mtps $0240   // Pri 5

        mov $text_go, R2
        JSR PC,PRINT

loop_:
        // statistics
        add $1, run_counter
        adc run_counter_mw
        adc run_counter_hw

        // finished?
        cmp $0764, kw11l_counter   // 50 Hz, 10 seconds
        ble  print_results

        // access to registers
        mov $0123, R0
        add $0123, R0
        sub $0123, R0
        mov R0, R1

        // SP
        mov R6, R1

//        SPL 7
        mtps $0340   // Pri 7
        mov $0224, R6
        add $0124, R6
        sub $0124, R6
        mov R1,R6
//        SPL 5
        mtps $0240   // Pri 5

        // PC
        ADD $2, R7
        NOP

        // shift + div
        mov $0123456, R0
//        ASH $-2,R0
        mov $01324, R1
//        ASH $01,R1
        mov $037777, R2
//        div r2,r0

        // mul
        mov $07,R0
        mov $017,R1
//        MUL R2,R0

        // compare
        CMP R0,R1
        bne bne_hit
        beq beq_hit
bne_hit:
beq_hit:

        // addressing modes
        mov $20, R5
addr_move_loop:
        //mov $scratch2,($scratch)

        MOV R0, R1     // 0
        MOV $scratch,R1
        MOV R0, (R1)   // 1
        MOV R0, (R1)+  // 2
        mov R0, @$scratch// 3
        MOV R0, -(R1)  // 4
        MOV R0, -(R1)  // 4
        //MOV R0, @-(R0) // 5  <-- triggers a halt in simh
        MOV R0, 2(R1)  // 6
        MOV R0, scratch// 7

        DEC R5
        BNE addr_move_loop


// misc
        clr  R0
        clrb R0
        dec  R0
        decb R0
        inc  R0
        incb R0
        neg  R0
        negb R0
        tst  R0
        tstb R0
        com  R0
        comb R0
        asr  R0
        asrb R0
        asl  R0
        aslb R0
//        ash  $2,R0
//        ash $-15,R0
//        ashc R2,R0
        adc  R0
        adcb R0
        sbc  R0
        sxt  R0
        rol  R0
        rolb R0
        ror  R0
        rorb R0
        swab R0
        add  $1234,R0
        sub  $1234,R0
        xor  R0,R2
        bis  $123,R0
        bisb $123,R0
        bic  $123,R0
        bicb $123,R0
        bit  $123,R0
        bitb $123,R0
        br   next_instr
next_instr:
        beq  next_instr2
next_instr2:
        jsr  pc,my_routine
        jmp  after_my_routine
my_routine:
        rts pc
after_my_routine:
        TRAP 0
        jmp loop_

print_results:                     // print results
//        spl 7             // stop increasing counter
        mtps $0340   // Pri 7
        mov $value_buffer, R2
        mov $run_counter_hw, R1
        jsr pc,make_hex
        mov $run_counter_mw, R1
        jsr pc,make_hex
        mov $run_counter, R1
        jsr pc,make_hex
        movb $0,(R2)+

        mov $text_finished, R2
        JSR PC,PRINT
        mov $value_buffer, R2
        JSR PC,PRINT
        mov $text_crlf, R2
        JSR PC,PRINT
        mov $text_crlf, R2
        JSR PC,PRINT

//	nop
	halt
finished:
        br finished

make_hex: 

        movb 1(R1), R4     // get first byte to process
//        ash $-4,R4        // high nibble
	asr r4
	asr r4
	asr r4
	asr r4
        BIC $0177760,R4
        add $hex_chars, R4
        movb (R4),(R2)+
        movb 1(R1), R4     // re-get byte to process
        BIC $0177760,R4        // low nibble
        add $hex_chars, R4
        movb (R4),(R2)+
        movb (R1), R4     // get second byte to process
//        ash $-4,R4        // high nibble
	asr R4
	asr R4
	asr R4
	asr R4
        BIC $0177760,R4
        add $hex_chars, R4
        movb (R4),(R2)+
        movb (R1), R4     // re-get byte to process
        BIC $0177760,R4        // low nibble
        add $hex_chars, R4
        movb (R4),(R2)+
        RTS PC

PRINT:  movb    (r2), r0
        beq     done_print

WAITTPS:   tstb 0177564       // Check TPS
        bpl     WAITTPS
        movb    r0, 0177566       // Send character through 'TBP'
        inc     r2
        br      PRINT
done_print:
        RTS PC

ill_isr:
	mov $1000, sp
        mov $text_ill,R2
        JSR PC,PRINT
ill_loop:
	br ill_loop

halted:
//	mov $1000, sp
//	mov $text_halted, R2
//	JSR PC,PRINT
h_loop:
	br h_loop
