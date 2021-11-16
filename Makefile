EMISSOR_C = src/emissor.c src/alarm.c src/sp_config.c
RECETOR_C = src/recetor.c src/sp_config.c
COMMON_H = src/protocol.h src/macros.h src/alarm.h src/sp_config.h

all: emissor recetor

emissor: ${EMISSOR_C} ${COMMON_H}
	gcc -o emissor ${EMISSOR_C}

recetor: ${RECETOR_C} ${COMMON_H}
	gcc -o recetor ${RECETOR_C}

clean:
	rm -f emissor recetor