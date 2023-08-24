/**
 * @file    alloc.c
 *
 * @brief   OS Abstraction Layer for alloc functions
 *
 * @author  Decawave Applications
 *
 * @attention Copyright (c) 2021 - 2022, Qorvo US, Inc.
 * All rights reserved
 * Redistribution and use in source and binary forms, with or without modification,
 *  are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice, this
 *  list of conditions, and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *  this list of conditions and the following disclaimer in the documentation
 *  and/or other materials provided with the distribution.
 * 3. You may only use this software, with or without any modification, with an
 *  integrated circuit developed by Qorvo US, Inc. or any of its affiliates
 *  (collectively, "Qorvo"), or any module that contains such integrated circuit.
 * 4. You may not reverse engineer, disassemble, decompile, decode, adapt, or
 *  otherwise attempt to derive or gain access to the source code to any software
 *  distributed under this license in binary or object code form, in whole or in
 *  part.
 * 5. You may not use any Qorvo name, trademarks, service marks, trade dress,
 *  logos, trade names, or other symbols or insignia identifying the source of
 *  Qorvo's products or services, or the names of any of Qorvo's developers to
 *  endorse or promote products derived from this software without specific prior
 *  written permission from Qorvo US, Inc. You must not call products derived from
 *  this software "Qorvo", you must not have "Qorvo" appear in their name, without
 *  the prior permission from Qorvo US, Inc.
 * 6. Qorvo may publish revised or new version of this license from time to time.
 *  No one other than Qorvo US, Inc. has the right to modify the terms applicable
 *  to the software provided under this license.
 * THIS SOFTWARE IS PROVIDED BY QORVO US, INC. "AS IS" AND ANY EXPRESS OR IMPLIED
 *  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. NEITHER
 *  QORVO, NOR ANY PERSON ASSOCIATED WITH QORVO MAKES ANY WARRANTY OR
 *  REPRESENTATION WITH RESPECT TO THE COMPLETENESS, SECURITY, RELIABILITY, OR
 *  ACCURACY OF THE SOFTWARE, THAT IT IS ERROR FREE OR THAT ANY DEFECTS WILL BE
 *  CORRECTED, OR THAT THE SOFTWARE WILL OTHERWISE MEET YOUR NEEDS OR EXPECTATIONS.
 * IN NO EVENT SHALL QORVO OR ANYBODY ASSOCIATED WITH QORVO BE LIABLE FOR ANY
 *  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 *  ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 *
 */

#include <stdlib.h>
#include <string.h>

#include "FreeRTOS.h"


/* Define the linked list structure.  This is used to link free blocks in order
of their memory address.
This structure came from freeRTOS sources */
typedef struct A_BLOCK_LINK
{
    struct A_BLOCK_LINK *pxNextFreeBlock; /*<< The next free block in the list. */
    size_t xBlockSize;                    /*<< The size of the free block. */
} BlockLink_t;


// Overload alloc functions to map Freertos alloc functions

void *calloc(size_t num, size_t size)
{
    void *ret = pvPortMalloc(num * size);
    if (ret == NULL)
        return NULL;
    memset(ret, 0, num * size);
    return ret;
}

void *malloc(size_t size)
{
    return pvPortMalloc(size);
}

void free(void *ptr)
{
    vPortFree(ptr);
    return;
}

struct _reent;
/*
 * The standard syscall malloc/free used in sscanf/sprintf.
 * We want them to be replaced with FreeRTOS's implementation.
 *
 * This leads that the memory allocation will be managed by FreeRTOS heap4 memory.
 * */
void *_calloc_r(struct _reent *re, size_t num, size_t size)
{
    return calloc(num, size);
}

void *_malloc_r(struct _reent *re, size_t size)
{
    return pvPortMalloc(size);
}

void _free_r(struct _reent *re, void *ptr)
{
    vPortFree(ptr);
    return;
}


void *realloc(void *ptr, size_t size)
{
    /* return address of realoc */
    void *ret_ptr = NULL;
    BlockLink_t *pxLink;
    /* Get the address of the original pointer*/
    unsigned char *pOriginal = (unsigned char *)ptr;
    size_t len;
    if ((ptr == NULL) && (size > 0))
    {
        ret_ptr = pvPortMalloc(size);
        return ret_ptr;
    }
    if ((ptr != NULL) && (size > 0))
    {
        /* The size of the structure placed at the beginning of each allocated memory
        block must by correctly byte aligned. */
        static const size_t xHeapStructSize = (sizeof(BlockLink_t) + ((size_t)(portBYTE_ALIGNMENT - 1))) & ~((size_t)portBYTE_ALIGNMENT_MASK);
        /* The size of every block is stored in a BlockLink_t structure, in the allocated block itself */
        pOriginal -= xHeapStructSize;
        pxLink = (void *)pOriginal;
        /* Allocate the new pointer */
        ret_ptr = pvPortMalloc(size);
        if (ret_ptr)
        {
            /* Get the size of the original pointer */
            len = pxLink->xBlockSize - xHeapStructSize;

            if (len > size)
                len = size;

            memcpy(ret_ptr, ptr, len);
            vPortFree(ptr);
        }
        else
        {
            /* Allocation failed; do not modify the original*/
        }
    }
    return (ret_ptr);
}

void *pvPortCalloc(size_t nelem, size_t elsize)
{
    void *ret_ptr = NULL;
    size_t len;
    len = nelem * elsize;
    ret_ptr = pvPortMalloc(len);
    if (ret_ptr != NULL)
        memset(ret_ptr, 0x00, len);
    return (ret_ptr);
}