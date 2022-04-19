
// =============================================================================================#=
// UTIL RING BUFFER IMPLEMENTATION
// platform/util/ring-buffer.c
//
// NOTICE!
// The approach used here assumes the buffer size is a power of two;
// this code won't work otherwise.
//
// Credit: https://www.snellman.net/blog/archive/2016-12-13-ring-buffers
// =============================================================================================#=

#include "platform/util/ring-buffer.h"



// =============================================================================================#=
// Private Internal Functions
// =============================================================================================#=

static uint32_t rb_mask( Ring_Buffer *rb, uint32_t headortail )
{
    return (headortail & (rb->size - 1));
};



// =============================================================================================#=
// Public API Functions
// =============================================================================================#=

uint32_t RB_Size( Ring_Buffer *rb )
{
    return rb->size;
};

uint32_t RB_Bytes_Available( Ring_Buffer *rb )
{
    return rb->tail - rb->head;
};

uint32_t RB_Slots_Available( Ring_Buffer *rb )
{
    return rb->size - RB_Bytes_Available(rb);
};


bool RB_Is_Empty( Ring_Buffer *rb )
{
    return rb->head == rb->tail;
};

bool RB_Is_Not_Empty( Ring_Buffer *rb )
{
    return rb->head != rb->tail;
};

bool RB_Is_Full( Ring_Buffer *rb )
{
    return RB_Bytes_Available(rb) == rb->size;
};


void RB_Write_Byte_To_Tail( Ring_Buffer *rb, uint8_t given_byte )
{
    rb->buff[rb_mask(rb, rb->tail++)] = given_byte;
    return;
};

uint8_t RB_Read_Byte_From_Head( Ring_Buffer *rb )
{
    return rb->buff[rb_mask(rb, rb->head++)];
};


