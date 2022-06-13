# SimAVR
Simulation of AVR MCU

## Overview
The objective of this project is to create an environment, running under linux,
that will provide the ability to load and execute (to a limited extent) a HEX object
file produced by the Arduino IDE (or other system capable of outputting the HEX
format).

The aim is to provide an environment that will allow the code to be disassembled and
executed instruction at a time, through subroutine calls, or until break points or
conditions are met.

Speed of simulation is not the primary goal.

## Status

Soooo .. been working on this for a while now.  This seems to have reached critical mass
now whereinteresting things are happenning.  It's also reached the point where I need to
implement some of the key devices that exist in the environment

```
SimAVR/src$ ./a.out ../Blink/Blink.ino.with_bootloader.hex *.sym *.fuse
[Information/Port/Config Change] 1:  New pin attached 0
(C)ontinue, (I)gnore, (B)reak, (F)ail or (A)bort? i
[Information/Port/Config Change] 1:  New pin attached 1
[Information/Port/Config Change] 1:  New pin attached 2
[Information/Port/Config Change] 1:  New pin attached 3
[Information/Port/Config Change] 1:  New pin attached 4
[Information/Port/Config Change] 1:  New pin attached 5
[Information/Port/Config Change] 1:  New pin attached 6
[Information/Port/Config Change] 1:  New pin attached 7
[Information/Map/Config Change] 1:  Base address shifted 32
(C)ontinue, (I)gnore, (B)reak, (F)ail or (A)bort? i
BOOT: clr r1
> d10
BOOT: clr r1
BOOT+1: in WL,MCUSR
BOOT+2: out MCUSR,r1
BOOT+3: sbrs WL,1
BOOT+4: rcall BOOT+245
BOOT+5: ldi WL,$05
BOOT+6: sts TCCR1B,WL
BOOT+8: ldi WL,$02
BOOT+9: sts UCSR0A,WL
BOOT+11: ldi WL,$18
BOOT: clr r1
>    
[Information/Coverage/Address OOR] 0:  New block number 0
(C)ontinue, (I)gnore, (B)reak, (F)ail or (A)bort? i
[Information/Coverage/Address OOR] 0:  New cons record 0
[Information/Coverage/Address OOR] 0:  New page record 63
BOOT+1: in WL,MCUSR
> 
BOOT+2: out MCUSR,r1
> 
BOOT+3: sbrs WL,1
> 
BOOT+4: rcall BOOT+245
> 
[Information/Coverage/Address OOR] 0:  New page record 8
BOOT+245: ldi WL,$00
> 
BOOT+246: rcall BOOT+223
> 
BOOT+223: ldi ZL,$60
> ?r
          r0=00	          r1=00	          r2=00	          r3=00
          r4=00	          r5=00	          r6=00	          r7=00
          r8=00	          r9=00	         r10=00	         r11=00
         r12=00	         r13=00	         r14=00	         r15=00
         r16=00	         r17=00	         r18=00	         r19=00
         r20=00	         r21=00	         r22=00	         r23=00
          WL=00	          WH=00	          XL=00	          XH=00
          YL=00	          YH=00	          ZL=00	          ZH=00
        SP=08FB	        SREG=02	        PC=3FDF	        EIND=00
        RAMD=00	        RAMX=00	        RAMY=00	        RAMZ=00
       MCUCR=00	       MCUSR=00	      BOOT=3F00	    IRQVec=0000
     fuse[0]=FF	     fuse[1]=FF	     fuse[2]=FF	     fuse[3]=FE

BOOT+223: ldi ZL,$60
> ?c
Target	Exec	Jump	Call	Data	Read	Write	Stack
0008FC	0	0	0	0	0	0	1
0008FD	0	0	0	0	0	0	1
0008FE	0	0	0	0	0	0	1
0008FF	0	0	0	0	0	0	1
003F00	1	0	0	0	0	0	0
003F01	1	0	0	0	0	0	0
003F02	1	0	0	0	0	0	0
003F03	1	0	0	0	0	0	0
003F04	1	0	0	0	0	0	0
003FDF	0	0	1	0	0	0	0
003FF5	1	0	1	0	0	0	0
003FF6	1	0	0	0	0	0	0
BOOT+223: ldi ZL,$60
> 
```

While the above remains essentailly accurate, the simulation has been improving and 
elements of the system show above are better (eg code and memory coverage can now
be reset and displayed separately).

The ability to 'execute over' an instruction (eg call or rcall) with the commands
'rs' and 'ts'.  The simulation now supports transient breakpoints to facilitate
this being a break point which is only triggered once before being automatically
removed.

I'm sure there is more.
