<!-- lc3 vm goes brr -->

## LC3 is an architecture for vms

instructions are in 16 bit form 

example of an instruction
```        0001 000 000 1 00101
        ^^^^ ^^^ ^^^ ^ ^^^^^
        |    |   |   | |
        |    |   |   | immediate value 
        |    |   |   immediate flag  
        |    |   source register 1
        |    destination register
        opcode 
```


specific codes can be extracted by using shift operators 

