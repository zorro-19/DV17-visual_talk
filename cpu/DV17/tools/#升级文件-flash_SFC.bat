@echo off

set a=isd_config.cfg
:set a=isd_tools_ddr2_400mhz.cfg
:set a=isd_tools_ddr2_500mhz.cfg
:set a=isd_tools_ddr2-450MHz_20190107.cfg


echo [ ѡ���i��ģʽ ]
echo 3: ��Դ�ļs+����
echo 5: ��Դ�ļs+����+uboot
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
echo %bfumode% 
copy sdram.bin sdram.app /y



isd_download.exe -cfg %a% -input uboot.boot ver.bin sdram.app -resource res audlogo 32 -bfumode %bfumode% -disk file -aline 4096 -dev upgrade_bfu

del upgrade\JL_AC*.bfu upgrade\*.bin

copy jl_isd.bfu upgrade\JL_ACSFC.bfu
del jl_isd.bfu 
del sdram.app
echo.
echo �����ļ���upgradeĿ¼�£���upgradeĿ¼�µ������ļ�copy��SD���ĸ�Ŀ¼���忨�ϵ缴������
echo.
pause