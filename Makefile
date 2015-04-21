NAME = Playbin

GCC_FLAGS = `pkg-config --cflags --libs gstreamer-1.0 gtk+-2.0`

all : ${NAME}

${NAME} : Playbin.c
	gcc Playbin.c -o ${NAME} ${GCC_FLAGS}
	
clear :
	@- rm -f ${NAME} 2> /dev/null
