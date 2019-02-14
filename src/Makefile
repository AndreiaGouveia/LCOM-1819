PROG=proj

SRCS = proj.c Mouse.c video.c timer.c keyboard.c words.c rtc.c Score.c UART.c

CPPFLAGS += -pedantic #-D __LCOM_OPTIMIZED__

DPADD += ${LIBLCF}
LDADD += -llcf

.include <minix.lcom.mk>
