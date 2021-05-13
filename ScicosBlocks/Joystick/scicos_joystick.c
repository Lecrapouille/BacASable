/*
** scicos_joystick.c
**
** Made by Crapouille
** Mail   <lecrapouille@gmail.com>
**
** Started on  Wed Oct 17 22:22:55 2007 Crapouille
** Last update Sat Apr 19 11:36:05 2008 Crapouille
*/

#include "scicos_joystick.h"

static struct s_joystick joy;
static int err = 0;

void            scicos_joystick(scicos_block    *block,
                                int             flag)
{
  static int    i;

  if (err == -1)
    return ;

  switch (flag)
    {
    case INITIALISATION:
      err = joystick_open(&joy, "/dev/input/js0", 1);
      if (err != -1)
        joystick_show_infos(stderr, &joy);
      break;
    case TERMINAISON:
      joystick_close(&joy);
      break;
    case CALCUL_DES_SORTIES:
      joystick_read(&joy);
      joystick_show_states(stderr, &joy);
      /* print axis */
      for (i = 0; i < joy.num_of_axis; ++i)
        block->outptr[0][i] = joy.axis[i];
      /* print buttons */
      for (i = 0; i < joy.num_of_buttons; ++i)
        block->outptr[1][i] = joy.buttons[i];
      break;
    default: break;
    }
}
