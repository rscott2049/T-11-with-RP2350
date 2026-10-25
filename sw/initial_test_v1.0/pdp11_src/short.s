    .TITLE Short test program
        .IDENT "V00.00"

        .GLOBAL start
	
	TEST_LOC = 0x1000
	STACK = 0x2000
        kbs = 0177560
        txs = 0177564
	ltc = 0177546
	rkds = 0177400
	rkcs = 0177404
	rkwc = 0177406
	rkba = 0177410
	rkda = 0177412

	rlcs = 0174400
	rlba = 0174402
	rlda = 0174404
	rlmp = 0174406

        .text
	. = 0
	// Here when reset or powerup
pup:
	jmp start


       // Here when halt instruction executed
        . = 4
	.word nxm_hand
	.word 0340

	. = 060
rx_irq_vec:
	.word rx_irq_handler
	.word 0340

	. = 064
tx_irq_vec:
	.word tx_irq_handler
	.word 0340

	. = 070
irq_4_vec:
	.word irq_handler
	.word 0340

irq_100_vec:
	. = 0100 // line time clock
	.word irq_handler
	.word 0342

irq_cp3_vec:
	. = 0114
	.word irq_handler
	.word 0340

irq_cp2_vec:
	. = 0134
	.word irq_handler
	.word 0340


irq_200_vec:
	. = 0200 
	.word irq_handler_4
	.word 0340

irq_300_vec:
	. = 0300 
	.word irq_handler
	.word 0341

	. = 0360
irq_inc:
	.word 0

inc_loc:	
	.word   0x55	

	// Program
	. = 0400
start:	
	mov $0x2000, r6
//	jmp ltc_t
//	jmp irq_t
//	jmp nxm_t
//	jmp nxm_t2
//	jmp tl_t       // RK05 tight loop test
	jmp rl_t       // RL02 test

// Default test
	mov $s_msg, r0
	jsr r1, sj_out
	mov $0x4000, r0
d_loop: incb (r0)
	br d_loop

//one:	
//	mov $0xffff, r1
//	mov $kbs, r0
//	mov r1, (r0)

//	tstb (r0) 
//one_w:	br one_w


//dl_test:
//	mov $kbs, r0
//wait:	tstb (r0)           	// Char available?
//	bpl wait		// No, wait
	
//t_wait:	tstb 4(r0)               // Transmitter ready?
//	bpl t_wait		// No, wait
//	mov 2(r0), 6(r0)	// Yes, transmit
//	br wait			// And get next char

dl_irq_test:
	mov	$kbs, r0        // RX status reg
//	bis 	$0x40, (r0)     // Enable RX interrupts
	bis 	$0x40, 4(r0)    // Enable TX interrupts

	mov	$14, r4		// Reset TX transmission count
        mtps 	$0140	        // Pri 3 (DL11 is at pri 4)
//	movb	$0x41, 6(r0)    // Start transmit process
tx_msg_done:
	cmp	$0, r4	
	bne	tx_msg_done
//	bic 	$0x40, 4(r0)    // Disable TX interrupts
	bis 	$0x40, (r0)     // Enable RX interrupts
	
//	br tx_msg_done
//	wait
//	cmp	$(msg_ptr + 13), msg_ptr
//	bne	tx_msg_done
//	bic 	$0x40, 4(r0)    // Disable TX interrupts

rx_wait:
	wait			// Wait for char
	movb 2(r0), r5          // Get it
	cmpb $0x03, r5            // Exit loop on cntrl c
	beq rx_exit
	movb r5, 6(r0)		// Otherwise, echo it
	br rx_wait

rx_exit:
	halt

//	mov $STACK, r6
//	mov $TEST_LOC, r0
//	clr r1
//	mov $0x8000, r1
//	com r1

//loop:	com (r0)
//loop:	inc (r0)
//loop:	mov r1, (r0)
	//halt
//loop:	mov $0xc000, (r0)
//	reset
//	halt
//	clr r0
//	mov $0xff00, r1
//	mov $0xFF74, r1

//	mov $0177564, r1
//	mov (r1), r2
//	inc (r1)+ 
//	inc (r1)+

//	dec r6
//	mov $0x1021,r2
//	movb r6,(r2)+
//	movb r1,(r2)+
//	movb r1,(r2)+
	nop
//	mov r1, (r1)
//	mov  (r1), r2
//        mtps $0340   // Pri 7
//        mtps $0300   // Pri 6
//        mtps $0240   // Pri 5
//	spl 0  // No SPL!
//loop:	nop
//	mov $inc_loc, r1
//loop:	inc (r1)
//loop:	inc r0
//	br loop

// IRQ test	
irq_t:
//	mtps $0      // Pri 1
	mtps $0240    // Pri 5
//	mtps $0340    // Pri 7
i_loop:	br i_loop    // spin

// LTC IRQ test	
ltc_t:
	mov $0x40, *$0177546  // enable ltc interrupts on vector 0100
	mtps $0240            // Pri 5, ltc is pri 6
l_loop:	br l_loop             // spin

WAITTPS:   tstb    $0177564       // Check TPS
//WAITTPS:   tstb    $0177564       // Check TPS
//WAITTPS:   mov    0177564,r1       // Check TPS
//        bpl     WAITTPS
done_wait:	
	br done_wait

rx_irq_handler:
	inc $irq_inc
	rti

s_msg:
	.asciz "Short test\r\n"

nxm_msg:
	.asciz "NXM handler\r\n"

msg:	.asciz "TX IRQ handler\r\n"
	.align 1, 0
msg_ptr:
	.word msg

tx_irq_handler:
	mov	msg_ptr, r1
print_tx:
	movb	(r1), r2
	beq	tx_irq_done
wait_tx:
//	tstb	0177564       // Check TPS
//        bpl	wait_tx
        movb    r2, 0177566       // Send character through 'TBP'
//        inc     r1
	inc	msg_ptr
	rti

//	mov msg_ptr, r1
//	beq tx_irq_done
//	movb (r1)+, txs
//	mov r1, msg_ptr
tx_irq_done:	
        bic    	$0x40, 0177564      // Clear TX interrupt enable when done
	clr	r4
	rti


irq_handler:
	inc r5
	rti

irq_handler_4:
	inc r4
	rti

handler:
	inc r2
	rti
//	br handler
//	reset
//	mov $STACK, r6
//h_loop:	nop
//	br h_loop

	
	// NXM test


nxm_t:
//	mov $nxm_hand, $*4  	// Setup handler
//	mov $0340, $*6

	mov $sj_msg, r0
	jsr r1, sj_out

	mov $0177700, r3
	mov $40, r4
nxm_l:	mov (r3)+, r5

//nxm_l:	clr (r3)+
	reset
//	br nxm_l
	sob r4, nxm_l

	mov $done_msg, r0
	jsr r1, sj_out
nxm_s:	br nxm_s

nxm_t2:
//	mov $0x4000, r0
	mov $0172000, r0
nxm_t2l:
	incb (r0)
	br nxm_t2l


nxm_hand:
//	halt
	mov $nxm_msg, r0
	jsr r1, sj_out
//	halt
	rti

// From SJ bootloader:
	. = 726
sj_out:	
//	MOVB (R0)+,@#0177566
//sj_txp:	TSTB @#0177564
	MOVB (R0)+, 0177566
sj_txp:	TSTB 0177564
	BPL sj_txp
	TSTB (R0)
	BNE sj_out
	RTS R1

	MOVB (R0)+,@#0177566
	TSTB @#0177564

sj_msg:	.asciz "?BOOT-U-Monitor file not found on volume\r\n"
done_msg:
	.asciz "test done\r\n"
	.align 1, 0	

	mov $sj_msg, r0
	jsr r1, sj_out
	mov $sj_msg, r0
	jsr r1, sj_out
	mov $sj_msg, r0
	jsr r1, sj_out
	mov $sj_msg, r0
	jsr r1, sj_out

sj_d:	br sj_d
//sj_d:	halt
	nop

	// RK05 tight loop test
	// RKCS:   000004
	// RKDA:   000002
	// RKBA:   001000
	// RKWC:   176000

tl_t:
	mov $0000002, *$rkda   // RKDA:   000002
	mov $0001000, *$rkba   // RKBA:   001000
	mov $0176000, *$rkwc   // RKWC:   176000
	mov $0177404, r5       // RKCS:  Command status reg
	clr r0
	mov $0x85, (r5)        // Send read command, with go bit set
tl_b:	mov (r5), r1
	mov r1, *$inc_loc      // Save status
	halt		       // Done
	

	inc r0      	   // bump count
	tstb r1	   	   // Wait until read
	bpl tl_b           // Test CSR
tl_d:	mov r0, *$inc_loc  // Save inc loc
	halt		   // Done



	SIMH_RL_BOOT_START =  02000
	SIMH_RL_BOOT_ENTRY = (SIMH_RL_BOOT_START + 002)
	SIMH_RL_BOOT_UNIT =  (SIMH_RL_BOOT_START + 010) 
	SIMH_RL_BOOT_CSR  =  (SIMH_RL_BOOT_START + 020)
	. = 02002
rl_t:	.word 0012706, SIMH_RL_BOOT_START    /* MOV #boot_start, SP */
	.word 0012700, 0000000	             /* MOV #unit, R0 */
	.word 0010003                        /* MOV R0, R3 */
	.word 0000303                        /* SWAB R3 */
	.word 0012701, 0174400               /* MOV #RLCS, R1        ; csr */
	.word 0012761, 0000013, 0000004      /* MOV #13, 4(R1)       ; clr err */
	.word 0052703, 0000004               /* BIS #4, R3           ; unit+gstat */
	.word 0010311                        /* MOV R3, (R1)         ; issue cmd */
	.word 0105711                        /* TSTB (R1)            ; wait */
	.word 0100376                        /* BPL .-2 */

	.word 0016102, 0000006               /* MOV 6(R1), R2        ; get hdr */
	.word 0042702, 0000077               /* BIC #77, R2          ; clr sector */
	.word 0005202                        /* INC R2               ; magic bit */
	.word 0010261, 0000004               /* MOV R2, 4(R1)        ; seek to 0 */
	.word 0105003                        /* CLRB R3 */
	.word 0052703, 0000006               /* BIS #6, R3           ; unit+seek */
	.word 0010311                        /* MOV R3, (R1)         ; issue cmd */
	.word 0105711                        /* TSTB (R1)            ; wait */
	.word 0100376                        /* BPL .-2 */
	.word 0005061, 0000002               /* CLR 2(R1)            ; clr ba */
	.word 0005061, 0000004               /* CLR 4(R1)            ; clr da */
	.word 0012761, 0177000, 0000006      /* MOV #-512., 6(R1)    ; set wc */
	.word 0105003                        /* CLRB R3 */
	.word 0052703, 0000014               /* BIS #14, R3          ; unit+read */
	.word 0010311                        /* MOV R3, (R1)         ; issue cmd */
	.word 0105711                        /* TSTB (R1)            ; wait */
	.word 0100376                        /* BPL .-2 */
	.word 0042711, 0000377               /* BIC #377, (R1) */
	.word 0005002                        /* CLR R2 */
	.word 0005003                        /* CLR R3 */
	.word 0012704, SIMH_RL_BOOT_START+020        /* MOV #START+20, R4 */
	.word 0005005                        /* CLR R5 */
	.word 0005007                        /* CLR PC */

	halt

	mov $rlcs, r0
	mov (r0), r1
	mov $010, (r0)
rl_s:	halt
