@echo off
set update_Folder=upgrade\
set AC57_bfu_name=%update_Folder%AC57s.bfu

echo %AC57_bfu_name%

if exist %update_Folder% (         
       del upgrade\*.bfu
    ) else (  
             
        md %update_Folder%  
    )  
packet_bfu.exe -packet dv17loader.enc jl_isd.bin -o %AC57_bfu_name%

echo AC57x >>%AC57_bfu_name%

echo.
echo 57�ӻ������ļ�(AC57s.bfu)��upgradeĿ¼�£�upgradeĿ¼�µ�AC57s.bfu�ļ��������ӻ�����
echo.

pause