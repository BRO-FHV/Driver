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
	.cdecls C,LIST,"interrupt/hw_interrupt.h"
	.cdecls C,LIST,"soc_AM335x.h"

;
; global regiter definitions
;
ADDR_THRESHOLD		.word	SOC_AINTC_REGS + INTC_THRESHOLD
ADDR_IRQ_PRIORITY	.word	SOC_AINTC_REGS + INTC_IRQ_PRIORITY
ADDR_SIR_IRQ		.word	SOC_AINTC_REGS + INTC_SIR_IRQ
ADDR_CONTROL		.word	SOC_AINTC_REGS + INTC_CONTROL

;
; bit masks
;
MASK_ACTIVE_IRQ		.set	INTC_SIR_IRQ_ACTIVEIRQ
MASK_NEW_IRQ		.set	INTC_CONTROL_NEWIRQAGR
MASK_SYS_MODE		.set	0x1F
MASK_IRQ_MODE		.set	0x12
MASK_I_BIT			.set	0x80
MASK_SWI_NUM		.set	0xFF000000

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
	.global irq_handler
	.global swi_handler
	.ref intIrqHandlers
	.ref intIrqResetHandlers

;
; definition of irq handlers
;
_intIrqHandlers:
	.word intIrqHandlers

;
; definition of irq reset handlers
;
_intIrqResetHandlers:
	.word intIrqResetHandlers

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
;	+ AND	=	Bitwise AND logical operations
;			->	http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0068b/BABGIEBE.html
;
;
;	+ MOV	=	Instruction copies the value of operand2 into Rd (operand1)
;			->	http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0204j/Cihcdbca.html
;
;	+ CMP	=	Compare operation
;			->	http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0068b/CIHIDDID.html
;
;	+ STRNE	=	???
;			->	???
;
;	+ DSB	=	Data Synchronization Barrier acts as a special kind of memory barrier. No instruction in program order after this instruction executes until this instruction completes.
;			->	http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0489c/CIHGHHIE.html
;
;	+ ORR	=	Bitwise OR logical operations
;			->	http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0068b/BABGIEBE.html
;
;	+ BICNE	=	???
;			->	???
;
;	+ MOVEQ	=	Move if equal
;
;	+ LDMFD	=
;
irq_handler:
	;
	; backup user context
	;	+ R13 	= stackpointer
	;	+ R14 	= link register
	;	+ SPSR	= saved program status register
	; 	+ LR	= link register
	;
	SUB		r14, r14, #4				; lr correction ???
	STMFD	r13!, {r0-r3, r12, r14}		; backup user context in irq stack
	MRS		r12, spsr					; copy SPSR
	STMFD	r13!, {r12}					; backup SPSR in irq stack

	;
	; backup IRQ threshold value
	;	+ threshold 	= ???
	;	+ irq threshold	= ???
	;
	;LDR		r0, ADDR_THRESHOLD			; store IRQ threshold address in r0
	;LDR		r1, [r0, #0]				; load value from ram (address in r0 + offset 0)
	;STMFD		r13!, {r1}					; backup the threshold value in irq stack

	;
	; set IRQ priority as threshold value
	;	+ irq priority	= ???
	;
	;LDR		r2, ADDR_IRQ_PRIORITY   	; store IRQ priority address in r2
	;LDR		r3, [r2, #0]				; load value from ram (address in r2 + offset 0)
	;STR		r3, [r0, #0]				; set the priority as threshold

	;
	; read active IRQ number
	;
	LDR		r1, ADDR_SIR_IRQ			; store IRQ status registe in r1
	LDR		r2, [r1, #0]				; load value from ram (address in r1 + offset 0)
	AND		r2, r2, #MASK_ACTIVE_IRQ	; mask active IRQ number

	;
	; check priotity not equal zero
	; ???
	;
	;CMP		r3, #0						; check priotity is zero
	;STRNE	r0, [r1]					; if priority > 0 , acknowledge INTC
	;DSB									; wait for acknowledge

	;
	; switch to system mode and
	; enable other IRQ if priority allows it
	;
	;MRS		r14, cpsr					; read cpsr in r14
	;ORR		r14, r14, #MASK_SYS_MODE	; mask system mode
	;BICNE	r14, r14, #MASK_I_BIT		; enable IRQ if priority > 0
	;MSR		cpsr_cxsf, r14				; store cpsr back
	;DSB									; wait for acknowledge to ensure system mode

	;
	; start interrupt handler
	;	+ r2	= contains active IRQ number
	;	+ r14	= link register
	;	+ pc	= program counter
	;
	STMFD	r13!, {r14}					; backup user link register
	LDR		r0, _intIrqHandlers			; load base of interrupt handler (implemented in interrupt.c)
	ADD		r14, pc, #0					; save return address in link register (return point)
	LDR		pc, [r0, r2, lsl #2]		; jump to interrupt handler

	;
	; read active IRQ number
	;
	LDR		r1, ADDR_SIR_IRQ			; store IRQ status registe in r1
	LDR		r2, [r1, #0]				; load value from ram (address in r1 + offset 0)
	AND		r2, r2, #MASK_ACTIVE_IRQ	; mask active IRQ number

	;
	; reset interrupt flags
	;
	LDR		r0, _intIrqResetHandlers	; load base of interrupt handler (implemented in interrupt.c)
	ADD		r14, pc, #0					; save return address in link register (return point)
	LDR		pc, [r0, r2, lsl #2]		; jump to interrupt handler

	;
	; enable IRQ generation
	;
	MOV		r0, #MASK_NEW_IRQ			; load mask for new IRQ generation in r0
	LDR		r1, ADDR_CONTROL			; load address for interrupt control register in r1
	STR		r0, [r1, #0]				; store content of r2 in RAM address in r1 + offset 0

	;
	; restore user link register
	;
	LDMFD	r13!, {r14}					; Restore lr_usr

	;
	; disable IRQ and change back to IRQ mode
	;
	;CPSID	i, #MASK_IRQ_MODE

	;
	; restore backuped values
	;
	;LDR		r0, ADDR_THRESHOLD			; load IRQ threshold
	;LDR		r1, [r0, #0]				; load threshold register from RAM
	;LDMFD	r13!, {r1}					; ???
	;STR		r1, [r0, #0]				; restore the threshold value
	LDMFD	r13!, {r12}					; get spsr
	MSR		spsr_cxsf, r12				; restore spsr
	LDMFD	r13!, {r0-r3, r12, pc}^		; restore the context and return

;
; SWI handler function definition
;	+ BL	=	The BL and BLX instructions copy the address of the next instruction into lr (r14, the link register).
;			->	http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0204j/Cihfddaf.html
;
swi_handler:
	STMFD	r13!, {r0-r1, r14}			; save current user context in stack
	SUB		r13, r13, #0x4				; adjust the stack pointer
	LDR		r0, [r14, #-4]				; R0 points to SWI instruction
	BIC		r0, r0, #MASK_SWI_NUM		; get the SWI number (r0)
	ADD		r13, r13, #0x4		 		; Adjust the stack pointer
	LDMFD	r13!, {r0-r1, pc}^	 		; restore the context and return
