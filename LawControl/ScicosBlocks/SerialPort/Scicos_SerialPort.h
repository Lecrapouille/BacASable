/*
** Scicos_SerialPort.h
**
** Made by Crapouille
** Mail   <lecrapouille@gmail.com>
**
** Started on  Sun Oct 19 17:33:50 2008 Crapouille
** Last update Fri Jun 11 01:04:31 2010 Crapouille
*/

#ifndef SCICOS_SERIAL_PORT_H_
#  define SCICOS_SERIAL_PORT_H_

#  include "../Scicos_Macros.h"        /* Scicos headers */
#  include "UNIX_UART.h"               /* OSI Layer: 0 */
#  include "UNIX_File.h"               /* OSI Layer: 0 */
#  include "UNIX_Timer.h"              /* System Time */

void    serialport_scicos_printf(scicos_block *block, int flag);
void    serialport_scicos_write(scicos_block *block, int flag);
void    serialport_scicos_read(scicos_block *block, int flag);
void    serialport_scicos_init(scicos_block *block, int flag);
void    serialport_scicos_bootloader(scicos_block *block, int flag);

#endif /* SCICOS_SERIAL_PORT_H_ */
