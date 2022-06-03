@ECHO OFF
::SGB compiler flags, uncomment to use but recoomment GBC flags
::lcc -o ghosty.gb spriteTest.c

::GBC compiler flags
lcc -Wm-yC0x143=0xC0h -o ghosty_color.gb spriteTest.c
ECHO ON
PAUSE