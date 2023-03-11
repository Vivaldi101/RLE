@echo off

IF NOT EXIST ..\build mkdir ..\build
pushd ..\build
cl -DSTD_C_ENTRY -MT -nologo -Gm- -GR- -EHa- -O2 -Oi -WX -Zi -FC -W4 -wd4101 -wd4706 -wd4201 -wd4100 -wd4189 ../code/main.cpp /link -incremental:no -opt:ref user32.lib gdi32.lib winmm.lib /out:rle.exe
popd



