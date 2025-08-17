#include <bits/types/struct_timeval.h>
#include <fcntl.h>
#include <iso646.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <sys/termios.h>
#include <sys/time.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>

#define MEM_MAX (1 << 16)
uint16_t memory[MEM_MAX];

enum {
  R_R0 = 0,
  R_R1,
  R_R2,
  R_R3,
  R_R4,
  R_R5,
  R_R6,
  R_R7,
  R_PC, /* prog counter */
  R_COND,
  R_COUNT
};
enum {
  trap_getc = 0x20,  // get char from kb, not echoed back to term
  trap_out = 0x21,   // output char
  trap_puts = 0x22,  // output words/string
  trap_in = 0x23,    // get char from kb,echo back to term
  trap_putsp = 0x24, // output a byte string
  trap_halt = 0x25,  // stop/halt the prog
};

uint16_t reg[R_COUNT];

enum {
  FL_POS = 1 << 0,
  FL_ZRO = 1 << 1,
  FL_NEG = 1 << 2,
};

enum {
  OP_BR = 0, /* branch */
  OP_ADD,    // x
  OP_LD,     /* load x */
  OP_ST,     /* store x */
  OP_JSR,    /* jump reg x */
  OP_AND,    /* bitwise and x */
  OP_LDR,    /* load reg x */
  OP_STR,    /* store reg x */
  OP_RTI,    /* unused */
  OP_NOT,    /* bitwise not x */
  OP_LDI,    /* load indirect x */
  OP_STI,    /* store indirect */
  OP_JMP,    /* jump */
  OP_RES,    /* reserved (unused) */
  OP_LEA,    /* load effective addr x */
  OP_TRAP    /* execute trap */
};

enum {
  MR_KBSR = 0xfe00, // tells if a key is pressed
  MR_KBDR = 0xfe02, // which key is pressed

};

struct termios term_og;

void enableRaw(void) {
  tcgetattr(STDIN_FILENO, &term_og);
  struct termios term_new = term_og;
  term_new.c_lflag &= ~ICANON & ~ECHO;
  tcsetattr(STDIN_FILENO, TCSANOW, &term_new);
}
void disbableRaw(void) { tcsetattr(STDIN_FILENO, TCSANOW, &term_og); }

uint16_t check_key(void) {
  fd_set fdr;
  FD_ZERO(&fdr);
  FD_SET(STDIN_FILENO, &fdr);

  struct timeval timeout;
  timeout.tv_sec = 0;
  timeout.tv_usec = 0;
  return select(1, &fdr, NULL, NULL, &timeout) != 0;
}

uint16_t mem_read(uint16_t addr) {
  if (addr == MR_KBSR) {
    if (check_key()) {
      memory[MR_KBSR] = 1 << 15;
      memory[MR_KBDR] = getchar();
    } else {
      memory[MR_KBSR] = 0;
    }
  }
  return memory[addr];
}

uint16_t sign_extend(uint16_t x, int bit_count) {
  if ((x >> (bit_count - 1)) & 1) {
    x |= (0xffff << bit_count);
  }
  return x;
}
void update_flag(uint16_t r) {
  if (reg[r] == 0)
    reg[R_COND] = FL_ZRO;
  else if (reg[r] >> 15)
    reg[R_COND] = FL_NEG;
  else
    reg[R_COND] = FL_POS;
}
void memwrite(uint16_t val, uint16_t addr) { memory[addr] = val; }

uint16_t swap16(uint16_t x) { return (x << 8) | (x >> 8); }

void readImgFile(FILE *fh) {
  uint16_t orig;
  fread(&orig, sizeof(orig), 1, fh);
  orig = swap16(orig);

  uint16_t memMax = MEM_MAX - orig;
  uint16_t *data = memory + orig;
  uint16_t i = fread(data, sizeof(uint16_t), memMax, fh);

  // swap the data
  while (i-- > 0) {
    *data = swap16(*data);
    ++data;
  }
}
int readImg(const char *imagePath) {
  FILE *fh = fopen(imagePath, "rb");
  if (!fh)
    return 0;

  readImgFile(fh);
  fclose(fh);
  return 1;
}
void interruptHandle(int signal) {
  disbableRaw();
  printf("\n");
  exit(-2);
}
int main(int argc, char *argv[]) {

  if (argc < 2) {
    printf("./main [img file] \n");
    exit(2);
  }
  for (int i = 1; i < argc; ++i) {
    if (!readImg(argv[i])) {
      printf("error loading img");
      exit(1);
    }
  }
  signal(SIGINT, interruptHandle);
  enableRaw();

  reg[R_COND] = FL_ZRO;

  enum { PC_START = 0x3000 };
  reg[R_PC] = PC_START;

  int running = 1;
  while (running) {
    uint16_t instr = mem_read(reg[R_PC]++);
    uint16_t op = instr >> 12;

    switch (op) {
    case OP_ADD: {
      // ADD R2 R0 R1 ; add the contents of R0 to R1 and store in R2.
      uint16_t r0 = (instr >> 9) & 0x7;
      uint16_t r1 = (instr >> 6) & 0x7;
      uint16_t imm5_flag = (instr >> 5) & 0x1;
      if (imm5_flag) {
        uint16_t imm = sign_extend(instr & 0x1f, 5);
        reg[r0] = reg[r1] + imm;
      } else {
        uint16_t r2 = (instr & 0x7);
        reg[r0] = reg[r1] + reg[r2];
      }
      update_flag(r0);
    } break;
    case OP_AND: {
      uint16_t r0 = (instr >> 9) & 0x7; // destination reg
      uint16_t r1 = (instr >> 6) & 0x7;
      uint16_t imm5_flag = (instr >> 5) & 0x1;
      if (imm5_flag) {
        uint16_t imm = sign_extend(instr & 0x1f, 5);
        reg[r0] = reg[r1] & imm;
      } else {
        uint16_t r2 = instr & 0x7;
        reg[r0] = reg[r1] & reg[r2];
      }
      update_flag(r0);
    } break;
    case OP_BR: {
      // todo:
      // explain and figure this guy out
      uint16_t pcOffset = sign_extend(instr & 0x1ff, 9); // cuz it is signed
      uint16_t condFlag = (instr >> 9) & 0x7;

      if (condFlag & reg[R_COND])
        reg[R_PC] += pcOffset;
    } break;

    case OP_JMP: {
      // idhar se udhar jane ko kehta
      uint16_t r0 = (instr >> 6) & 0x7;
      reg[R_PC] = reg[r0];
    } break;

    case OP_JSR: {
      uint16_t flag = (instr >> 11) & 1;
      reg[R_R7] = reg[R_PC];
      if (flag) {
        uint16_t offset = sign_extend(instr & 0x7ff, 11);
        reg[R_PC] += offset;
      } else {
        uint r0 = (instr >> 6) & 0x7;
        reg[R_PC] = reg[r0];
      }
    } break;
    case OP_LD: {
      uint16_t r0 = (instr >> 9) & 0x7;
      uint16_t offset = sign_extend(instr & 0x1ff, 9);
      reg[r0] = mem_read(reg[R_PC] + offset);
      update_flag(r0);
    } break;
    case OP_LDI: {
      uint16_t r0 = (instr >> 9) & 0x7;
      uint16_t offset = sign_extend(instr & 0x1ff, 9);
      reg[r0] = mem_read(mem_read(reg[R_PC] + offset));
      update_flag(r0);
    } break;
    case OP_LDR: {
      uint16_t r0 = (instr >> 9) & 0x7;
      uint16_t r1 = (instr >> 6) & 0x7;
      uint16_t offset = sign_extend(instr & 0x3f, 6);
      reg[r0] = mem_read(reg[r1] + offset);
      update_flag(r0);
    } break;
    case OP_LEA: {
      uint16_t r0 = (instr >> 9) & 0x7;
      uint16_t offset = sign_extend(instr & 0x1ff, 9);
      reg[r0] = reg[R_PC] + offset;
      update_flag(r0);
    } break;
    case OP_NOT: {
      uint16_t r0 = (instr >> 9) & 0x7;
      uint16_t r1 = (instr >> 6) & 0x7;
      reg[r0] = ~reg[r1];
      update_flag(r0);
    } break;
    case OP_ST: {
      uint16_t r0 = (instr >> 9) & 0x7;
      uint16_t offset = sign_extend(instr & 0x1ff, 9);
      memwrite(reg[r0], reg[R_PC] + offset);
    } break;
    case OP_STI: {
      uint16_t r0 = (instr >> 9) & 0x7;
      uint16_t offset = sign_extend(instr & 0x1ff, 9);
      memwrite(reg[r0], mem_read(reg[R_PC] + offset));
    } break;
    case OP_STR: {
      uint16_t r0 = (instr >> 9) & 0x7;
      uint16_t r1 = (instr >> 6) & 0x7;
      uint16_t offset = sign_extend(instr & 0x3f, 6);
      memwrite(reg[r0], reg[r1] + offset);
    } break;
    case OP_TRAP:

      reg[R_R7] = reg[R_PC];
      switch (instr & 0xff) {
      case trap_puts: {
        uint16_t *ch = memory + reg[R_R0];
        while (*ch) {
          putc((char)*ch, stdout);
          ++ch;
        }

        fflush(stdout);
      } break;
      case trap_getc: {
        reg[R_R0] = (uint16_t)getchar();
        update_flag(R_R0);

      } break;
      case trap_in: {
        printf("enter char");
        char c = getchar();
        putc(c, stdout);
        fflush(stdout);
        reg[R_R0] = (uint16_t)c;
        update_flag(R_R0);
      } break;
      case trap_out: {
        putc((char)(reg[R_R0]), stdout);
        fflush(stdout);
      } break;
      case trap_putsp: {
        uint16_t *c = memory + reg[R_R0];
        while (*c) {
          char c1 = *c & 0xff;
          putc(c1, stdout);
          char c2 = *c >> 8;
          if (c2)
            putc(c2, stdout);

          ++c;
        }
        fflush(stdout);
      } break;
      case trap_halt:
        puts("halting");
        fflush(stdout);
        running = 0;
        break;
      }
      break;
    case OP_RES:
    case OP_RTI:
    default:
      abort();
      break;
    }
  }
  disbableRaw();
}
