@echo off

set a=isd_config.cfg
:set a=isd_tools_ddr2_400mhz.cfg
:set a=isd_tools_ddr2_500mhz.cfg
:set a=isd_tools_ddr2-450MHz_20190107.cfg

echo [ ѡ������ģʽ ]
echo 3: ��Դ�ļ�+����
echo 5: ��Դ�ļ�+����+uboot
echo q: ȡ��
echo .

choice /c 35q /n /m "������ѡ�"

if %errorlevel% EQU 1 (
    set /A bfumode = 3
) else if %errorlevel% EQU 2 (
    set /A bfumode = 5
) else if %errorlevel% EQU 3 (
    exit
)

isd_download.exe -cfg %a% -input uboot.boot ver.bin sdram.apu -resource res audlogo 32 -disk file -aline 4096 -bfumode %bfumode% -dev upgrade_bfu

del upgrade\JL_AC*.bfu upgrade\*.bin

file_package.exe -file sdram.bin -dir upgrade -o package
copy /B jl_isd.bfu+package upgrade\JL_AC5X.bfu
del jl_isd.bfu package
echo.
echo �����ļ���upgradeĿ¼�£���upgradeĿ¼�µ������ļ�copy��SD���ĸ�Ŀ¼���忨�ϵ缴������
echo.
pause