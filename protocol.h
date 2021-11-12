#include <stdio.h>
#include <string.h>

#define FALSE 0
#define TRUE 1
#define MAX_SIZE 255
#define TIME_OUT 3
#define MAX_RESENDS 3
#define MSG_SIZE 5


#define FLAG    0x7E
#define A_1     0x03
#define A_2     0x01
#define SET     0x03
#define UA      0x07

const char get_bcc(char a, char b);

char SET_message[MSG_SIZE] = {FLAG, A_1, SET, 0, FLAG};
char UA_message[MSG_SIZE] = {FLAG, A_1, UA, 0, FLAG};

void finish_setting_messages() {
  SET_message[3] = get_bcc(A_1, SET);
  UA_message[3] = get_bcc(A_1, UA);
}



typedef enum {
  INFORMATION,
  SUPERVISION,
  UNNUMBERED
} FrameType;

typedef struct {
  FrameType type;
  char frame[MAX_SIZE];
  int size; // Size in bytes counting both flags
} Frame;


int frame_cmp(Frame *a, Frame *b) {
  if (a->size != b->size || a->type != b->type)
    return FALSE;
  for (int i = 0; i < a->size; i++) {
    if (a->frame[i] != b->frame[i])
      return FALSE;
  }
  return TRUE;
}

const char get_bcc(char a, char b) {
  return a ^ b;
}

void set_buffer_from_frame(char* buf, Frame *frm) {
  memcpy(buf, frm->frame, frm->size);
}

void set_frame_from_buffer(char* buf, Frame* frm) {
  if (frm->size != MSG_SIZE)
    frm->type = INFORMATION;
  else
    frm->type = UNNUMBERED; // TODO: how to decide between S and U?
  memcpy(frm->frame, buf, MAX_SIZE);
}

void set_command_SET(Frame *frm) {
  frm->type = UNNUMBERED;
  frm->size = MSG_SIZE;
  memcpy(frm->frame, SET_message, MSG_SIZE);
}

void set_answer_UA(Frame *frm) {
  frm->type = UNNUMBERED;
  frm->size = MSG_SIZE;
  memcpy(frm->frame, UA_message, MSG_SIZE);
}

void print_frame(Frame *frm, char *str) {
  printf("%s:\n", str);
  for (int i = 0; i < frm->size; i++) {
    printf(":%x", frm->frame[i]);
  }
  printf("\n");
}
