# layerone2016

layerone2016 eagle and badge firmware

Needs 
  PSoC Creator  3.3 SP2 (3.3.0.9604)
  First build will generate all the generated code and so on, different versions of PSOC will need the boot hex path updated in the config
  
LayerOne2015 firmware is similar.

lights_servers is my hackpad VC20015 project for windows (see L12015 for other OS's) it sends out the UDP packets for different sized strips and can also do an audio visualiser with BASS http://www.un4seen.com/

The PSOC4 $4 dev board can be used to program the badge after the first firmware flash. The 3 pin UART connector maps directly to the PSOC5 snap off on the board, ground is closest to the middle of the CPU/ LED side of the board. If you don't want to snap it off, remove the 0R resistor and install a jumper in the PSOC4 side of the dev board. Then use the Bootloader option in Cypress Creator and not the Program option, baud is 115200K
