openocd -f interface/stlink.cfg -f target/stm32f1x.cfg -c "build/f103_rc.elf verify reset exit"
