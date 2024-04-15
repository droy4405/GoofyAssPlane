SHELL=cmd
CC=c51
COMPORT = $(shell type COMPORT.inc)
OBJS=EFM8_JDY41_Throttle.obj

EFM8_JDY41_Throttle.hex: $(OBJS)
	$(CC) $(OBJS)
	@echo Yeppie!
	
EFM8_JDY41_Throttle.obj: EFM8_JDY41_Throttle.c
	$(CC) -c EFM8_JDY41_Throttle.c

clean:
	@del $(OBJS) *.asm *.lkr *.lst *.map *.hex *.map 2> nul

LoadFlash:
	@Taskkill /IM putty.exe /F 2>NUL | wait 500
	EFM8_prog.exe -ft230 -r EFM8_JDY41_Throttle.hex
	cmd /c c:\putty.exe -serial $(COMPORT) -sercfg 115200,8,n,1,N

putty:
	@Taskkill /IM putty.exe /F 2>NUL | wait 500
	cmd /c c:\putty.exe -serial $(COMPORT) -sercfg 115200,8,n,1,N

Dummy: EFM8_JDY41_Throttle.hex EFM8_JDY41_Throttle.Map
	@echo Nothing to see here!
	
explorer:
	cmd /c start explorer .
		