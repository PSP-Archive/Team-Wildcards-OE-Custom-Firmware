	.file	1 "main.c"
	.section .mdebug.eabi32
	.section .gcc_compiled_long32
	.previous
	.text
	.align	2
	.globl	Reboot_Entry
	.ent	Reboot_Entry
Reboot_Entry:
	.frame	$fp,24,$31		# vars= 16, regs= 2/0, args= 0, gp= 0
	.mask	0xc0000000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-24
	sw	$31,20($sp)
	sw	$fp,16($sp)
	move	$fp,$sp
	sw	$4,0($fp)
	sw	$5,4($fp)
	sw	$6,8($fp)
	sw	$7,12($fp)
	lw	$4,0($fp)
	lw	$5,4($fp)
	lw	$6,8($fp)
	lw	$7,12($fp)
	jal	Main
	nop

	move	$sp,$fp
	lw	$31,20($sp)
	lw	$fp,16($sp)
	addiu	$sp,$sp,24
	j	$31
	nop

	.set	macro
	.set	reorder
	.end	Reboot_Entry
	.size	Reboot_Entry, .-Reboot_Entry
	.globl	Real_Reboot
	.data
	.align	2
	.type	Real_Reboot, @object
	.size	Real_Reboot, 4
Real_Reboot:
	.word	-2006974464
	.globl	sceBootLfatOpen
	.align	2
	.type	sceBootLfatOpen, @object
	.size	sceBootLfatOpen, 4
sceBootLfatOpen:
	.word	-2006942036
	.globl	DcacheClear340
	.align	2
	.type	DcacheClear340, @object
	.size	DcacheClear340, 4
DcacheClear340:
	.word	-2006964288
	.globl	IcacheClear340
	.align	2
	.type	IcacheClear340, @object
	.size	IcacheClear340, 4
IcacheClear340:
	.word	-2006965876
	.globl	DcacheClear150
	.align	2
	.type	DcacheClear150, @object
	.size	DcacheClear150, 4
DcacheClear150:
	.word	-2000671644
	.globl	IcacheClear150
	.align	2
	.type	IcacheClear150, @object
	.size	IcacheClear150, 4
IcacheClear150:
	.word	-2000671600
	.rdata
	.align	2
	.type	key_data_340, @object
	.size	key_data_340, 16
key_data_340:
	.byte	-82
	.byte	14
	.byte	-113
	.byte	-61
	.byte	-76
	.byte	21
	.byte	-127
	.byte	120
	.byte	27
	.byte	30
	.byte	-99
	.byte	-41
	.byte	111
	.byte	-71
	.byte	73
	.byte	-81
	.text
	.align	2
	.ent	buld_key_param
buld_key_param:
	.frame	$fp,32,$31		# vars= 24, regs= 1/0, args= 0, gp= 0
	.mask	0x40000000,-8
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-32
	sw	$fp,24($sp)
	move	$fp,$sp
	sw	$4,16($fp)
	li	$2,-1969487872			# 0xffffffff8a9c0000
	ori	$2,$2,0xedae
	sw	$2,4($fp)
	li	$2,1396572160			# 0x533e0000
	ori	$2,$2,0xa2e
	sw	$2,0($fp)
	sw	$0,8($fp)
	j	$L4
	nop

$L5:
	lw	$3,8($fp)
	lw	$2,4($fp)
	addu	$3,$3,$2
	li	$2,891486208			# 0x35230000
	ori	$2,$2,0x1452
	addu	$5,$3,$2
	lw	$3,8($fp)
	lw	$2,0($fp)
	addu	$3,$3,$2
	li	$2,891486208			# 0x35230000
	ori	$2,$2,0x1452
	addu	$2,$3,$2
	lbu	$4,0($2)
	lw	$2,8($fp)
	move	$3,$2
	lw	$2,16($fp)
	addu	$2,$3,$2
	lbu	$2,0($2)
	xor	$2,$4,$2
	andi	$3,$2,0x00ff
	li	$2,-107			# 0xffffffffffffff95
	xor	$2,$3,$2
	andi	$2,$2,0x00ff
	sb	$2,0($5)
	lw	$2,8($fp)
	addiu	$2,$2,1
	sw	$2,8($fp)
$L4:
	lw	$2,8($fp)
	slt	$2,$2,16
	bne	$2,$0,$L5
	nop

	move	$sp,$fp
	lw	$fp,24($sp)
	addiu	$sp,$sp,32
	j	$31
	nop

	.set	macro
	.set	reorder
	.end	buld_key_param
	.size	buld_key_param, .-buld_key_param
	.align	2
	.globl	memcmp
	.ent	memcmp
memcmp:
	.frame	$fp,32,$31		# vars= 24, regs= 1/0, args= 0, gp= 0
	.mask	0x40000000,-8
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-32
	sw	$fp,24($sp)
	move	$fp,$sp
	sw	$4,8($fp)
	sw	$5,12($fp)
	sw	$6,16($fp)
	sw	$0,0($fp)
	j	$L9
	nop

$L10:
	lw	$2,0($fp)
	move	$3,$2
	lw	$2,8($fp)
	addu	$2,$3,$2
	lbu	$4,0($2)
	lw	$2,0($fp)
	move	$3,$2
	lw	$2,12($fp)
	addu	$2,$3,$2
	lbu	$2,0($2)
	beq	$4,$2,$L11
	nop

	lw	$2,0($fp)
	move	$3,$2
	lw	$2,12($fp)
	addu	$2,$3,$2
	lbu	$2,0($2)
	move	$4,$2
	lw	$2,0($fp)
	move	$3,$2
	lw	$2,8($fp)
	addu	$2,$3,$2
	lbu	$2,0($2)
	subu	$4,$4,$2
	sw	$4,20($fp)
	j	$L13
	nop

$L11:
	lw	$2,0($fp)
	addiu	$2,$2,1
	sw	$2,0($fp)
$L9:
	lw	$2,0($fp)
	lw	$3,16($fp)
	slt	$2,$2,$3
	bne	$2,$0,$L10
	nop

	sw	$0,20($fp)
$L13:
	lw	$2,20($fp)
	move	$sp,$fp
	lw	$fp,24($sp)
	addiu	$sp,$sp,32
	j	$31
	nop

	.set	macro
	.set	reorder
	.end	memcmp
	.size	memcmp, .-memcmp
	.align	2
	.globl	ClearCaches340
	.ent	ClearCaches340
ClearCaches340:
	.frame	$fp,8,$31		# vars= 0, regs= 2/0, args= 0, gp= 0
	.mask	0xc0000000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-8
	sw	$31,4($sp)
	sw	$fp,0($sp)
	move	$fp,$sp
	lui	$2,%hi(DcacheClear340)
	lw	$2,%lo(DcacheClear340)($2)
	jal	$2
	nop

	lui	$2,%hi(IcacheClear340)
	lw	$2,%lo(IcacheClear340)($2)
	jal	$2
	nop

	move	$sp,$fp
	lw	$31,4($sp)
	lw	$fp,0($sp)
	addiu	$sp,$sp,8
	j	$31
	nop

	.set	macro
	.set	reorder
	.end	ClearCaches340
	.size	ClearCaches340, .-ClearCaches340
	.align	2
	.globl	ClearCaches150
	.ent	ClearCaches150
ClearCaches150:
	.frame	$fp,8,$31		# vars= 0, regs= 2/0, args= 0, gp= 0
	.mask	0xc0000000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-8
	sw	$31,4($sp)
	sw	$fp,0($sp)
	move	$fp,$sp
	lui	$2,%hi(DcacheClear150)
	lw	$2,%lo(DcacheClear150)($2)
	jal	$2
	nop

	lui	$2,%hi(IcacheClear150)
	lw	$2,%lo(IcacheClear150)($2)
	jal	$2
	nop

	move	$sp,$fp
	lw	$31,4($sp)
	lw	$fp,0($sp)
	addiu	$sp,$sp,8
	j	$31
	nop

	.set	macro
	.set	reorder
	.end	ClearCaches150
	.size	ClearCaches150, .-ClearCaches150
	.rdata
	.align	2
$LC0:
	.ascii	"/kd\000"
	.align	2
$LC1:
	.ascii	"/vsh/module\000"
	.text
	.align	2
	.globl	sceBootLfatOpenPatched
	.ent	sceBootLfatOpenPatched
sceBootLfatOpenPatched:
	.frame	$fp,16,$31		# vars= 8, regs= 2/0, args= 0, gp= 0
	.mask	0xc0000000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-16
	sw	$31,12($sp)
	sw	$fp,8($sp)
	move	$fp,$sp
	sw	$4,0($fp)
	lw	$3,0($fp)
	lui	$2,%hi($LC0)
	addiu	$2,$2,%lo($LC0)
	move	$4,$3
	move	$5,$2
	li	$6,3			# 0x3
	jal	memcmp
	nop

	bne	$2,$0,$L21
	nop

	lw	$2,0($fp)
	addiu	$3,$2,2
	li	$2,110
	sb	$2,0($3)
	j	$L23
	nop

$L21:
	lw	$3,0($fp)
	lui	$2,%hi($LC1)
	addiu	$2,$2,%lo($LC1)
	move	$4,$3
	move	$5,$2
	li	$6,11			# 0xb
	jal	memcmp
	nop

	bne	$2,$0,$L23
	nop

	lw	$2,0($fp)
	addiu	$3,$2,5
	li	$2,110
	sb	$2,0($3)
$L23:
	lui	$2,%hi(sceBootLfatOpen)
	lw	$2,%lo(sceBootLfatOpen)($2)
	lw	$4,0($fp)
	jal	$2
	nop

	move	$sp,$fp
	lw	$31,12($sp)
	lw	$fp,8($sp)
	addiu	$sp,$sp,16
	j	$31
	nop

	.set	macro
	.set	reorder
	.end	sceBootLfatOpenPatched
	.size	sceBootLfatOpenPatched, .-sceBootLfatOpenPatched
	.align	2
	.globl	sceKernelCheckExecFilePatched
	.ent	sceKernelCheckExecFilePatched
sceKernelCheckExecFilePatched:
	.frame	$fp,40,$31		# vars= 32, regs= 2/0, args= 0, gp= 0
	.mask	0xc0000000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-40
	sw	$31,36($sp)
	sw	$fp,32($sp)
	move	$fp,$sp
	sw	$4,16($fp)
	sw	$5,20($fp)
	lw	$2,16($fp)
	lw	$3,0($2)
	li	$2,-1179451392			# 0xffffffffb9b30000
	ori	$2,$2,0xba81
	addu	$2,$3,$2
	sltu	$2,$2,1
	sw	$2,4($fp)
	lw	$2,4($fp)
	beq	$2,$0,$L27
	nop

	lw	$2,20($fp)
	addiu	$2,$2,68
	lw	$2,0($2)
	beq	$2,$0,$L27
	nop

	lw	$2,20($fp)
	addiu	$3,$2,72
	li	$2,1			# 0x1
	sw	$2,0($3)
	sw	$0,24($fp)
	j	$L30
	nop

$L27:
	lui	$2,%hi(sceKernelCheckExecFile)
	lw	$2,%lo(sceKernelCheckExecFile)($2)
	lw	$4,16($fp)
	lw	$5,20($fp)
	jal	$2
	nop

	sw	$2,8($fp)
	lw	$2,4($fp)
	beq	$2,$0,$L31
	nop

	lw	$2,20($fp)
	addiu	$2,$2,76
	lw	$2,0($2)
	sw	$2,0($fp)
	lw	$2,20($fp)
	addiu	$2,$2,76
	lw	$2,0($2)
	bgez	$2,$L33
	nop

	lw	$2,0($fp)
	addiu	$2,$2,3
	sw	$2,0($fp)
$L33:
	lw	$2,20($fp)
	addiu	$2,$2,8
	lw	$3,0($2)
	li	$2,32			# 0x20
	beq	$3,$2,$L35
	nop

	lw	$2,20($fp)
	addiu	$2,$2,8
	lw	$2,0($2)
	slt	$2,$2,33
	bne	$2,$0,$L31
	nop

	lw	$2,20($fp)
	addiu	$2,$2,8
	lw	$2,0($2)
	slt	$2,$2,82
	beq	$2,$0,$L31
	nop

$L35:
	lw	$3,0($fp)
	li	$2,4			# 0x4
	bne	$2,$0,1f
	div	$0,$3,$2
	break	7
1:
	mflo	$2
	sll	$2,$2,2
	move	$3,$2
	lw	$2,16($fp)
	addu	$2,$3,$2
	lw	$2,0($2)
	andi	$2,$2,0xff00
	beq	$2,$0,$L31
	nop

	lw	$2,20($fp)
	addiu	$3,$2,68
	li	$2,1			# 0x1
	sw	$2,0($3)
	lw	$2,20($fp)
	addiu	$4,$2,88
	lw	$3,0($fp)
	li	$2,4			# 0x4
	bne	$2,$0,1f
	div	$0,$3,$2
	break	7
1:
	mflo	$2
	sll	$2,$2,2
	move	$3,$2
	lw	$2,16($fp)
	addu	$2,$3,$2
	lw	$2,0($2)
	andi	$2,$2,0xffff
	sw	$2,0($4)
	sw	$0,24($fp)
	j	$L30
	nop

$L31:
	lw	$2,8($fp)
	sw	$2,24($fp)
$L30:
	lw	$2,24($fp)
	move	$sp,$fp
	lw	$31,36($sp)
	lw	$fp,32($sp)
	addiu	$sp,$sp,40
	j	$31
	nop

	.set	macro
	.set	reorder
	.end	sceKernelCheckExecFilePatched
	.size	sceKernelCheckExecFilePatched, .-sceKernelCheckExecFilePatched
	.align	2
	.globl	PatchLoadCore340
	.ent	PatchLoadCore340
PatchLoadCore340:
	.frame	$fp,32,$31		# vars= 24, regs= 2/0, args= 0, gp= 0
	.mask	0xc0000000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-32
	sw	$31,28($sp)
	sw	$fp,24($sp)
	move	$fp,$sp
	sw	$4,8($fp)
	sw	$5,12($fp)
	sw	$6,16($fp)
	sw	$7,20($fp)
	lw	$2,20($fp)
	addiu	$2,$2,-3000
	sw	$2,0($fp)
	lui	$2,%hi(sceKernelCheckExecFilePatched)
	addiu	$2,$2,%lo(sceKernelCheckExecFilePatched)
	srl	$3,$2,2
	li	$2,67043328			# 0x3ff0000
	ori	$2,$2,0xffff
	and	$3,$3,$2
	li	$2,201326592			# 0xc000000
	or	$3,$3,$2
	lw	$2,0($fp)
	addiu	$2,$2,5576
	move	$4,$3
	move	$5,$2
	jal	_sw
	nop

	lui	$2,%hi(sceKernelCheckExecFilePatched)
	addiu	$2,$2,%lo(sceKernelCheckExecFilePatched)
	srl	$3,$2,2
	li	$2,67043328			# 0x3ff0000
	ori	$2,$2,0xffff
	and	$3,$3,$2
	li	$2,201326592			# 0xc000000
	or	$3,$3,$2
	lw	$2,0($fp)
	addiu	$2,$2,5656
	move	$4,$3
	move	$5,$2
	jal	_sw
	nop

	lui	$2,%hi(sceKernelCheckExecFilePatched)
	addiu	$2,$2,%lo(sceKernelCheckExecFilePatched)
	srl	$3,$2,2
	li	$2,67043328			# 0x3ff0000
	ori	$2,$2,0xffff
	and	$3,$3,$2
	li	$2,201326592			# 0xc000000
	or	$3,$3,$2
	lw	$2,0($fp)
	addiu	$2,$2,18060
	move	$4,$3
	move	$5,$2
	jal	_sw
	nop

	jal	ClearCaches340
	nop

	lw	$2,0($fp)
	addiu	$2,$2,16308
	move	$3,$2
	lui	$2,%hi(sceKernelCheckExecFile)
	sw	$3,%lo(sceKernelCheckExecFile)($2)
	lw	$2,20($fp)
	lw	$4,8($fp)
	lw	$5,12($fp)
	lw	$6,16($fp)
	jal	$2
	nop

	move	$sp,$fp
	lw	$31,28($sp)
	lw	$fp,24($sp)
	addiu	$sp,$sp,32
	j	$31
	nop

	.set	macro
	.set	reorder
	.end	PatchLoadCore340
	.size	PatchLoadCore340, .-PatchLoadCore340
	.align	2
	.ent	_sw
_sw:
	.frame	$fp,16,$31		# vars= 8, regs= 1/0, args= 0, gp= 0
	.mask	0x40000000,-8
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-16
	sw	$fp,8($sp)
	move	$fp,$sp
	sw	$4,0($fp)
	sw	$5,4($fp)
	lw	$3,4($fp)
	lw	$2,0($fp)
	sw	$2,0($3)
	move	$sp,$fp
	lw	$fp,8($sp)
	addiu	$sp,$sp,16
	j	$31
	nop

	.set	macro
	.set	reorder
	.end	_sw
	.size	_sw, .-_sw
	.align	2
	.globl	PatchLoadCore150
	.ent	PatchLoadCore150
PatchLoadCore150:
	.frame	$fp,24,$31		# vars= 16, regs= 2/0, args= 0, gp= 0
	.mask	0xc0000000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-24
	sw	$31,20($sp)
	sw	$fp,16($sp)
	move	$fp,$sp
	sw	$4,0($fp)
	sw	$5,4($fp)
	sw	$6,8($fp)
	li	$2,873267200			# 0x340d0000
	ori	$4,$2,0x1
	li	$2,-2013200384			# 0xffffffff88010000
	ori	$5,$2,0x52e0
	jal	_sw
	nop

	jal	ClearCaches150
	nop

	lw	$2,8($fp)
	lw	$4,0($fp)
	lw	$5,4($fp)
	jal	$2
	nop

	move	$sp,$fp
	lw	$31,20($sp)
	lw	$fp,16($sp)
	addiu	$sp,$sp,24
	j	$31
	nop

	.set	macro
	.set	reorder
	.end	PatchLoadCore150
	.size	PatchLoadCore150, .-PatchLoadCore150
	.rdata
	.align	2
$LC2:
	.ascii	"19196\000"
	.text
	.align	2
	.globl	Main
	.ent	Main
Main:
	.frame	$fp,24,$31		# vars= 16, regs= 2/0, args= 0, gp= 0
	.mask	0xc0000000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-24
	sw	$31,20($sp)
	sw	$fp,16($sp)
	move	$fp,$sp
	sw	$4,0($fp)
	sw	$5,4($fp)
	sw	$6,8($fp)
	sw	$7,12($fp)
	lui	$2,%hi($LC2)
	addiu	$3,$2,%lo($LC2)
	li	$2,-2000617472			# 0xffffffff88c10000
	ori	$4,$2,0x6cb2
	move	$5,$3
	li	$6,5			# 0x5
	jal	memcmp
	nop

	beq	$2,$0,$L47
	nop

	lui	$2,%hi(sceBootLfatOpenPatched)
	addiu	$2,$2,%lo(sceBootLfatOpenPatched)
	srl	$3,$2,2
	li	$2,67043328			# 0x3ff0000
	ori	$2,$2,0xffff
	and	$3,$3,$2
	li	$2,201326592			# 0xc000000
	or	$2,$3,$2
	move	$4,$2
	li	$2,-2006974464			# 0xffffffff88600000
	ori	$5,$2,0x94
	jal	_sw
	nop

	li	$4,-1348141056			# 0xffffffffafa50000
	li	$2,-2006974464			# 0xffffffff88600000
	ori	$5,$2,0x514c
	jal	_sw
	nop

	li	$4,547553280			# 0x20a30000
	li	$2,-2006974464			# 0xffffffff88600000
	ori	$5,$2,0x5150
	jal	_sw
	nop

	li	$2,65011712			# 0x3e00000
	ori	$4,$2,0x8
	li	$2,-2006974464			# 0xffffffff88600000
	ori	$5,$2,0xfa0
	jal	_sw
	nop

	li	$2,604110848			# 0x24020000
	ori	$4,$2,0x1
	li	$2,-2006974464			# 0xffffffff88600000
	ori	$5,$2,0xfa4
	jal	_sw
	nop

	move	$4,$0
	li	$2,-2006974464			# 0xffffffff88600000
	ori	$5,$2,0x8c
	jal	_sw
	nop

	li	$4,648478720			# 0x26a70000
	li	$2,-2006974464			# 0xffffffff88600000
	ori	$5,$2,0x4c2c
	jal	_sw
	nop

	lui	$2,%hi(PatchLoadCore340)
	addiu	$2,$2,%lo(PatchLoadCore340)
	move	$3,$2
	li	$2,201326592			# 0xc000000
	addu	$3,$3,$2
	li	$2,67043328			# 0x3ff0000
	ori	$2,$2,0xffff
	and	$2,$3,$2
	move	$4,$2
	li	$2,-2006974464			# 0xffffffff88600000
	ori	$5,$2,0x4c30
	jal	_sw
	nop

	li	$4,622657536			# 0x251d0000
	li	$2,-2006974464			# 0xffffffff88600000
	ori	$5,$2,0x4c34
	jal	_sw
	nop

	move	$4,$0
	li	$2,-2006974464			# 0xffffffff88600000
	ori	$5,$2,0x4084
	jal	_sw
	nop

	move	$4,$0
	li	$2,-2006974464			# 0xffffffff88600000
	ori	$5,$2,0x40dc
	jal	_sw
	nop

	li	$2,-1402994688			# 0xffffffffac600000
	ori	$4,$2,0xc7e0
	li	$2,-2006908928			# 0xffffffff88610000
	ori	$5,$2,0x742
	jal	_sw
	nop

	lui	$2,%hi(key_data_340)
	addiu	$4,$2,%lo(key_data_340)
	jal	buld_key_param
	nop

	jal	ClearCaches340
	nop

	j	$L49
	nop

$L47:
	li	$2,37748736			# 0x2400000
	ori	$4,$2,0x3021
	li	$2,-2000683008			# 0xffffffff88c00000
	ori	$5,$2,0xfec
	jal	_sw
	nop

	lui	$2,%hi(PatchLoadCore150)
	addiu	$2,$2,%lo(PatchLoadCore150)
	srl	$3,$2,2
	li	$2,67043328			# 0x3ff0000
	ori	$2,$2,0xffff
	and	$3,$3,$2
	li	$2,201326592			# 0xc000000
	or	$2,$3,$2
	move	$4,$2
	li	$2,-2000683008			# 0xffffffff88c00000
	ori	$5,$2,0xff8
	jal	_sw
	nop

	lui	$3,%hi(Real_Reboot)
	li	$2,-2000683008			# 0xffffffff88c00000
	sw	$2,%lo(Real_Reboot)($3)
	jal	ClearCaches150
	nop

$L49:
	lui	$2,%hi(Real_Reboot)
	lw	$2,%lo(Real_Reboot)($2)
	lw	$4,0($fp)
	lw	$5,4($fp)
	lw	$6,8($fp)
	lw	$7,12($fp)
	jal	$2
	nop

	move	$sp,$fp
	lw	$31,20($sp)
	lw	$fp,16($sp)
	addiu	$sp,$sp,24
	j	$31
	nop

	.set	macro
	.set	reorder
	.end	Main
	.size	Main, .-Main

	.comm	sceKernelCheckExecFile,4,4
	.ident	"GCC: (GNU) 4.1.0 (PSPDEV 20060507)"
