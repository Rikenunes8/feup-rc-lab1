COMMON_C = src/alarm.c src/app.c src/sp_config.c src/dl_protocol.c src/state_machine.c src/aux.c src/log.c
COMMON_H = src/dl_protocol.h src/macros.h src/alarm.h src/sp_config.h src/state_machine.h src/aux.h

all: app

app:
	gcc -Wall -o app ${COMMON_C}

clean:
	rm -f app
