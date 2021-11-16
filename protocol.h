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
#define DISC    0x0B
#define UA      0x07
#define S_0     0x00
#define S_1     0x40
#define RR_0    0x05
#define RR_1    0x85
#define REJ_0   0x01
#define REJ_1   0x81

const char get_bcc(char a, char b) {
  return a ^ b;
}

typedef enum {
  INFORMATION,
  SUPERVISION,
  UNNUMBERED
} FrameType;


typedef struct {
  FrameType type;
  char address;
  char control;
  char bcc1;
  char data[MAX_SIZE-6];
  char bcc2;
  int size; // Size in bytes counting both flags
} Frame;

int frame_cmp(Frame *a, Frame *b) {
  if (a->size != b->size || a->type != b->type)
    return FALSE;
  if (a->address != b->address || a->control != b->control || a->bcc1 != b->bcc1) 
    return FALSE;
  if (a->type == INFORMATION && a->bcc2 != b->bcc2) {
    return FALSE;
  }
  return TRUE;
}

void set_buffer_from_frame(char* buf, Frame *frame) {
  buf[0] = FLAG;
  buf[1] = frame->address;
  buf[2] = frame->control;
  buf[3] = frame->bcc1;
  if (frame->type == INFORMATION) {
    memcpy(buf+4, frame->data, frame->size-6);
    buf[frame->size-2] = frame->bcc2;
  }
  buf[frame->size-1] = FLAG;
}
void set_frame_from_buffer(char* buf, Frame* frame) {
  frame->address = buf[1];
  frame->control = buf[2];
  frame->bcc1 = buf[3];
  if (frame->size != 5) {
    frame->type = INFORMATION;
    memcpy(frame->data, buf+4, frame->size-6);
    frame->bcc2 = buf[frame->size-2];
  }
  else {
    frame->type = UNNUMBERED; // TODO: how to decide between S and U?
  }
}

void set_command_SET(Frame *frame) {
  frame->address = A_1;
  frame->control = SET;
  frame->bcc1 = get_bcc(A_1, SET);
  frame->size = 5;
  frame->type = UNNUMBERED;
}

void set_answer_UA(Frame *frame) {
  frame->address = A_1;
  frame->control = UA;
  frame->bcc1 = get_bcc(A_1, UA);
  frame->size = 5;
  frame->type = UNNUMBERED;
}

void print_frame(Frame *frame, char *str) {
  printf("%s:\n", str);
  printf("%x:%x:%x:%x:", FLAG, frame->address, frame->control, frame->bcc1);
  if (frame->type == INFORMATION) {
    for (int i = 0; i < frame->size - 6; i++) {
      printf("%x:", frame->data[i]);
    }
    printf("%x:", frame->bcc2);
  }
  printf("%x\n", FLAG);
}


/*const char get_bcc(char a, char b);

char SET_message[MSG_SIZE] = {FLAG, A_1, SET, 0, FLAG};
char UA_message[MSG_SIZE] = {FLAG, A_1, UA, 0, FLAG};

void finish_setting_messages() {
  SET_message[3] = get_bcc(A_1, SET);
  UA_message[3] = get_bcc(A_1, UA);
}*/
/*typedef struct {
  FrameType type;
  char frame[MAX_SIZE];
  int size; // Size in bytes counting both flags
} Frame;*/

/*int frame_cmp(Frame *a, Frame *b) {
  if (a->size != b->size || a->type != b->type)
    return FALSE;
  for (int i = 0; i < a->size; i++) {
    if (a->frame[i] != b->frame[i])
      return FALSE;
  }
  return TRUE;
}*/
/*void set_buffer_from_frame(char* buf, Frame *frm) {
  memcpy(buf, frm->frame, frm->size);
}*/

/*void set_frame_from_buffer(char* buf, Frame* frm) {
  if (frm->size != MSG_SIZE)
    frm->type = INFORMATION;
  else
    frm->type = UNNUMBERED; // TODO: how to decide between S and U?
  memcpy(frm->frame, buf, MAX_SIZE);
}*/
/*void set_command_SET(Frame *frm) {
  frm->type = UNNUMBERED;
  frm->size = MSG_SIZE;
  memcpy(frm->frame, SET_message, MSG_SIZE);
}*/
/*void set_answer_UA(Frame *frm) {
  frm->type = UNNUMBERED;
  frm->size = MSG_SIZE;
  memcpy(frm->frame, UA_message, MSG_SIZE);
}*/
/*void print_frame(Frame *frm, char *str) {
  printf("%s:\n", str);
  for (int i = 0; i < frm->size; i++) {
    printf(":%x", frm->frame[i]);
  }
  printf("\n");
}*/