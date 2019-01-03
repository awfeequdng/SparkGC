# Tiny Virtual Machine Byte Code

### Instructions
| Mnemonic | Opcode (in hex) | Other bytes ([count]: [operand labels]) | Description |
|:--------:|:---------------:|:---------------------------------------:|:-----------:|
|nop|00|none|Do noting|
|int|01|1:index|Call interrupt|
|jmp|02|8:index|Jump to an 64bit index|
|jmp_if|03|8:index|Jump to an 64bit index if top element equals to zero|
|pop|04|none|Pop top element from stack|
|push_i32|05|4:bytes|Push an 32bit integer into stack|
|push_i64|06|8:bytes|Push an 64bit integer into stack|
|push_f32|07|4:bytes|Push a 32bit floating into stack|
|push_f64|08|8:bytes|Push a 64bit floating into stack|
|add_i32|09|none|Add two element|
|add_i64|0a|none|Add two element|
|add_f32|0b|none|Add two element|
|add_f64|0c|none|Add two element|
|sub_i32|0d|none|Subtract two element|
|sub_i64|0e|none|Subtract two element|
|sub_f32|0f|none|Subtract two element|
|sub_f64|10|none|Subtract two element|
|mul_i32|11|none|Multiply two element|
|mul_i64|12|none|Multiply two element|
|mul_f32|13|none|Multiply two element|
|mul_f64|14|none|Multiply two element|
|div_i32|15|none|Divide two element|
|div_i64|16|none|Divide two element|
|div_f32|17|none|Divide two element|
|div_f64|18|none|Divide two element|
|eof|ff|none|Mark the end of the file|

### Interrupts
| Mnemonic | Opcode (in hex) | Other bytes ([count]: [operand labels]) | Description |
|:--------:|:---------------:|:---------------------------------------:|:-----------:|
|Print_i32|01|4:bytes|Print top element|
|Print_i64|02|8:bytes|Print top element|