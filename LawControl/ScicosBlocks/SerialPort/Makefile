##
## Makefile for H4H project in ${H4HD}/Software/Applications/Scicos/SerialPort
##
## Made by Crapouille
## Mail   <lecrapouille@gmail.com>
##
## Started on  Wed Oct 10 00:55:40 2007 Crapouille
## Last update Sat Apr 12 15:00:36 2008 Crapouille
##

include $(H4HD)/Makefile.libs

LIBSO=$(NAME_SCICOS_SERIAL_LINK).so
LIBA=$(NAME_SCICOS_SERIAL_LINK).a

EXT_LIBS=  \
	$(LIB_UNIX_TIMER)  \
	$(LIB_COLLECTIONS) \
	$(LIB_BLACK_BOX)   \
	$(LIB_UNIX_UART)   \
	$(LIB_UNIX_FILE)   \
	$(LIB_DATA_LINK_LAYER) \
	$(LIB_TFTP)

INC_PATH= \
	-I$(SCI)/routines/ \
	$(INC_UNIX_TIMER)  \
	$(INC_COLLECTIONS) \
	$(INC_BLACK_BOX)   \
	$(INC_UNIX_UART)   \
	$(INC_UNIX_FILE)   \
	$(INC_DATA_LINK_LAYER) \
	$(INC_TFTP)

SOURCES= \
	Scicos_SerialPort.c

include $(H4HD)/Makefile.skl

clean:
	@rm -fr loader.sce Makelib myblock.cosf names *.bin SerialPort.cosf .libs lib
	@make -f $(H4HD)/Makefile.skl clean
