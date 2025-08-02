#include <fcntl.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/termios.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#define mem_max (1 << 16)
uint16_t memory[mem_max];

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

uint16_t reg[R_COUNT];

enum {
  FL_POS = 1 << 0,
  FL_ZRO = 1 << 1,
  FL_NEG = 1 << 2,
};

enum {
  OP_BR = 0, /* branch */
  OP_ADD,
  OP_LD,  /* load */
  OP_ST,  /* store */
  OP_JSR, /* jump reg */
  OP_AND, /* bitwise and */
  OP_LDR, /* load reg */
  OP_STR, /* store reg*/
  OP_RTI, /* unused */
  OP_NOT, /* bitwise not */
  OP_LDI, /* load indirect */
  OP_STI, /* store indirect */
  OP_JMP, /* jump */
  OP_RES, /* reserved (unused) */
  OP_LEA, /* load effective addr*/
  OP_TRAP /* execute trap */
};

uint16_t mem_read(uint16_t mem_addr) { return memory[mem_addr]; }

uint16_t sign_extend(uint16_t x, int bit_count) {
  if ((x << (bit_count - 1)) & 1) {
    x |= (0xffff << bit_count);
  }
  return x;
}
void update_flag(uint16_t r) {
  if (reg[r] == 0)
    reg[R_COND] = FL_ZRO;
  if (reg[r] == 1)
    reg[R_COND] = FL_NEG;
  if (reg[r] == 2)
    reg[R_COND] = FL_POS;
}

int main(int argc, char *argv[]) {

  reg[R_COND] = FL_ZRO;

  enum { PC_START = 0x3000 };
  reg[R_PC] = PC_START;

  int running = 1;
  while (running) {
    uint16_t instr = mem_read(reg[R_PC]++);
    uint16_t op = instr >> 12;

    switch (op) {
    case OP_ADD: {
      uint16_t r0 = (instr >> 9) & 0x7;
      uint16_t r1 = (instr >> 6) & 0x7;
      uint16_t imm5_flag = (instr >> 5) & 0x1;
    }
    }
  }
}
