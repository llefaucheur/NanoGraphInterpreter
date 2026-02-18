/* ----------------------------------------------------------------------
 * Project:      NanoGraph
 * Title:        nanograph_io.c
 * Description:  callback used after data moves at the boundary of the graph
 *
 * $Date:        15 February 2023
 * $Revision:    V0.0.1
 * -------------------------------------------------------------------- */
/*
 * Copyright (C) 2010-2023 ARM Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
* 
 */

#ifdef __cplusplus
 extern "C" {
#endif

#include <stdint.h>


#include "../../NanoGraphStore/nanograph_store_common_const.h"
#include "../../NanoGraphStore/nanograph_store_common_types.h"
#include "nanograph_const.h"      
#include "nanograph_types.h"  
#include "nanograph_extern.h"

/*----------------------------------------------------------------------------
    convert a physical address to a portable multiprocessor address (software MMU)
 *----------------------------------------------------------------------------*/
static uint32_t lin2pack (intptr_t buffer, uint8_t ** long_offset)
{
    intptr_t distance;
    uint32_t ret;
    uint8_t i;

    /* packed address range is [ long_offset[IDX]  +/- 8MB ]*/
#define MAX_PACK_ADDR_RANGE (((1<<(SIZE_FMT0_MSB - SIZE_FMT0_LSB+1))-1))

    /* find the base offset */
    for (distance = i = 0; i < (uint8_t)MAX_PROC_MEMBANK; i++)
    {
        distance = buffer - (intptr_t)(long_offset[i]);
        if (ABS(distance) < MAX_PACK_ADDR_RANGE) 
        {   break;
        }
    }

    ret = 0;
    ST(ret, DATAOFF_ARCW0, i);              // base index (software MMU)
    ST(ret, SIGNED_SIZE_FMT0, distance);    // signed offset from this address
    return ret;
}


/**
  @brief         data transfer acknowledge
  @param[in]     command    operation to do
  @param[in]     ptr1       1st data pointer 
  @param[in/out] ptr2       2nd data pointer 
  @param[in]     data3      3rd parameter integer
  @return        none

  @par           NanoGraph_io do the data moves with arc descriptor update
                 or simple assign the base address of the ring buffer to the data (no data move)
                 check there is no flow error

                 RX case : "I prepared for you a buffer of data, copy the data or use it directly 
                 from this place, and for this amount of bytes". The address can change from last 
                 callback in case the device driver is commander and using a ping-pong buffer protocol.

        Trigger is "give me data to arc[w], and raise the flag" 
        Upon RX ISR (*,n) : check overflow, then either
            set arc base address to * parameter, and release the flag
            or copy to arc, increment W, 
                if arc(data-received) > consumer frame,  then reset the flag


                 TX case : IO is commander case : "I have completed the last transfer, you can fill 
                 this buffer for the next transfer". IO is slave case : "I have completed the 
                 transfer of this buffer you told me to move out using io_start() with this amount 
                 of bytes, you can reset the flag telling the transfer is on-going". 

        Trigger is "new data to send from arc[r], raise the flag
        Upon TX ISR (*,n) : check overflow, flag-reset happened, then either
            set the arc base address to * parameter, n = frame size, src=dst, reset the flag
            or copy arc[r], n data, R=R+n, if arc(available data) < Frame TX then reset the flag
  @remark
 */

void NanoGraph_io_ack (uint8_t graph_hwio_idx, void *data, uintptr_t size)
{
    extern uintptr_t all_ptr_instances[];
    extern nanograph_instance_t* platform_io_callback_parameter;
 
    nanograph_instance_t* S = platform_io_callback_parameter;

    uint32_t *arc;
    uint32_t* pio_hw_control;
    uint32_t* pio_sw_control;
    uint8_t *long_base;
    uintptr_t addr;
    uint8_t *src;
    uint8_t *dst;

    uintptr_t read;
    uintptr_t write;
    uintptr_t fifosize;
    uint8_t graph_io_idx;
    uint8_t instance_idx;
    uint8_t ongoing_mask, ongoing_idx;
    uint8_t cache_flush;


    /* read the HW IO detail from the graph using the default instance pointer S */
    pio_hw_control = &(S->pio_hw[graph_hwio_idx * TRANSLATE_PLATFORM_HWIO_AL_IDX_SIZE_W32]);
    graph_io_idx = (uint8_t)RD(*pio_hw_control, IDX_TO_NANOGRAPH_HWIO_CONTROL);         /* IO SW index */
    pio_sw_control = &(S->pio_graph[graph_io_idx * NANOGRAPH_IOFMT_SIZE_W32]);

    /* instance affinity index is in the HW IO descriptor */
    instance_idx = (uint8_t)RD(*pio_hw_control, FWIOIDX_IOFMT0);

    /* read the SW IO detail from the graph using the default instance pointer S */

    /* read the table of all the instances to switch to the right context */
    S = (nanograph_instance_t*)all_ptr_instances[instance_idx];

    arc = S->all_arcs;
    arc = &(arc[(int)SIZEOF_ARCDESC_W32 * (int)RD(*pio_sw_control, IOARCID_IOFMT0)]);  /* FIFO/arc descriptor */
    pack2lin(&addr, arc[BASE_ARCW0], S->long_offset); 
    long_base = (uint8_t*)addr;                                                     /* FIFO base address of the buffer */
    cache_flush = RD(arc[BASE_ARCW0], MPFLUSH_ARCW0);

    
    if (cache_flush)
    {   // INVALIDATE_BUFFER_1LINE((arc[BASE_ARCW0]));                                 /* reload the cache for descriptors */
        // INVALIDATE_BUFFER_1LINE((arc[SIZE_ARCW1]));
    }

    fifosize = RD(arc[SIZE_ARCW1], BUFF_SIZE_ARCW1);                                /* FIFO size */
    read = RD(arc[RD_ARCW2], READ_ARCW2);
    write = RD(arc[WR_ARCW3], WRITE_ARCW3);
    ongoing_idx = graph_io_idx / 8;
    ongoing_mask = (uint8_t)~(1 << (graph_io_idx - ongoing_idx * 8));

    /*  test RX/TX  */
    if (0 == TEST_BIT(*pio_sw_control, RX0TX1_IOFMT0_LSB))
    {   /* -----------------------------------------------------------------------------------
            RX: from the graph point of view
           ---------------------------------------------------------------------------------- */

        /* invalidate/reload the cache for buffers used with DMA and multiprocessing */
        if (cache_flush)
        {   INVALIDATE_BUFFER_RANGE(data, size);    
        }

        if (IO_COMMAND_SET_BUFFER != RD(*pio_sw_control, SET0COPY1_IOFMT0))
        {               
            /* IO_COMMAND_DATA_COPY : reset the ONGOING flag when enough small 
                sub-frames have been received
            */  
            if (fifosize - write < size)   /* free area too small => overflow */
            {   /* overflow issue */
                //platform_al(PLATFORM_ERROR, 0,0,0);
                /* TODO : implement the flow management desired for "flow_error" */
                //flow_error = (uint8_t) RD(arc[RD_ARCW2], OVERFLRD_ARCW2);
                dst = 0;
                size = 0; // fifosize - write;
            }
            else
            {   uint32_t producer_frame_size, i;

                /* only one node can read the write-index at a time : no collision is possible */
                src = data;
                dst = &(long_base[write]);
                MEMCPY (dst, src, size)
                write = write + size;

                /* does the write index is already far, ask for data realignment by the consumer node */
                i = RD(arc[FMT_ARCW4],PRODUCFMT_ARCW4) * NANOGRAPH_FORMAT_SIZE_W32;
                producer_frame_size = RD(S->all_formats[i], FRAMESIZE_FMT0);

                if (write > fifosize - producer_frame_size)
                {   SET_BIT(arc[WR_ARCW3], ALIGNBLCK_ARCW3_LSB);
                }
            }
        } 
        else /* IO_COMMAND_SET_BUFFER, data holds the address of input in-place access */
        {   
            /* arc_set_base_address_to_arc */
            dst = data;
            ST(arc[BASE_ARCW0], BASEIDXOFFARCW0, lin2pack((intptr_t)data, (uint8_t **)S->long_offset));
            ST(arc[SIZE_ARCW1], BUFF_SIZE_ARCW1, size);  /* FIFO size aligned with the buffer size */
            write = size;
        }

        /* reset the data transfert flag is a frame is fully received */
        {   
            uint32_t i, consumer_frame_size;
            // uint32_t time_stamp_prod, time_stamp_cons;

            i = NANOGRAPH_FORMAT_SIZE_W32 * RD(arc[FMT_ARCW4],CONSUMFMT_ARCW4);
            consumer_frame_size = RD(S->all_formats[i], FRAMESIZE_FMT0);

            //time_stamp_prod = RD(S->all_formats[i +1], TIMSTAMP_FMT1);

            ///* check the need for time-stamp insertion */
            //i = RD(arc[FMT_ARCW4],CONSUMFMT_ARCW4) * NANOGRAPH_FORMAT_SIZE_W32;
            //time_stamp_cons = RD(S->all_formats[i +1], TIMSTAMP_FMT1);

            //if (time_stamp_prod == NO_TIMESTAMP && time_stamp_cons != NO_TIMESTAMP)
            //{   extern conv_int32_fp32_t graph_global_time_stamp;
            //    src = dst;
            //    dst = src + 4;
            //    MEMCPY (dst, src, size)
            //    write = write + 4;
            //    *src = graph_global_time_stamp.u; 
            //}

            /* clear the ongoing" flag when we have enough data */
            if (write - read >= consumer_frame_size)
            {   S->ongoing_async_IO[ongoing_idx] &= ongoing_mask;
            }
        }

        ST(arc[WR_ARCW3], WRITE_ARCW3, write);     /* finaly update the write index */
        if (cache_flush)
        {   //CLEAN_BUFFER_1LINE(&(arcpt[WR_ARCW3]));    /* MP synchronization */
        }
    }
    else 
    {   /* -----------------------------------------------------------------------------------
            TX: from the graph point of view
           ---------------------------------------------------------------------------------- */
        if (IO_COMMAND_SET_BUFFER != RD(*pio_sw_control, SET0COPY1_IOFMT0))
        {     
           /* IO_COMMAND_DATA_COPY : reset the ONGOING flag when the remaining data to transmit 
                is small, and below the transmitter frame size
            */
            if (write - read < size)   /* data available for TX is too small => underflow */
            {   /* underflow issue */
                //platform_al(PLATFORM_ERROR, 0,0,0);
                /* TODO : implement the flow management desired for "flow_error" */
                //flow_error = (uint8_t) RD(arc[RD_ARCW2], OVERFLRD_ARCW2);
                size = 0; // write - read;
            }

            /* only one node can read the write-index at a time : no collision is possible */
            src = &(long_base[read]);
            dst = data;
            MEMCPY (dst, src, size)
            read = read + size;
            ST(arc[RD_ARCW2], READ_ARCW2, read);   /* update the read index */

            /* check need for alignement */
            if (TEST_BIT (arc[WR_ARCW3], ALIGNBLCK_ARCW3_LSB))
            {   src = &(long_base[read]);
                dst =  long_base;
                MEMCPY (dst, src, (uint32_t)(write-read))

                /* update the indexes Read=0, Write=dataLength, then clear the flag */
                ST(arc[RD_ARCW2], READ_ARCW2, 0);
                ST(arc[WR_ARCW3], WRITE_ARCW3, write-read);
                CLEAR_BIT(arc[WR_ARCW3], ALIGNBLCK_ARCW3_LSB);
                if (cache_flush)
                {   CLEAN_BUFFER_RANGE(dst, write - read);  /* MP synchronization */
                    //CLEAN_BUFFER_1LINE(&(arcpt[WR_ARCW3]));
                }
            }

            /* reset the data transfert flag if no frame is ready for transmit */
            {   uint32_t consumer_frame_size, i;
                i = RD(arc[FMT_ARCW4],CONSUMFMT_ARCW4) * NANOGRAPH_FORMAT_SIZE_W32;
                consumer_frame_size = RD(S->all_formats[i], FRAMESIZE_FMT0);
                if (write - read < consumer_frame_size)
                {   S->ongoing_async_IO[ongoing_idx] &= ongoing_mask;
                }
            }
        } 
        else /* IO_COMMAND_SET_BUFFER, data hold the address for the next frame to send */
        {
            /*arc_set_base_address_to_arc */
            ST(arc[BASE_ARCW0], BASEIDXOFFARCW0, lin2pack((intptr_t)data, (uint8_t **)S->long_offset));
            ST(arc[RD_ARCW2], READ_ARCW2, 0);
            ST(arc[WR_ARCW3], WRITE_ARCW3, 0);
            S->ongoing_async_IO[ongoing_idx] &= ongoing_mask;
            if (cache_flush)
            {   // CLEAN_BUFFER_1LINE(&(arcpt[WR_ARCW3]));    /* MP synchronization */
            }
        }

        /* flush the cache and the memory barriers for buffers used with DMA and multiprocessing */
        if (cache_flush)
        {   // CLEAN_BUFFER_1LINE(&(arcpt[RD_ARCW2]));    /* MP synchronization */
            // CLEAN_BUFFER_RANGE(data, size);
        }
    }
}


#ifdef __cplusplus
}
#endif
