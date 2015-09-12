# Addressing ($i)

Memory = uint8_t[256]

i = 0 → 0
i < 31 # 8 bit Registers
i < 64 # Memory[register[i - 32]]

# Representation
Position | Element
---------+--------------
00 - 02  | Reserved       
02 - 06  | Opcode
06 - 12  | First argument
12 - 18  | Second argument
18 - 24  | Third argument

# Operations
## Arithmetic

Mnemonic         | Opcode | Operation
-----------------+--------+----------------
`add $1, $2, $3`| 0000   | $1 = $3 + $3
`sub $1, $2, $3`| 0001   | $1 = $2 - $3
`mul $1, $2, $3`| 0010   | $1 = $2 * $3
`div $1, $2, $3`| 0011   | $1 = $2 / $3


## Bitwise

Mnemonic         | Opcode | Operation
-----------------+--------+----------------
`and $1, $2, $3`| 0100   | $1 = $2 & $3
`not $1, $2, $l`| 0101   | if ($l & 1) { $1 = lnot $2 } else { $1 = not $2 }
`ior $1, $2, $3`| 0110   | $1 = $2 | $3
`xor $1, $2, $3`| 0111   | $1 = $2 ^ $3


## Branching

Mnemonic         | Opcode | Operation
-----------------+--------+----------------
`jeq $1, $2, $3`| 1000   | if ($2 eq $3) ip = $1
`jne $1, $2, $3`| 1001   | if ($2 ne $3) ip = $1
`jlt $1, $2, $3`| 1010   | if ($2 lt $3) ip = $1
`jle $1, $2, $3`| 1011   | if ($2 le $3) ip = $1
`jgt $1, $2, $3`| 1100   | if ($2 gt $3) ip = $1
`jge $1, $2, $3`| 1101   | if ($2 ge $3) ip = $1


## IO

Mnemonic         | Opcode | Operation
-----------------+--------+----------------
`out  $1, XX, XX`| 1110   | $1 → STDOUT
`in   $1, XX, XX`| 1111   | $1 ← STDIN
