Part1

The model consists of one initiator and one target. The initiator sends a series of 128-bit
(16-byte) burst transactions through the 32-bit (4-byte) socket, where each burst consists of a stream
of 4 x 32-bit words written to the same address. The top byte of each 32-bit word is disabled.
The address is aligned, i.e. divisible by 4.

Each burst should be structured as follows:

Byte  0, address = A
Byte  1, address = A+1
Byte  2, address = A+2
Byte  3, (disabled)

Byte  4, address = A
Byte  5, address = A+1
Byte  6, address = A+2
Byte  7, (disabled)

Byte  8, address = A
Byte  9, address = A+1
Byte 10, address = A+2
Byte 11, (disabled)

Byte 12, address = A
Byte 13, address = A+1
Byte 14, address = A+2
Byte 15, (disabled)

This desciption assumes you are running on a little-endian host. Otherwise, the address order
within each word will be reversed.

the data is random 

Have the target return a generic payload error response if
- the command is not a write
- the streaming width is not equal to 4
- the address is not aligned with 4-byte word boundaries
- the data length is not a multiple of 4

The target prints out the address, data, and byte enable for each 32-bit word received.

Testing error responses by sending some bad transactions.


Part2

The target prints the address of the transaction object, whether or not the transaction
has a memory manager, and if it has, the value of the reference count.

The initiator allocates transactions using the memory manager provided in file
../common/mm.h.  The initiator calls acquire() and release().
