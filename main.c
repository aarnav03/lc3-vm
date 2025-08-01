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
  reg_r0 = 0,
  reg_r1,
  reg_r2,
  reg_r3,
  reg_r4,
  reg_r5,
  reg_r6,
  reg_r7,
  reg_progcount,
  reg_cond,
  reg_count,
};

uint16_t reg[reg_count];

enum {
  fl_pos = 1 << 0,
  fl_zr = 1 << 1,
  fl_neg = 1 << 2,
};

enum {
  op_branch = 0,
  op_add,
  op_load,
  op_store,
  op_and,
  op_loadreg,
  op_storereg,
  op_un, /* unused */
  op_not,
  op_loadi,  /* load indirect */
  op_storei, /* store indirect */
  op_jump,
  op_res,  /*reserved */
  op_lea,  /*load effective addr */
  op_trap, /*trap execution */

};
