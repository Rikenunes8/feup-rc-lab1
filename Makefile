COMMON_C = src/alarm.c src/app.c src/data_link_aux.c src/data_link.c src/log.c src/state_machine.c

all: app

app:
	gcc -Wall -o app ${COMMON_C}

clean:
	rm -f app
