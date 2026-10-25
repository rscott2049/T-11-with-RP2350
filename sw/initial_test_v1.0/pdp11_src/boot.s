    .TITLE Handle HALT on T11
        .IDENT "V00.00"

        .GLOBAL start
	
        kbs = 0177560
        txs = 0177564

	//  Make sure we've set the T11 mode register to start here on powerup
	// Note: we use the linker to place code at the T11 start location
        .text
	. = 0
start:
	jmp *$0	    		// Start boot block on powerup
	
restart:    			// Drop into halt handler on restart
	mov r0, save_r0         // Save scratch register

	mov (sp), r0		// Get next PC 
	tst -2(r0)		// See prev addr pointed to a halt (all zeros)
				// (Though it could be immediate data, etc!)
	beq hlt_ins		// Yup, go handle it

halt_pin:
	mov save_r0, r0         // Restore R0
	mov *$4, pc    	        // Jmp indirect via NXM trap address

hlt_ins:
	mov $hlt_msg, r0        // Get start of halt message
	jsr pc, prt_str
	mov (sp), r0            // Save next PC for display
	mov $0165144, r1        // Return address for ptroct
	jmp *$0165430

prt_str:
	mov $txs, r1      	// Get TX CSR address (keeping code PIC)
p_loop:	movb (r0)+, 2(r1)       // Output char
hlt_t:  tstb (r1)	
        bpl hlt_t
        tstb (r0)
	bne p_loop
	rts pc

save_r0:
	.word 0
hlt_msg: .asciz "HALTED. Next PC: "
	.end
