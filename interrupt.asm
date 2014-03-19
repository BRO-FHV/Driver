;
; Driver: interrupt.asm
; Part of BRO Project, 2014 <<https://github.com/BRO-FHV>>
;
; Created on: Mar 19, 2014
; Description:
; Assembler interrupt handler
;
; The ARM state register set:
; 	http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0211i/ch02s08s01.html
;
; shared header files
;
    .cdecls C,LIST,"hw_interrupt.h"
    .cdecls C,LIST,"soc_AM335x.h"

;
; global regiter definitions
;
ADDR_THRESHOLD		.word	SOC_AINTC_REGS + INTC_THRESHOLD
ADDR_IRQ_PRIORITY	.word	SOC_AINTC_REGS + INTC_IRQ_PRIORITY
ADDR_SIR_IRQ		.word	SOC_AINTC_REGS + INTC_SIR_IRQ
ADDR_CONTROL		.word	SOC_AINTC_REGS + INTC_CONTROL

;
; ???
;
MASK_ACTIVE_IRQ		.set	INTC_SIR_IRQ_ACTIVEIRQ
NEWIRQAGR			.set	INTC_CONTROL_NEWIRQAGR

;
; define section of memory
;
	.text

;
; source file is assembled for ARM instructions
;
	.state32

;
; define global symbols (share between c and asm)
;
    .global IntIRQHandler
	.ref intIrqHandlers

;
; definition of irq handlers
;
_intIrqHandlers:
    .word intIrqHandlers

;
; IRQ handler function definition
;
;	+ SUB	= 	Subtract with or without Carry
;			-> 	http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0489c/CIHGJHHH.html
;
;	+ STMFD	=	The load and store multiple instructions can update the base register
;			->	http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0068b/Cacbgchh.html
;
;	+ MRS	= 	Move to ARM register from system coprocessor register
;			-> 	http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0489c/CIHGJHHH.html
;
;	+ LDR	=	The load instruction can be used to move single data item between register and memory
;			->	http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0041c/Babbfdih.html
;			->	http://www.bravegnu.org/gnu-eprog/arm-iset.html
;
;	+ STR	=	The store instruction can be used to move single data item between register and memory
;			->	http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0068b/BABDJCHA.html
;			->	http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0068b/BABDJCHA.html
;
;	+ AND	=	???
;
;	+ MOV	=	instruction copies the value of operand2 into Rd (operand1)
;			->	http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0204j/Cihcdbca.html
;
IntIRQHandler:
	;
	; backup user context
	;	+ R13 	= stackpointer
	;	+ R14 	= link register
	;	+ SPSR	= saved program status register
	; 	+ LR	= link register
	;
	SUB      r14, r14, #4				; lr correction ???
	STMFD    r13!, {r0-r3, r12, r14}	; backup user context in irq stack
	MRS      r12, spsr					; copy SPSR
	STMFD    r13!, {r12}				; backup SPSR in irq stack

	;
	; backup IRQ threshold value
	;	+ threshold 	= ???
	;	+ irq threshold	= ???
	;
	LDR      r0, ADDR_THRESHOLD			; store IRQ threshold address in r0
	LDR      r1, [r0, #0]				; load value from ram (address in r0 + offset 0)
	STMFD    r13!, {r1}					; backup the threshold value in irq stack

	;
	; set IRQ priority as threshold value
	;	+ irq priority	= ???
	;
	LDR      r2, ADDR_IRQ_PRIORITY   	; store IRQ priority address in r2
	LDR      r3, [r2, #0]				; load value from ram (address in r2 + offset 0)
	STR      r3, [r0, #0]				; set the priority as threshold

	;
	; read active IRQ number
	;
	LDR      r1, ADDR_SIR_IRQ			; store IRQ status registe in r1
	LDR      r2, [r1, #0]				; load value from ram (address in r1 + offset 0)
	AND      r2, r2, #MASK_ACTIVE_IRQ	; mask active IRQ number

	;
	; enable IRQ generation
	;
	MOV      r0, #NEWIRQAGR				; load mask for new IRQ generation in r0
	LDR      r1, ADDR_CONTROL			; load address for interrupt control register in r1


	; ---------------------------------------------------------------------------------------
	CMP      r3, #0                   ; Check if non-maskable priority 0
	STRNE    r0, [r1]                 ; if > 0 priority, acknowledge INTC
	DSB                               ; Make sure acknowledgement is completed

	;
	; Enable IRQ and switch to system mode. But IRQ shall be enabled
	; only if priority level is > 0. Note that priority 0 is non maskable.
	; Interrupt Service Routines will execute in System Mode.
	;
	MRS      r14, cpsr                ; Read cpsr
	ORR      r14, r14, #MODE_SYS
	BICNE    r14, r14, #I_BIT         ; Enable IRQ if priority > 0
	MSR      cpsr_cxsf, r14

	STMFD    r13!, {r14}              ; Save lr_usr
	LDR      r0, _fnRAMVectors        ; Load the base of the vector table
	ADD      r14, pc, #0              ; Save return address in LR
	LDR      pc, [r0, r2, lsl #2]     ; Jump to the ISR

	LDMFD    r13!, {r14}              ; Restore lr_usr
	;
	; Disable IRQ and change back to IRQ mode
	;
	CPSID    i, #MODE_IRQ

	LDR      r0, ADDR_THRESHOLD      ; Get the IRQ Threshold
	LDR      r1, [r0, #0]
	CMP      r1, #0                   ; If priority 0
	MOVEQ    r2, #NEWIRQAGR           ; Enable new IRQ Generation
	LDREQ    r1, ADDR_CONTROL
	STREQ    r2, [r1]
	LDMFD    r13!, {r1}
	STR      r1, [r0, #0]             ; Restore the threshold value
	VLDMIA   r13!, {d0-d7}            ; Restore D0-D7 Neon/VFP registers
	LDMFD    r13!, {r1, r12}          ; Get fpscr and spsr
	MSR      spsr_cxsf, r12           ; Restore spsr
	VMSR     fpscr, r1                ; Restore fpscr
	LDMFD    r13!, {r0-r3, r12, pc}^  ; Restore the context and return
