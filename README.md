<!-- lc3 vm goes brr -->

## What  
a simple implementation of a virtual machine which emulates the LC3 Architecture and its instruction set.

it fetches -> decodes -> executes instructions like a CPU, but emulated in the form of a C programme. 

---

instructions are 16 bit long   
example of an instruction
```     0001 000 000 1 00101
        ^^^^ ^^^ ^^^ ^ ^^^^^
        |    |   |   | |
        |    |   |   | immediate value 
        |    |   |   immediate flag  
        |    |   source register 1
        |    destination register
        opcode 
```
the particular opcode and source register can vary from instruction to instruction but this is a general representation





