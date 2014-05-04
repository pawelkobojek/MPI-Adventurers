CC=mpicc
C_FLAGS=-Wall  -g
L_FLAGS=-lm -lrt
TARGET=prlab3
FILES=main.o
SOURCE=main.c
HOSTFILE=hosts
n=4
k=3
mapa=./mapa

${TARGET}: ${FILES}
	${CC} -o ${TARGET} ${FILES} ${L_FLAGS}

${FILES}: ${SOURCE}
	${CC} -o ${FILES} -c ${SOURCE} ${C_FLAGS}

.PHONY: clean run

run:
	mpirun --hostfile ${HOSTFILE} ./${TARGET} ${n} ${k} ${mapa}
	
clean:
	-rm -f ${FILES} ${TARGET}
 
