target remote | openocd -f board/st_nucleo_f3.cfg -c "gdb_port pipe"
set remote hardware-breakpoint-limit 6
set remote hardware-watchpoint-limit 4
mon reset halt
load
