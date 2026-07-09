%~dp0\ffprobe.exe -v error -show_streams -of default=noprint_wrappers=1:nokey=1 -print_format ini -i %1 > "%~dp0\myprobe.ini"
