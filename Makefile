COMMON_C = src/alarm.c src/app.c src/sp_config.c src/dl_protocol.c src/state_machine.c src/auxiliar.c src/log.c

all: app

app:
	gcc -Wall -o app ${COMMON_C}

clean:
	rm -f app
