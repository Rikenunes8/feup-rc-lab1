COMMON_C = src/alarm.c src/sp_config.c src/dl_protocol.c src/state_machine.c src/aux.c
COMMON_H = src/dl_protocol.h src/macros.h src/alarm.h src/sp_config.h src/state_machine.h src/aux.h

all: emissor recetor

emissor:
	gcc -Wall -o emissor src/emissor.c ${COMMON_C}

recetor: 
	gcc -Wall -o recetor src/recetor.c ${COMMON_C}

clean:
	rm -f emissor recetor
