EMISSOR_C = src/alarm.c src/emissor.c
RECETOR_C = src/recetor.c 
COMMON_H = src/protocol.h src/macros.h src/alarm.h

all: emissor recetor

emissor: ${EMISSOR_C} ${COMMON_H}
	gcc -o emissor ${EMISSOR_C}

recetor: ${RECETOR_C} ${COMMON_H}
	gcc -o recetor ${RECETOR_C}

clean:
	rm -f emissor recetor