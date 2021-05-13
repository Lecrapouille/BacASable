/*
** Scicos_Macros.h
**
** Made by Crapouille
** Mail   <lecrapouille@gmail.com>
**
** Started on  Sun Oct 19 17:33:50 2008 Crapouille
** Last update Sun Oct 19 17:38:44 2008 Crapouille
*/

#ifndef SCICOS_MACROS_H_
#  define SCICOS_MACROS_H_

/* Scicos headers */
#  include <scicos/scicos_block.h>
#  include <machine.h>

/* SCICOS EVENTS */
#  define SCICOS_INIT_PHASE       4
#  define SCICOS_EVENT_PHASE      3
#  define SCICOS_LOOP_PHASE       1
#  define SCICOS_END_PHASE        5

extern int  C2F(cvstr)  __PARAMS((integer *,integer *,char *,integer *,unsigned long int));
extern void sciprint(char *fmt, ...);
extern int  cvstr_();

#endif /* SCICOS_MACROS_H_ */
