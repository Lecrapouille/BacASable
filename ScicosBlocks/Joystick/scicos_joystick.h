/*
** scicos_joystick.h for H4H project in ${H4HD}/Software/Applications/Scicos/Joystick
**
** Made by Crapouille
** Mail   <lecrapouille@gmail.com>
**
** Started on  Wed Oct 17 22:22:55 2007 Crapouille
** Last update Sat Apr 19 11:36:08 2008 Crapouille
*/

#ifndef SCICOS_JOYSTICK_H
#  define SCICOS_JOYSTICK_H

#  define CALCUL_DES_SORTIES  1
#  define INITIALISATION      4
#  define TERMINAISON         5

#  include <scicos/scicos_block.h>

//#  include "../../../../Drivers/Unix/Joystick/UNIX_joystick_driver.h"
#  include "UNIX_joystick_driver.h"

void    scicos_joystick(scicos_block    *block,
                        int             flag);

#endif /* SCICOS_JOYSTICK_H */
