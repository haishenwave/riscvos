
user/init.elf：     文件格式 elf64-littleriscv


Disassembly of section .text:

0000000000000000 <_start>:
   0:	4501                	li	a0,0
   2:	4581                	li	a1,0
   4:	231000ef          	jal	a34 <main>
   8:	4889                	li	a7,2
   a:	00000073          	ecall
   e:	a001                	j	e <_start+0xe>

0000000000000010 <fork>:
  10:	4885                	li	a7,1
  12:	00000073          	ecall
  16:	8082                	ret

0000000000000018 <exit>:
  18:	4889                	li	a7,2
  1a:	00000073          	ecall

000000000000001e <wait>:
  1e:	488d                	li	a7,3
  20:	00000073          	ecall
  24:	8082                	ret

0000000000000026 <kill>:
  26:	4899                	li	a7,6
  28:	00000073          	ecall
  2c:	8082                	ret

000000000000002e <getpid>:
  2e:	48ad                	li	a7,11
  30:	00000073          	ecall
  34:	8082                	ret

0000000000000036 <sbrk>:
  36:	48b1                	li	a7,12
  38:	00000073          	ecall
  3c:	8082                	ret

000000000000003e <read>:
  3e:	4895                	li	a7,5
  40:	00000073          	ecall
  44:	8082                	ret

0000000000000046 <write>:
  46:	48c1                	li	a7,16
  48:	00000073          	ecall
  4c:	8082                	ret

000000000000004e <uptime>:
  4e:	48b9                	li	a7,14
  50:	00000073          	ecall
  54:	8082                	ret

0000000000000056 <open>:
  56:	48bd                	li	a7,15
  58:	00000073          	ecall
  5c:	8082                	ret

000000000000005e <close>:
  5e:	48d5                	li	a7,21
  60:	00000073          	ecall
  64:	8082                	ret

0000000000000066 <unlink>:
  66:	48c9                	li	a7,18
  68:	00000073          	ecall
  6c:	8082                	ret

000000000000006e <logdump>:
  6e:	48d9                	li	a7,22
  70:	00000073          	ecall
  74:	8082                	ret

0000000000000076 <logstat>:
  76:	48dd                	li	a7,23
  78:	00000073          	ecall
  7c:	8082                	ret

000000000000007e <print_uint>:
  7e:	715d                	add	sp,sp,-80
  80:	e0a2                	sd	s0,64(sp)
  82:	e486                	sd	ra,72(sp)
  84:	0800                	add	s0,sp,16
  86:	fc26                	sd	s1,56(sp)
  88:	8722                	mv	a4,s0
  8a:	4825                	li	a6,9
  8c:	e501                	bnez	a0,94 <print_uint+0x16>
  8e:	a0bd                	j	fc <print_uint+0x7e>
  90:	873e                	mv	a4,a5
  92:	8536                	mv	a0,a3
  94:	02b576b3          	remu	a3,a0,a1
  98:	0ff6f793          	zext.b	a5,a3
  9c:	0307861b          	addw	a2,a5,48
  a0:	0577879b          	addw	a5,a5,87
  a4:	0ff7f793          	zext.b	a5,a5
  a8:	00d86463          	bltu	a6,a3,b0 <print_uint+0x32>
  ac:	0ff67793          	zext.b	a5,a2
  b0:	00f70023          	sb	a5,0(a4)
  b4:	02b556b3          	divu	a3,a0,a1
  b8:	00170793          	add	a5,a4,1
  bc:	fcb57ae3          	bgeu	a0,a1,90 <print_uint+0x12>
  c0:	9f01                	subw	a4,a4,s0
  c2:	0017079b          	addw	a5,a4,1
  c6:	fff7871b          	addw	a4,a5,-1
  ca:	fff40493          	add	s1,s0,-1
  ce:	1702                	sll	a4,a4,0x20
  d0:	94be                	add	s1,s1,a5
  d2:	9301                	srl	a4,a4,0x20
  d4:	943e                	add	s0,s0,a5
  d6:	8c99                	sub	s1,s1,a4
  d8:	fff44783          	lbu	a5,-1(s0)
  dc:	4605                	li	a2,1
  de:	147d                	add	s0,s0,-1
  e0:	00f10593          	add	a1,sp,15
  e4:	4505                	li	a0,1
  e6:	00f107a3          	sb	a5,15(sp)
  ea:	f5dff0ef          	jal	46 <write>
  ee:	fe9415e3          	bne	s0,s1,d8 <print_uint+0x5a>
  f2:	60a6                	ld	ra,72(sp)
  f4:	6406                	ld	s0,64(sp)
  f6:	74e2                	ld	s1,56(sp)
  f8:	6161                	add	sp,sp,80
  fa:	8082                	ret
  fc:	85a2                	mv	a1,s0
  fe:	03000793          	li	a5,48
 102:	4605                	li	a2,1
 104:	4505                	li	a0,1
 106:	00f10823          	sb	a5,16(sp)
 10a:	f3dff0ef          	jal	46 <write>
 10e:	60a6                	ld	ra,72(sp)
 110:	6406                	ld	s0,64(sp)
 112:	74e2                	ld	s1,56(sp)
 114:	6161                	add	sp,sp,80
 116:	8082                	ret

0000000000000118 <strlen>:
 118:	00054783          	lbu	a5,0(a0)
 11c:	cf81                	beqz	a5,134 <strlen+0x1c>
 11e:	0505                	add	a0,a0,1
 120:	87aa                	mv	a5,a0
 122:	0007c703          	lbu	a4,0(a5)
 126:	86be                	mv	a3,a5
 128:	0785                	add	a5,a5,1
 12a:	ff65                	bnez	a4,122 <strlen+0xa>
 12c:	40a6853b          	subw	a0,a3,a0
 130:	2505                	addw	a0,a0,1
 132:	8082                	ret
 134:	4501                	li	a0,0
 136:	8082                	ret

0000000000000138 <memset>:
 138:	0ff5f593          	zext.b	a1,a1
 13c:	87aa                	mv	a5,a0
 13e:	00a60733          	add	a4,a2,a0
 142:	00c05763          	blez	a2,150 <memset+0x18>
 146:	00b78023          	sb	a1,0(a5)
 14a:	0785                	add	a5,a5,1
 14c:	fee79de3          	bne	a5,a4,146 <memset+0xe>
 150:	8082                	ret

0000000000000152 <puts>:
 152:	00054783          	lbu	a5,0(a0)
 156:	85aa                	mv	a1,a0
 158:	cf89                	beqz	a5,172 <puts+0x20>
 15a:	00150693          	add	a3,a0,1
 15e:	87b6                	mv	a5,a3
 160:	0007c703          	lbu	a4,0(a5)
 164:	863e                	mv	a2,a5
 166:	0785                	add	a5,a5,1
 168:	ff65                	bnez	a4,160 <puts+0xe>
 16a:	9e15                	subw	a2,a2,a3
 16c:	2605                	addw	a2,a2,1
 16e:	4505                	li	a0,1
 170:	bdd9                	j	46 <write>
 172:	4601                	li	a2,0
 174:	4505                	li	a0,1
 176:	bdc1                	j	46 <write>

0000000000000178 <printf>:
 178:	7135                	add	sp,sp,-160
 17a:	e13a                	sd	a4,128(sp)
 17c:	ec86                	sd	ra,88(sp)
 17e:	e8a2                	sd	s0,80(sp)
 180:	e4a6                	sd	s1,72(sp)
 182:	e0ca                	sd	s2,64(sp)
 184:	fc4e                	sd	s3,56(sp)
 186:	f852                	sd	s4,48(sp)
 188:	f456                	sd	s5,40(sp)
 18a:	f05a                	sd	s6,32(sp)
 18c:	ec5e                	sd	s7,24(sp)
 18e:	f4ae                	sd	a1,104(sp)
 190:	f8b2                	sd	a2,112(sp)
 192:	fcb6                	sd	a3,120(sp)
 194:	e53e                	sd	a5,136(sp)
 196:	e942                	sd	a6,144(sp)
 198:	ed46                	sd	a7,152(sp)
 19a:	00054783          	lbu	a5,0(a0)
 19e:	10b8                	add	a4,sp,104
 1a0:	e43a                	sd	a4,8(sp)
 1a2:	18078763          	beqz	a5,330 <printf+0x1b8>
 1a6:	00001a17          	auipc	s4,0x1
 1aa:	9caa0a13          	add	s4,s4,-1590 # b70 <main+0x13c>
 1ae:	4985                	li	s3,1
 1b0:	8b2a                	mv	s6,a0
 1b2:	4a85                	li	s5,1
 1b4:	02500413          	li	s0,37
 1b8:	4955                	li	s2,21
 1ba:	00001497          	auipc	s1,0x1
 1be:	9ba48493          	add	s1,s1,-1606 # b74 <main+0x140>
 1c2:	414989bb          	subw	s3,s3,s4
 1c6:	10879963          	bne	a5,s0,2d8 <printf+0x160>
 1ca:	001b4783          	lbu	a5,1(s6)
 1ce:	12078a63          	beqz	a5,302 <printf+0x18a>
 1d2:	10878f63          	beq	a5,s0,2f0 <printf+0x178>
 1d6:	f9d7879b          	addw	a5,a5,-99
 1da:	0ff7f793          	zext.b	a5,a5
 1de:	00f96763          	bltu	s2,a5,1ec <printf+0x74>
 1e2:	078a                	sll	a5,a5,0x2
 1e4:	97a6                	add	a5,a5,s1
 1e6:	439c                	lw	a5,0(a5)
 1e8:	97a6                	add	a5,a5,s1
 1ea:	8782                	jr	a5
 1ec:	4605                	li	a2,1
 1ee:	00710593          	add	a1,sp,7
 1f2:	4505                	li	a0,1
 1f4:	008103a3          	sb	s0,7(sp)
 1f8:	e4fff0ef          	jal	46 <write>
 1fc:	001b4783          	lbu	a5,1(s6)
 200:	4605                	li	a2,1
 202:	00710593          	add	a1,sp,7
 206:	4505                	li	a0,1
 208:	00f103a3          	sb	a5,7(sp)
 20c:	e3bff0ef          	jal	46 <write>
 210:	001b0713          	add	a4,s6,1
 214:	000a851b          	sext.w	a0,s5
 218:	00174783          	lbu	a5,1(a4)
 21c:	00170b13          	add	s6,a4,1
 220:	2a85                	addw	s5,s5,1
 222:	f3d5                	bnez	a5,1c6 <printf+0x4e>
 224:	60e6                	ld	ra,88(sp)
 226:	6446                	ld	s0,80(sp)
 228:	64a6                	ld	s1,72(sp)
 22a:	6906                	ld	s2,64(sp)
 22c:	79e2                	ld	s3,56(sp)
 22e:	7a42                	ld	s4,48(sp)
 230:	7aa2                	ld	s5,40(sp)
 232:	7b02                	ld	s6,32(sp)
 234:	6be2                	ld	s7,24(sp)
 236:	610d                	add	sp,sp,160
 238:	8082                	ret
 23a:	67a2                	ld	a5,8(sp)
 23c:	45c1                	li	a1,16
 23e:	0007e503          	lwu	a0,0(a5)
 242:	07a1                	add	a5,a5,8
 244:	e43e                	sd	a5,8(sp)
 246:	e39ff0ef          	jal	7e <print_uint>
 24a:	b7d9                	j	210 <printf+0x98>
 24c:	67a2                	ld	a5,8(sp)
 24e:	638c                	ld	a1,0(a5)
 250:	07a1                	add	a5,a5,8
 252:	e43e                	sd	a5,8(sp)
 254:	c9d5                	beqz	a1,308 <printf+0x190>
 256:	0005c783          	lbu	a5,0(a1)
 25a:	cbe9                	beqz	a5,32c <printf+0x1b4>
 25c:	00158693          	add	a3,a1,1
 260:	87b6                	mv	a5,a3
 262:	0007c703          	lbu	a4,0(a5)
 266:	863e                	mv	a2,a5
 268:	0785                	add	a5,a5,1
 26a:	ff65                	bnez	a4,262 <printf+0xea>
 26c:	9e15                	subw	a2,a2,a3
 26e:	2605                	addw	a2,a2,1
 270:	4505                	li	a0,1
 272:	dd5ff0ef          	jal	46 <write>
 276:	bf69                	j	210 <printf+0x98>
 278:	00001797          	auipc	a5,0x1
 27c:	8f878793          	add	a5,a5,-1800 # b70 <main+0x13c>
 280:	0017c703          	lbu	a4,1(a5)
 284:	863e                	mv	a2,a5
 286:	0785                	add	a5,a5,1
 288:	ff65                	bnez	a4,280 <printf+0x108>
 28a:	85d2                	mv	a1,s4
 28c:	00c9863b          	addw	a2,s3,a2
 290:	4505                	li	a0,1
 292:	db5ff0ef          	jal	46 <write>
 296:	67a2                	ld	a5,8(sp)
 298:	45c1                	li	a1,16
 29a:	6388                	ld	a0,0(a5)
 29c:	07a1                	add	a5,a5,8
 29e:	e43e                	sd	a5,8(sp)
 2a0:	ddfff0ef          	jal	7e <print_uint>
 2a4:	b7b5                	j	210 <printf+0x98>
 2a6:	67a2                	ld	a5,8(sp)
 2a8:	0007ab83          	lw	s7,0(a5)
 2ac:	07a1                	add	a5,a5,8
 2ae:	e43e                	sd	a5,8(sp)
 2b0:	060bc163          	bltz	s7,312 <printf+0x19a>
 2b4:	45a9                	li	a1,10
 2b6:	855e                	mv	a0,s7
 2b8:	dc7ff0ef          	jal	7e <print_uint>
 2bc:	bf91                	j	210 <printf+0x98>
 2be:	67a2                	ld	a5,8(sp)
 2c0:	4605                	li	a2,1
 2c2:	00710593          	add	a1,sp,7
 2c6:	4398                	lw	a4,0(a5)
 2c8:	4505                	li	a0,1
 2ca:	07a1                	add	a5,a5,8
 2cc:	e43e                	sd	a5,8(sp)
 2ce:	00e103a3          	sb	a4,7(sp)
 2d2:	d75ff0ef          	jal	46 <write>
 2d6:	bf2d                	j	210 <printf+0x98>
 2d8:	4605                	li	a2,1
 2da:	00710593          	add	a1,sp,7
 2de:	4505                	li	a0,1
 2e0:	00f103a3          	sb	a5,7(sp)
 2e4:	d63ff0ef          	jal	46 <write>
 2e8:	000a851b          	sext.w	a0,s5
 2ec:	875a                	mv	a4,s6
 2ee:	b72d                	j	218 <printf+0xa0>
 2f0:	4605                	li	a2,1
 2f2:	00710593          	add	a1,sp,7
 2f6:	4505                	li	a0,1
 2f8:	008103a3          	sb	s0,7(sp)
 2fc:	d4bff0ef          	jal	46 <write>
 300:	bf01                	j	210 <printf+0x98>
 302:	fffa851b          	addw	a0,s5,-1
 306:	bf39                	j	224 <printf+0xac>
 308:	00001597          	auipc	a1,0x1
 30c:	86058593          	add	a1,a1,-1952 # b68 <main+0x134>
 310:	b7b1                	j	25c <printf+0xe4>
 312:	02d00793          	li	a5,45
 316:	4605                	li	a2,1
 318:	00710593          	add	a1,sp,7
 31c:	4505                	li	a0,1
 31e:	00f103a3          	sb	a5,7(sp)
 322:	41700bb3          	neg	s7,s7
 326:	d21ff0ef          	jal	46 <write>
 32a:	b769                	j	2b4 <printf+0x13c>
 32c:	4601                	li	a2,0
 32e:	b789                	j	270 <printf+0xf8>
 330:	4501                	li	a0,0
 332:	bdcd                	j	224 <printf+0xac>

0000000000000334 <strcmp>:
 334:	00054783          	lbu	a5,0(a0)
 338:	e791                	bnez	a5,344 <strcmp+0x10>
 33a:	a02d                	j	364 <strcmp+0x30>
 33c:	00054783          	lbu	a5,0(a0)
 340:	cf89                	beqz	a5,35a <strcmp+0x26>
 342:	85b6                	mv	a1,a3
 344:	0005c703          	lbu	a4,0(a1)
 348:	0505                	add	a0,a0,1
 34a:	00158693          	add	a3,a1,1
 34e:	fef707e3          	beq	a4,a5,33c <strcmp+0x8>
 352:	0007851b          	sext.w	a0,a5
 356:	9d19                	subw	a0,a0,a4
 358:	8082                	ret
 35a:	0015c703          	lbu	a4,1(a1)
 35e:	4501                	li	a0,0
 360:	9d19                	subw	a0,a0,a4
 362:	8082                	ret
 364:	0005c703          	lbu	a4,0(a1)
 368:	4501                	li	a0,0
 36a:	b7f5                	j	356 <strcmp+0x22>

000000000000036c <test_fs_integrity>:
 36c:	7159                	add	sp,sp,-112
 36e:	00001517          	auipc	a0,0x1
 372:	86250513          	add	a0,a0,-1950 # bd0 <main+0x19c>
 376:	f486                	sd	ra,104(sp)
 378:	f0a2                	sd	s0,96(sp)
 37a:	eca6                	sd	s1,88(sp)
 37c:	e8ca                	sd	s2,80(sp)
 37e:	e4ce                	sd	s3,72(sp)
 380:	df9ff0ef          	jal	178 <printf>
 384:	20200593          	li	a1,514
 388:	00001517          	auipc	a0,0x1
 38c:	87050513          	add	a0,a0,-1936 # bf8 <main+0x1c4>
 390:	cc7ff0ef          	jal	56 <open>
 394:	14054963          	bltz	a0,4e6 <test_fs_integrity+0x17a>
 398:	85aa                	mv	a1,a0
 39a:	84aa                	mv	s1,a0
 39c:	00001517          	auipc	a0,0x1
 3a0:	88c50513          	add	a0,a0,-1908 # c28 <main+0x1f4>
 3a4:	dd5ff0ef          	jal	178 <printf>
 3a8:	00001417          	auipc	s0,0x1
 3ac:	27040413          	add	s0,s0,624 # 1618 <buffer>
 3b0:	8522                	mv	a0,s0
 3b2:	d67ff0ef          	jal	118 <strlen>
 3b6:	862a                	mv	a2,a0
 3b8:	892a                	mv	s2,a0
 3ba:	85a2                	mv	a1,s0
 3bc:	8526                	mv	a0,s1
 3be:	c89ff0ef          	jal	46 <write>
 3c2:	89aa                	mv	s3,a0
 3c4:	10a91163          	bne	s2,a0,4c6 <test_fs_integrity+0x15a>
 3c8:	85ca                	mv	a1,s2
 3ca:	00001517          	auipc	a0,0x1
 3ce:	8ae50513          	add	a0,a0,-1874 # c78 <main+0x244>
 3d2:	da7ff0ef          	jal	178 <printf>
 3d6:	8526                	mv	a0,s1
 3d8:	c87ff0ef          	jal	5e <close>
 3dc:	4581                	li	a1,0
 3de:	00001517          	auipc	a0,0x1
 3e2:	81a50513          	add	a0,a0,-2022 # bf8 <main+0x1c4>
 3e6:	c71ff0ef          	jal	56 <open>
 3ea:	84aa                	mv	s1,a0
 3ec:	12054b63          	bltz	a0,522 <test_fs_integrity+0x1b6>
 3f0:	04000613          	li	a2,64
 3f4:	4581                	li	a1,0
 3f6:	850a                	mv	a0,sp
 3f8:	d41ff0ef          	jal	138 <memset>
 3fc:	03f00613          	li	a2,63
 400:	858a                	mv	a1,sp
 402:	8526                	mv	a0,s1
 404:	c3bff0ef          	jal	3e <read>
 408:	892a                	mv	s2,a0
 40a:	14054163          	bltz	a0,54c <test_fs_integrity+0x1e0>
 40e:	04050793          	add	a5,a0,64
 412:	00044703          	lbu	a4,0(s0)
 416:	978a                	add	a5,a5,sp
 418:	fc078023          	sb	zero,-64(a5)
 41c:	868a                	mv	a3,sp
 41e:	87a2                	mv	a5,s0
 420:	e719                	bnez	a4,42e <test_fs_integrity+0xc2>
 422:	aa3d                	j	560 <test_fs_integrity+0x1f4>
 424:	0007c703          	lbu	a4,0(a5)
 428:	0e070163          	beqz	a4,50a <test_fs_integrity+0x19e>
 42c:	86c6                	mv	a3,a7
 42e:	0006c803          	lbu	a6,0(a3)
 432:	0785                	add	a5,a5,1
 434:	00168893          	add	a7,a3,1
 438:	fee806e3          	beq	a6,a4,424 <test_fs_integrity+0xb8>
 43c:	00001517          	auipc	a0,0x1
 440:	8ac50513          	add	a0,a0,-1876 # ce8 <main+0x2b4>
 444:	d35ff0ef          	jal	178 <printf>
 448:	864e                	mv	a2,s3
 44a:	00001597          	auipc	a1,0x1
 44e:	1ce58593          	add	a1,a1,462 # 1618 <buffer>
 452:	00001517          	auipc	a0,0x1
 456:	8ae50513          	add	a0,a0,-1874 # d00 <main+0x2cc>
 45a:	d1fff0ef          	jal	178 <printf>
 45e:	864a                	mv	a2,s2
 460:	858a                	mv	a1,sp
 462:	00001517          	auipc	a0,0x1
 466:	8be50513          	add	a0,a0,-1858 # d20 <main+0x2ec>
 46a:	d0fff0ef          	jal	178 <printf>
 46e:	0c091663          	bnez	s2,53a <test_fs_integrity+0x1ce>
 472:	8526                	mv	a0,s1
 474:	bebff0ef          	jal	5e <close>
 478:	00000517          	auipc	a0,0x0
 47c:	78050513          	add	a0,a0,1920 # bf8 <main+0x1c4>
 480:	be7ff0ef          	jal	66 <unlink>
 484:	ed25                	bnez	a0,4fc <test_fs_integrity+0x190>
 486:	00001517          	auipc	a0,0x1
 48a:	8da50513          	add	a0,a0,-1830 # d60 <main+0x32c>
 48e:	cebff0ef          	jal	178 <printf>
 492:	4581                	li	a1,0
 494:	00000517          	auipc	a0,0x0
 498:	76450513          	add	a0,a0,1892 # bf8 <main+0x1c4>
 49c:	bbbff0ef          	jal	56 <open>
 4a0:	842a                	mv	s0,a0
 4a2:	08054563          	bltz	a0,52c <test_fs_integrity+0x1c0>
 4a6:	00001517          	auipc	a0,0x1
 4aa:	90a50513          	add	a0,a0,-1782 # db0 <main+0x37c>
 4ae:	ccbff0ef          	jal	178 <printf>
 4b2:	8522                	mv	a0,s0
 4b4:	babff0ef          	jal	5e <close>
 4b8:	70a6                	ld	ra,104(sp)
 4ba:	7406                	ld	s0,96(sp)
 4bc:	64e6                	ld	s1,88(sp)
 4be:	6946                	ld	s2,80(sp)
 4c0:	69a6                	ld	s3,72(sp)
 4c2:	6165                	add	sp,sp,112
 4c4:	8082                	ret
 4c6:	864a                	mv	a2,s2
 4c8:	85aa                	mv	a1,a0
 4ca:	00000517          	auipc	a0,0x0
 4ce:	77e50513          	add	a0,a0,1918 # c48 <main+0x214>
 4d2:	ca7ff0ef          	jal	178 <printf>
 4d6:	7406                	ld	s0,96(sp)
 4d8:	70a6                	ld	ra,104(sp)
 4da:	6946                	ld	s2,80(sp)
 4dc:	69a6                	ld	s3,72(sp)
 4de:	8526                	mv	a0,s1
 4e0:	64e6                	ld	s1,88(sp)
 4e2:	6165                	add	sp,sp,112
 4e4:	bead                	j	5e <close>
 4e6:	00000517          	auipc	a0,0x0
 4ea:	72250513          	add	a0,a0,1826 # c08 <main+0x1d4>
 4ee:	7406                	ld	s0,96(sp)
 4f0:	70a6                	ld	ra,104(sp)
 4f2:	64e6                	ld	s1,88(sp)
 4f4:	6946                	ld	s2,80(sp)
 4f6:	69a6                	ld	s3,72(sp)
 4f8:	6165                	add	sp,sp,112
 4fa:	b9bd                	j	178 <printf>
 4fc:	00001517          	auipc	a0,0x1
 500:	87c50513          	add	a0,a0,-1924 # d78 <main+0x344>
 504:	c75ff0ef          	jal	178 <printf>
 508:	b769                	j	492 <test_fs_integrity+0x126>
 50a:	0016c783          	lbu	a5,1(a3)
 50e:	f20797e3          	bnez	a5,43c <test_fs_integrity+0xd0>
 512:	858a                	mv	a1,sp
 514:	00000517          	auipc	a0,0x0
 518:	7b450513          	add	a0,a0,1972 # cc8 <main+0x294>
 51c:	c5dff0ef          	jal	178 <printf>
 520:	bf89                	j	472 <test_fs_integrity+0x106>
 522:	00000517          	auipc	a0,0x0
 526:	76e50513          	add	a0,a0,1902 # c90 <main+0x25c>
 52a:	b7d1                	j	4ee <test_fs_integrity+0x182>
 52c:	00001517          	auipc	a0,0x1
 530:	86450513          	add	a0,a0,-1948 # d90 <main+0x35c>
 534:	c45ff0ef          	jal	178 <printf>
 538:	b741                	j	4b8 <test_fs_integrity+0x14c>
 53a:	00014583          	lbu	a1,0(sp)
 53e:	00001517          	auipc	a0,0x1
 542:	80250513          	add	a0,a0,-2046 # d40 <main+0x30c>
 546:	c33ff0ef          	jal	178 <printf>
 54a:	b725                	j	472 <test_fs_integrity+0x106>
 54c:	00000517          	auipc	a0,0x0
 550:	76450513          	add	a0,a0,1892 # cb0 <main+0x27c>
 554:	c25ff0ef          	jal	178 <printf>
 558:	8526                	mv	a0,s1
 55a:	b05ff0ef          	jal	5e <close>
 55e:	bfa9                	j	4b8 <test_fs_integrity+0x14c>
 560:	00014783          	lbu	a5,0(sp)
 564:	b76d                	j	50e <test_fs_integrity+0x1a2>

0000000000000566 <test_fs_concurrent>:
 566:	7139                	add	sp,sp,-64
 568:	00001517          	auipc	a0,0x1
 56c:	87050513          	add	a0,a0,-1936 # dd8 <main+0x3a4>
 570:	fc06                	sd	ra,56(sp)
 572:	f822                	sd	s0,48(sp)
 574:	f426                	sd	s1,40(sp)
 576:	f04a                	sd	s2,32(sp)
 578:	ec4e                	sd	s3,24(sp)
 57a:	bffff0ef          	jal	178 <printf>
 57e:	a93ff0ef          	jal	10 <fork>
 582:	842a                	mv	s0,a0
 584:	cd0d                	beqz	a0,5be <test_fs_concurrent+0x58>
 586:	a8bff0ef          	jal	10 <fork>
 58a:	c90d                	beqz	a0,5bc <test_fs_concurrent+0x56>
 58c:	a85ff0ef          	jal	10 <fork>
 590:	4409                	li	s0,2
 592:	c515                	beqz	a0,5be <test_fs_concurrent+0x58>
 594:	850a                	mv	a0,sp
 596:	a89ff0ef          	jal	1e <wait>
 59a:	850a                	mv	a0,sp
 59c:	a83ff0ef          	jal	1e <wait>
 5a0:	850a                	mv	a0,sp
 5a2:	a7dff0ef          	jal	1e <wait>
 5a6:	7442                	ld	s0,48(sp)
 5a8:	70e2                	ld	ra,56(sp)
 5aa:	74a2                	ld	s1,40(sp)
 5ac:	7902                	ld	s2,32(sp)
 5ae:	69e2                	ld	s3,24(sp)
 5b0:	00001517          	auipc	a0,0x1
 5b4:	87850513          	add	a0,a0,-1928 # e28 <main+0x3f4>
 5b8:	6121                	add	sp,sp,64
 5ba:	be7d                	j	178 <printf>
 5bc:	4405                	li	s0,1
 5be:	74736737          	lui	a4,0x74736
 5c2:	57470713          	add	a4,a4,1396 # 74736574 <buffer+0x74734f5c>
 5c6:	0304079b          	addw	a5,s0,48
 5ca:	c03a                	sw	a4,0(sp)
 5cc:	05f00713          	li	a4,95
 5d0:	00e10223          	sb	a4,4(sp)
 5d4:	00f102a3          	sb	a5,5(sp)
 5d8:	00010323          	sb	zero,6(sp)
 5dc:	4915                	li	s2,5
 5de:	00001997          	auipc	s3,0x1
 5e2:	82298993          	add	s3,s3,-2014 # e00 <main+0x3cc>
 5e6:	20200593          	li	a1,514
 5ea:	850a                	mv	a0,sp
 5ec:	a6bff0ef          	jal	56 <open>
 5f0:	84aa                	mv	s1,a0
 5f2:	00054c63          	bltz	a0,60a <test_fs_concurrent+0xa4>
 5f6:	4611                	li	a2,4
 5f8:	85ce                	mv	a1,s3
 5fa:	a4dff0ef          	jal	46 <write>
 5fe:	8526                	mv	a0,s1
 600:	a5fff0ef          	jal	5e <close>
 604:	850a                	mv	a0,sp
 606:	a61ff0ef          	jal	66 <unlink>
 60a:	397d                	addw	s2,s2,-1
 60c:	fc091de3          	bnez	s2,5e6 <test_fs_concurrent+0x80>
 610:	85a2                	mv	a1,s0
 612:	00000517          	auipc	a0,0x0
 616:	7f650513          	add	a0,a0,2038 # e08 <main+0x3d4>
 61a:	b5fff0ef          	jal	178 <printf>
 61e:	4501                	li	a0,0
 620:	9f9ff0ef          	jal	18 <exit>

0000000000000624 <test_fs_multiple_files>:
 624:	7135                	add	sp,sp,-160
 626:	00001517          	auipc	a0,0x1
 62a:	83250513          	add	a0,a0,-1998 # e58 <main+0x424>
 62e:	ed06                	sd	ra,152(sp)
 630:	fcce                	sd	s3,120(sp)
 632:	f8d2                	sd	s4,112(sp)
 634:	f4d6                	sd	s5,104(sp)
 636:	f0da                	sd	s6,96(sp)
 638:	e922                	sd	s0,144(sp)
 63a:	e526                	sd	s1,136(sp)
 63c:	e14a                	sd	s2,128(sp)
 63e:	ecde                	sd	s7,88(sp)
 640:	b39ff0ef          	jal	178 <printf>
 644:	4595                	li	a1,5
 646:	00001517          	auipc	a0,0x1
 64a:	83a50513          	add	a0,a0,-1990 # e80 <main+0x44c>
 64e:	656c7ab7          	lui	s5,0x656c7
 652:	b27ff0ef          	jal	178 <printf>
 656:	4985                	li	s3,1
 658:	966a8a93          	add	s5,s5,-1690 # 656c6966 <buffer+0x656c534e>
 65c:	00001a17          	auipc	s4,0x1
 660:	864a0a13          	add	s4,s4,-1948 # ec0 <main+0x48c>
 664:	4b19                	li	s6,6
 666:	02f9879b          	addw	a5,s3,47
 66a:	20200593          	li	a1,514
 66e:	850a                	mv	a0,sp
 670:	c056                	sw	s5,0(sp)
 672:	00f10223          	sb	a5,4(sp)
 676:	000102a3          	sb	zero,5(sp)
 67a:	9ddff0ef          	jal	56 <open>
 67e:	84aa                	mv	s1,a0
 680:	12054663          	bltz	a0,7ac <test_fs_multiple_files+0x188>
 684:	4401                	li	s0,0
 686:	0009891b          	sext.w	s2,s3
 68a:	2405                	addw	s0,s0,1
 68c:	4611                	li	a2,4
 68e:	85d2                	mv	a1,s4
 690:	8526                	mv	a0,s1
 692:	9b5ff0ef          	jal	46 <write>
 696:	ff241ae3          	bne	s0,s2,68a <test_fs_multiple_files+0x66>
 69a:	2985                	addw	s3,s3,1
 69c:	8526                	mv	a0,s1
 69e:	9c1ff0ef          	jal	5e <close>
 6a2:	fd6992e3          	bne	s3,s6,666 <test_fs_multiple_files+0x42>
 6a6:	4595                	li	a1,5
 6a8:	00001517          	auipc	a0,0x1
 6ac:	82050513          	add	a0,a0,-2016 # ec8 <main+0x494>
 6b0:	ac9ff0ef          	jal	178 <printf>
 6b4:	00001517          	auipc	a0,0x1
 6b8:	83450513          	add	a0,a0,-1996 # ee8 <main+0x4b4>
 6bc:	656c79b7          	lui	s3,0x656c7
 6c0:	ab9ff0ef          	jal	178 <printf>
 6c4:	4911                	li	s2,4
 6c6:	03000413          	li	s0,48
 6ca:	4a85                	li	s5,1
 6cc:	96698993          	add	s3,s3,-1690 # 656c6966 <buffer+0x656c534e>
 6d0:	00001b17          	auipc	s6,0x1
 6d4:	850b0b13          	add	s6,s6,-1968 # f20 <main+0x4ec>
 6d8:	00001b97          	auipc	s7,0x1
 6dc:	830b8b93          	add	s7,s7,-2000 # f08 <main+0x4d4>
 6e0:	4a61                	li	s4,24
 6e2:	a035                	j	70e <test_fs_multiple_files+0xea>
 6e4:	95bff0ef          	jal	3e <read>
 6e8:	86aa                	mv	a3,a0
 6ea:	858a                	mv	a1,sp
 6ec:	855a                	mv	a0,s6
 6ee:	0009061b          	sext.w	a2,s2
 6f2:	01268563          	beq	a3,s2,6fc <test_fs_multiple_files+0xd8>
 6f6:	a83ff0ef          	jal	178 <printf>
 6fa:	4a81                	li	s5,0
 6fc:	8526                	mv	a0,s1
 6fe:	2405                	addw	s0,s0,1
 700:	2911                	addw	s2,s2,4
 702:	95dff0ef          	jal	5e <close>
 706:	0ff47413          	zext.b	s0,s0
 70a:	03490c63          	beq	s2,s4,742 <test_fs_multiple_files+0x11e>
 70e:	4581                	li	a1,0
 710:	850a                	mv	a0,sp
 712:	00810223          	sb	s0,4(sp)
 716:	c04e                	sw	s3,0(sp)
 718:	000102a3          	sb	zero,5(sp)
 71c:	93bff0ef          	jal	56 <open>
 720:	04000613          	li	a2,64
 724:	080c                	add	a1,sp,16
 726:	84aa                	mv	s1,a0
 728:	fa055ee3          	bgez	a0,6e4 <test_fs_multiple_files+0xc0>
 72c:	858a                	mv	a1,sp
 72e:	855e                	mv	a0,s7
 730:	2405                	addw	s0,s0,1
 732:	2911                	addw	s2,s2,4
 734:	a45ff0ef          	jal	178 <printf>
 738:	4a81                	li	s5,0
 73a:	0ff47413          	zext.b	s0,s0
 73e:	fd4918e3          	bne	s2,s4,70e <test_fs_multiple_files+0xea>
 742:	040a9e63          	bnez	s5,79e <test_fs_multiple_files+0x17a>
 746:	00001517          	auipc	a0,0x1
 74a:	82250513          	add	a0,a0,-2014 # f68 <main+0x534>
 74e:	656c74b7          	lui	s1,0x656c7
 752:	a27ff0ef          	jal	178 <printf>
 756:	03000413          	li	s0,48
 75a:	96648493          	add	s1,s1,-1690 # 656c6966 <buffer+0x656c534e>
 75e:	03500913          	li	s2,53
 762:	00810223          	sb	s0,4(sp)
 766:	2405                	addw	s0,s0,1
 768:	850a                	mv	a0,sp
 76a:	0ff47413          	zext.b	s0,s0
 76e:	c026                	sw	s1,0(sp)
 770:	000102a3          	sb	zero,5(sp)
 774:	8f3ff0ef          	jal	66 <unlink>
 778:	ff2415e3          	bne	s0,s2,762 <test_fs_multiple_files+0x13e>
 77c:	00001517          	auipc	a0,0x1
 780:	80450513          	add	a0,a0,-2044 # f80 <main+0x54c>
 784:	9f5ff0ef          	jal	178 <printf>
 788:	60ea                	ld	ra,152(sp)
 78a:	644a                	ld	s0,144(sp)
 78c:	64aa                	ld	s1,136(sp)
 78e:	690a                	ld	s2,128(sp)
 790:	79e6                	ld	s3,120(sp)
 792:	7a46                	ld	s4,112(sp)
 794:	7aa6                	ld	s5,104(sp)
 796:	7b06                	ld	s6,96(sp)
 798:	6be6                	ld	s7,88(sp)
 79a:	610d                	add	sp,sp,160
 79c:	8082                	ret
 79e:	00000517          	auipc	a0,0x0
 7a2:	7aa50513          	add	a0,a0,1962 # f48 <main+0x514>
 7a6:	9d3ff0ef          	jal	178 <printf>
 7aa:	bf71                	j	746 <test_fs_multiple_files+0x122>
 7ac:	858a                	mv	a1,sp
 7ae:	00000517          	auipc	a0,0x0
 7b2:	6f250513          	add	a0,a0,1778 # ea0 <main+0x46c>
 7b6:	9c3ff0ef          	jal	178 <printf>
 7ba:	b7f9                	j	788 <test_fs_multiple_files+0x164>

00000000000007bc <test_fs_large_file>:
 7bc:	dd010113          	add	sp,sp,-560
 7c0:	00000517          	auipc	a0,0x0
 7c4:	7e850513          	add	a0,a0,2024 # fa8 <main+0x574>
 7c8:	22113423          	sd	ra,552(sp)
 7cc:	22813023          	sd	s0,544(sp)
 7d0:	20913c23          	sd	s1,536(sp)
 7d4:	21213823          	sd	s2,528(sp)
 7d8:	21313423          	sd	s3,520(sp)
 7dc:	99dff0ef          	jal	178 <printf>
 7e0:	20200593          	li	a1,514
 7e4:	00000517          	auipc	a0,0x0
 7e8:	7e450513          	add	a0,a0,2020 # fc8 <main+0x594>
 7ec:	86bff0ef          	jal	56 <open>
 7f0:	12054563          	bltz	a0,91a <test_fs_large_file+0x15e>
 7f4:	892a                	mv	s2,a0
 7f6:	868a                	mv	a3,sp
 7f8:	4781                	li	a5,0
 7fa:	45e9                	li	a1,26
 7fc:	20000613          	li	a2,512
 800:	02b7e73b          	remw	a4,a5,a1
 804:	0685                	add	a3,a3,1
 806:	2785                	addw	a5,a5,1
 808:	0417071b          	addw	a4,a4,65
 80c:	fee68fa3          	sb	a4,-1(a3)
 810:	fec798e3          	bne	a5,a2,800 <test_fs_large_file+0x44>
 814:	45a9                	li	a1,10
 816:	00000517          	auipc	a0,0x0
 81a:	7ea50513          	add	a0,a0,2026 # 1000 <main+0x5cc>
 81e:	95bff0ef          	jal	178 <printf>
 822:	4401                	li	s0,0
 824:	4481                	li	s1,0
 826:	49a9                	li	s3,10
 828:	20000613          	li	a2,512
 82c:	858a                	mv	a1,sp
 82e:	854a                	mv	a0,s2
 830:	817ff0ef          	jal	46 <write>
 834:	20000793          	li	a5,512
 838:	0af51263          	bne	a0,a5,8dc <test_fs_large_file+0x120>
 83c:	2405                	addw	s0,s0,1
 83e:	2004849b          	addw	s1,s1,512
 842:	ff3413e3          	bne	s0,s3,828 <test_fs_large_file+0x6c>
 846:	85a6                	mv	a1,s1
 848:	00000517          	auipc	a0,0x0
 84c:	43050513          	add	a0,a0,1072 # c78 <main+0x244>
 850:	929ff0ef          	jal	178 <printf>
 854:	854a                	mv	a0,s2
 856:	809ff0ef          	jal	5e <close>
 85a:	4581                	li	a1,0
 85c:	00000517          	auipc	a0,0x0
 860:	76c50513          	add	a0,a0,1900 # fc8 <main+0x594>
 864:	ff2ff0ef          	jal	56 <open>
 868:	892a                	mv	s2,a0
 86a:	4401                	li	s0,0
 86c:	00055a63          	bgez	a0,880 <test_fs_large_file+0xc4>
 870:	00000517          	auipc	a0,0x0
 874:	7d850513          	add	a0,a0,2008 # 1048 <main+0x614>
 878:	901ff0ef          	jal	178 <printf>
 87c:	a099                	j	8c2 <test_fs_large_file+0x106>
 87e:	9c29                	addw	s0,s0,a0
 880:	20000613          	li	a2,512
 884:	858a                	mv	a1,sp
 886:	854a                	mv	a0,s2
 888:	fb6ff0ef          	jal	3e <read>
 88c:	fea049e3          	bgtz	a0,87e <test_fs_large_file+0xc2>
 890:	06940d63          	beq	s0,s1,90a <test_fs_large_file+0x14e>
 894:	8626                	mv	a2,s1
 896:	85a2                	mv	a1,s0
 898:	00000517          	auipc	a0,0x0
 89c:	7f850513          	add	a0,a0,2040 # 1090 <main+0x65c>
 8a0:	8d9ff0ef          	jal	178 <printf>
 8a4:	854a                	mv	a0,s2
 8a6:	fb8ff0ef          	jal	5e <close>
 8aa:	00000517          	auipc	a0,0x0
 8ae:	71e50513          	add	a0,a0,1822 # fc8 <main+0x594>
 8b2:	fb4ff0ef          	jal	66 <unlink>
 8b6:	00001517          	auipc	a0,0x1
 8ba:	80250513          	add	a0,a0,-2046 # 10b8 <main+0x684>
 8be:	8bbff0ef          	jal	178 <printf>
 8c2:	22813083          	ld	ra,552(sp)
 8c6:	22013403          	ld	s0,544(sp)
 8ca:	21813483          	ld	s1,536(sp)
 8ce:	21013903          	ld	s2,528(sp)
 8d2:	20813983          	ld	s3,520(sp)
 8d6:	23010113          	add	sp,sp,560
 8da:	8082                	ret
 8dc:	85a2                	mv	a1,s0
 8de:	00000517          	auipc	a0,0x0
 8e2:	74250513          	add	a0,a0,1858 # 1020 <main+0x5ec>
 8e6:	893ff0ef          	jal	178 <printf>
 8ea:	854a                	mv	a0,s2
 8ec:	f72ff0ef          	jal	5e <close>
 8f0:	22813083          	ld	ra,552(sp)
 8f4:	22013403          	ld	s0,544(sp)
 8f8:	21813483          	ld	s1,536(sp)
 8fc:	21013903          	ld	s2,528(sp)
 900:	20813983          	ld	s3,520(sp)
 904:	23010113          	add	sp,sp,560
 908:	8082                	ret
 90a:	85a2                	mv	a1,s0
 90c:	00000517          	auipc	a0,0x0
 910:	76450513          	add	a0,a0,1892 # 1070 <main+0x63c>
 914:	865ff0ef          	jal	178 <printf>
 918:	b771                	j	8a4 <test_fs_large_file+0xe8>
 91a:	22013403          	ld	s0,544(sp)
 91e:	22813083          	ld	ra,552(sp)
 922:	21813483          	ld	s1,536(sp)
 926:	21013903          	ld	s2,528(sp)
 92a:	20813983          	ld	s3,520(sp)
 92e:	00000517          	auipc	a0,0x0
 932:	6aa50513          	add	a0,a0,1706 # fd8 <main+0x5a4>
 936:	23010113          	add	sp,sp,560
 93a:	83fff06f          	j	178 <printf>

000000000000093e <test_log_basic>:
 93e:	1141                	add	sp,sp,-16
 940:	00000517          	auipc	a0,0x0
 944:	7a050513          	add	a0,a0,1952 # 10e0 <main+0x6ac>
 948:	e406                	sd	ra,8(sp)
 94a:	82fff0ef          	jal	178 <printf>
 94e:	00000517          	auipc	a0,0x0
 952:	7c250513          	add	a0,a0,1986 # 1110 <main+0x6dc>
 956:	823ff0ef          	jal	178 <printf>
 95a:	f1cff0ef          	jal	76 <logstat>
 95e:	60a2                	ld	ra,8(sp)
 960:	00000517          	auipc	a0,0x0
 964:	7e050513          	add	a0,a0,2016 # 1140 <main+0x70c>
 968:	0141                	add	sp,sp,16
 96a:	80fff06f          	j	178 <printf>

000000000000096e <test_log_dump>:
 96e:	1141                	add	sp,sp,-16
 970:	00000517          	auipc	a0,0x0
 974:	7f850513          	add	a0,a0,2040 # 1168 <main+0x734>
 978:	e406                	sd	ra,8(sp)
 97a:	ffeff0ef          	jal	178 <printf>
 97e:	00001517          	auipc	a0,0x1
 982:	81250513          	add	a0,a0,-2030 # 1190 <main+0x75c>
 986:	ff2ff0ef          	jal	178 <printf>
 98a:	ee4ff0ef          	jal	6e <logdump>
 98e:	60a2                	ld	ra,8(sp)
 990:	00001517          	auipc	a0,0x1
 994:	82850513          	add	a0,a0,-2008 # 11b8 <main+0x784>
 998:	0141                	add	sp,sp,16
 99a:	fdeff06f          	j	178 <printf>

000000000000099e <test_log_with_activity>:
 99e:	1101                	add	sp,sp,-32
 9a0:	00001517          	auipc	a0,0x1
 9a4:	83850513          	add	a0,a0,-1992 # 11d8 <main+0x7a4>
 9a8:	ec06                	sd	ra,24(sp)
 9aa:	e822                	sd	s0,16(sp)
 9ac:	fccff0ef          	jal	178 <printf>
 9b0:	00001517          	auipc	a0,0x1
 9b4:	85850513          	add	a0,a0,-1960 # 1208 <main+0x7d4>
 9b8:	fc0ff0ef          	jal	178 <printf>
 9bc:	e54ff0ef          	jal	10 <fork>
 9c0:	cd05                	beqz	a0,9f8 <test_log_with_activity+0x5a>
 9c2:	0068                	add	a0,sp,12
 9c4:	e5aff0ef          	jal	1e <wait>
 9c8:	45b2                	lw	a1,12(sp)
 9ca:	00001517          	auipc	a0,0x1
 9ce:	88e50513          	add	a0,a0,-1906 # 1258 <main+0x824>
 9d2:	fa6ff0ef          	jal	178 <printf>
 9d6:	00001517          	auipc	a0,0x1
 9da:	8aa50513          	add	a0,a0,-1878 # 1280 <main+0x84c>
 9de:	f9aff0ef          	jal	178 <printf>
 9e2:	e94ff0ef          	jal	76 <logstat>
 9e6:	6442                	ld	s0,16(sp)
 9e8:	60e2                	ld	ra,24(sp)
 9ea:	00001517          	auipc	a0,0x1
 9ee:	8be50513          	add	a0,a0,-1858 # 12a8 <main+0x874>
 9f2:	6105                	add	sp,sp,32
 9f4:	f84ff06f          	j	178 <printf>
 9f8:	20200593          	li	a1,514
 9fc:	00001517          	auipc	a0,0x1
 a00:	84c50513          	add	a0,a0,-1972 # 1248 <main+0x814>
 a04:	e52ff0ef          	jal	56 <open>
 a08:	842a                	mv	s0,a0
 a0a:	02054263          	bltz	a0,a2e <test_log_with_activity+0x90>
 a0e:	4611                	li	a2,4
 a10:	00001597          	auipc	a1,0x1
 a14:	84058593          	add	a1,a1,-1984 # 1250 <main+0x81c>
 a18:	e2eff0ef          	jal	46 <write>
 a1c:	8522                	mv	a0,s0
 a1e:	e40ff0ef          	jal	5e <close>
 a22:	00001517          	auipc	a0,0x1
 a26:	82650513          	add	a0,a0,-2010 # 1248 <main+0x814>
 a2a:	e3cff0ef          	jal	66 <unlink>
 a2e:	4501                	li	a0,0
 a30:	de8ff0ef          	jal	18 <exit>

0000000000000a34 <main>:
 a34:	1141                	add	sp,sp,-16
 a36:	00001517          	auipc	a0,0x1
 a3a:	8a250513          	add	a0,a0,-1886 # 12d8 <main+0x8a4>
 a3e:	e406                	sd	ra,8(sp)
 a40:	f38ff0ef          	jal	178 <printf>
 a44:	00001517          	auipc	a0,0x1
 a48:	8d450513          	add	a0,a0,-1836 # 1318 <main+0x8e4>
 a4c:	f2cff0ef          	jal	178 <printf>
 a50:	00001517          	auipc	a0,0x1
 a54:	88850513          	add	a0,a0,-1912 # 12d8 <main+0x8a4>
 a58:	f20ff0ef          	jal	178 <printf>
 a5c:	00001517          	auipc	a0,0x1
 a60:	8ec50513          	add	a0,a0,-1812 # 1348 <main+0x914>
 a64:	f14ff0ef          	jal	178 <printf>
 a68:	00001517          	auipc	a0,0x1
 a6c:	92050513          	add	a0,a0,-1760 # 1388 <main+0x954>
 a70:	f08ff0ef          	jal	178 <printf>
 a74:	00001517          	auipc	a0,0x1
 a78:	95450513          	add	a0,a0,-1708 # 13c8 <main+0x994>
 a7c:	efcff0ef          	jal	178 <printf>
 a80:	8edff0ef          	jal	36c <test_fs_integrity>
 a84:	ae3ff0ef          	jal	566 <test_fs_concurrent>
 a88:	b9dff0ef          	jal	624 <test_fs_multiple_files>
 a8c:	d31ff0ef          	jal	7bc <test_fs_large_file>
 a90:	00001517          	auipc	a0,0x1
 a94:	8b850513          	add	a0,a0,-1864 # 1348 <main+0x914>
 a98:	ee0ff0ef          	jal	178 <printf>
 a9c:	00001517          	auipc	a0,0x1
 aa0:	96c50513          	add	a0,a0,-1684 # 1408 <main+0x9d4>
 aa4:	ed4ff0ef          	jal	178 <printf>
 aa8:	00001517          	auipc	a0,0x1
 aac:	92050513          	add	a0,a0,-1760 # 13c8 <main+0x994>
 ab0:	ec8ff0ef          	jal	178 <printf>
 ab4:	e8bff0ef          	jal	93e <test_log_basic>
 ab8:	eb7ff0ef          	jal	96e <test_log_dump>
 abc:	ee3ff0ef          	jal	99e <test_log_with_activity>
 ac0:	00001517          	auipc	a0,0x1
 ac4:	98850513          	add	a0,a0,-1656 # 1448 <main+0xa14>
 ac8:	eb0ff0ef          	jal	178 <printf>
 acc:	00001517          	auipc	a0,0x1
 ad0:	9bc50513          	add	a0,a0,-1604 # 1488 <main+0xa54>
 ad4:	ea4ff0ef          	jal	178 <printf>
 ad8:	00001517          	auipc	a0,0x1
 adc:	80050513          	add	a0,a0,-2048 # 12d8 <main+0x8a4>
 ae0:	e98ff0ef          	jal	178 <printf>
 ae4:	00001517          	auipc	a0,0x1
 ae8:	9dc50513          	add	a0,a0,-1572 # 14c0 <main+0xa8c>
 aec:	e8cff0ef          	jal	178 <printf>
 af0:	00001517          	auipc	a0,0x1
 af4:	9f850513          	add	a0,a0,-1544 # 14e8 <main+0xab4>
 af8:	e80ff0ef          	jal	178 <printf>
 afc:	00001517          	auipc	a0,0x1
 b00:	a1c50513          	add	a0,a0,-1508 # 1518 <main+0xae4>
 b04:	e74ff0ef          	jal	178 <printf>
 b08:	00001517          	auipc	a0,0x1
 b0c:	a3850513          	add	a0,a0,-1480 # 1540 <main+0xb0c>
 b10:	e68ff0ef          	jal	178 <printf>
 b14:	00001517          	auipc	a0,0x1
 b18:	a5450513          	add	a0,a0,-1452 # 1568 <main+0xb34>
 b1c:	e5cff0ef          	jal	178 <printf>
 b20:	00001517          	auipc	a0,0x1
 b24:	a6850513          	add	a0,a0,-1432 # 1588 <main+0xb54>
 b28:	e50ff0ef          	jal	178 <printf>
 b2c:	00001517          	auipc	a0,0x1
 b30:	a8450513          	add	a0,a0,-1404 # 15b0 <main+0xb7c>
 b34:	e44ff0ef          	jal	178 <printf>
 b38:	00001517          	auipc	a0,0x1
 b3c:	a9850513          	add	a0,a0,-1384 # 15d0 <main+0xb9c>
 b40:	e38ff0ef          	jal	178 <printf>
 b44:	00001517          	auipc	a0,0x1
 b48:	aac50513          	add	a0,a0,-1364 # 15f0 <main+0xbbc>
 b4c:	e2cff0ef          	jal	178 <printf>
 b50:	00001517          	auipc	a0,0x1
 b54:	8f850513          	add	a0,a0,-1800 # 1448 <main+0xa14>
 b58:	e20ff0ef          	jal	178 <printf>
 b5c:	4501                	li	a0,0
 b5e:	cbaff0ef          	jal	18 <exit>
