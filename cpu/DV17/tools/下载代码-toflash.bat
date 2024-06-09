@echo off

set a=isd_tools_ddr2.cfg
:set a=isd_tools_ddr2_32M.cfg
:set a=isd_tools_ddr2_400mhz.cfg
:set a=isd_tools_ddr2_500mhz.cfg
:set a=isd_tools_ddr2-450MHz_20190107.cfg
:set a=isd_tools_ddr2-500a.cfg



apu_make.exe -addr 0x4000000 -infile sdram.bin  -ofile sdram.apu


isd_download.exe -cfg %a% -input uboot.boot ver.bin sdram.apu -resource res audlogo 32 -disk norflash -div 2 -dev dv17 -boot 0x3f02000 -aline 4096 -reboot -format 0 1 -wait 300


IF %ERRORLEVEL% LEQ 1 goto okay
pause
:okay
exit
