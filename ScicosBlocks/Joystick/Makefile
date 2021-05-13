##
## Makefile for H4H project in ${H4HD}/Software/Applications/Scicos/Joystick
##
## Made by Crapouille
## Mail   <lecrapouille@gmail.com>
##
## Started on  Wed Oct 10 00:55:40 2007 Crapouille
## Last update Sat Apr 12 15:00:36 2008 Crapouille
##

include $(H4HD)/Makefile.libs

LIBSO=libScicosJoystick_$(HOSTTYPE).so

EXTLIB=	\
	$(LIB_JOYSTICK)

INC_PATH= \
	-I$(SCI)/routines/

SOURCES= \
	scicos_joystick.c

include $(H4HD)/Makefile.skl

clean:
	rm -fr loader.sce Makelib myblock.cosf names *.bin .libs lib
	make -f $(H4HD)/Makefile.skl clean
