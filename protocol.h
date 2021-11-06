#define FALSE 0
#define TRUE 1
#define MAX_SIZE 255


const char FLAG  = 0x7E;
const char A_1   = 0x03;
const char A_2   = 0x01;
const char SET   = 0x03;
const char UA    = 0x07;

struct linkLayer {
  char frame[MAX_SIZE];
  int size;
};

int frame_cmp(struct linkLayer *a, struct linkLayer *b) {
  if (a->size != b->size)
    return FALSE;
  for (int i = 0; i < a->size; i++) {
    if (a->frame[i] != b->frame[i])
      return FALSE;
  }
  return TRUE;
}

char get_bcc(char a, char b) {
  return a ^ b;
}

void set_command_SET(struct linkLayer *msg) {
  msg->frame[0] = FLAG;
  msg->frame[1] = A_1;
  msg->frame[2] = SET;
  msg->frame[3] = get_bcc(A_1, SET);
  msg->frame[4] = FLAG;
  msg->size = 5;
}

void set_answer_UA(struct linkLayer *msg) {
  msg->frame[0] = FLAG;
  msg->frame[1] = A_1;
  msg->frame[2] = UA;
  msg->frame[3] = get_bcc(A_1, UA);
  msg->frame[4] = FLAG;
  msg->size = 5;
}