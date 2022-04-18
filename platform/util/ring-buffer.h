
// =============================================================================================#=
// UTIL RING BUFFER API
// platform/util/ring-buffer.h
// =============================================================================================#=

#include <stdint.h>

// =============================================================================================#=
// API Types
// =============================================================================================#=

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
// This structure type defines a descriptor for
// a ring buffer and its current state; The client should allocate space
// for both the buffer and the descriptor and also initialize
// the descriptor before calling the first ring buffer API.
//
// NOTICE!
// The size of a ring buffer must Must MUST be a POWER of TWO!
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
typedef struct
{
    uint8_t  *buff;   // Pointer to the first slot in the ring buffer.
    uint32_t  size;   // Number of slots in the buffer MUST be a power of two.
    uint32_t  tail;   // Identifies the slot to which we ADD a byte.
    uint32_t  head;   // Identifies the slot from which we REMOVE the next byte.

} Ring_Buffer;


// =============================================================================================#=
// Public API Functions
// =============================================================================================#=

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
// Ring Buffer Accessor Helper Functions
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
// These are simple low-level accessor functions.
// All these functions assume the caller has performed
// the necessary precondition checks and critical region protections.
//
// NOTICE! The approach used here assumes the buffer size is
// a power of two; this code won't work otherwise.
// Credit: https://www.snellman.net/blog/archive/2016-12-13-ring-buffers
//
// CONCEPTS:
// A ring buffer is an ordered sequence of slots;
// The size of the ring buffer is the number of slots it contains;
// A slot may be either available or occupied;
// Bytes are written to the slot at the tail;
// Bytes are read from the slot at the head;
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
uint32_t  RB_Size( Ring_Buffer *rb );

bool      RB_Is_Empty( Ring_Buffer *rb );

bool      RB_Is_Full( Ring_Buffer *rb );

uint32_t  RB_Bytes_Available( Ring_Buffer *rb );

uint32_t  RB_Slots_Available( Ring_Buffer *rb );

void      RB_Write_Byte_To_Tail( Ring_Buffer *rb, uint8_t given_byte );

uint8_t   RB_Read_Byte_From_Head( Ring_Buffer *rb );


