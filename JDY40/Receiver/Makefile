SHELL=cmd
CC=gcc
COMPORT = $(shell type COMPORT.inc)
OBJS=EFM8_JDY40_Receiver.obj
	
EFM8_JDY40_Receiver.hex: $(OBJS)
	$(CC) $(OBJS)
	@echo Done!
	
EFM8_JDY40_Receiver.obj: EFM8_JDY40_Receiver.c
	$(CC) -c EFM8_JDY40_Receiver.c

clean:
	@del $(OBJS) *.asm *.lkr *.lst *.map *.hex *.map 2> nul

LoadFlash:
	@Taskkill /IM putty.exe /F 2>NUL | wait 500
	EFM8_prog.exe -ft230 -r EFM8_JDY40_Receiver.hex
	cmd /c c:\putty.exe -serial $(COMPORT) -sercfg 115200,8,n,1,N

putty:
	@Taskkill /IM putty.exe /F 2>NUL | wait 500
	cmd /c c:\putty.exe -serial $(COMPORT) -sercfg 115200,8,n,1,N

Dummy: EFM8_JDY40_Receiver.hex EFM8_JDY40_Receiver.Map
	@echo Nothing to see here!
	
explorer:
	cmd /c start explorer .
		
