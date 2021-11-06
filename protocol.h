#define MAX_SIZE 255


const char FLAG  = 0x7E;
const char A_1   = 0x03;
const char A_2   = 0x01;
const char SET   = 0x03;
const char UA    = 0x07;

struct linkLayer {
  char frame[MAX_SIZE];
  size_t size;
};

char get_bcc(char a, char b) {
  return a ^ b;
}