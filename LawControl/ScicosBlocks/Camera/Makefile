##
## Makefile for H4H project in ${H4HD}/Software/Applications/Scicos/Camera
##
## Made by Crapouille
## Login   <quadra_qepita.fr>
##
## Started on  Wed Sep 15 22:41:57 2004 Crapouille
## Last update Thu Apr  5 18:10:54 2007 Crapouille
##

-include ../Makefile.rules

LIB=libdevice_${HOSTTYPE}.a

SOURCES=			\
	camera_text.c		\
	camera_picture.c	\
	camera_firewire.c

OBJS=${SOURCES:.c=.o}

all:${LIB}

${LIB}:${OBJS}
	ar cr $(LIB) $(OBJS)
	ranlib $(LIB)

%.o: %.c
	${CC} ${CFLAGS} ${DEFINE} -c $< -o $

clean:
	rm -fr \#* .\#* *~ ._* .*~ .??* \#* *~ *.a *.o *.la *.lo *.so loader.sce Makelib *.cosf names *.bin .DS_Store .gdb_history lib $(OBJS);

distclean: clean
	rm -fr *.a

whitespace:
	for i in *.[ch] ;				\
	do						\
		emacs $$i --no-init-file --no-splash	\
		--funcall delete-trailing-whitespace	\
		--funcall save-buffer --kill ;		\
	done ;
