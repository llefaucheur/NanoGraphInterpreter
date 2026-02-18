/* ----------------------------------------------------------------------
 * Project:      NanoGraph
 * Title:        platform_computer.c
 * Description:  platform-specific declarations (memory, nodes)
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
/*-----------------------------------------------------------------------*/
#define DATA_FROM_FILES 1

//#define _CRT_SECURE_NO_DEPRECATE 1
#if DATA_FROM_FILES
#include <stdio.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "../../NanoGraphStore/nanograph_store_common_const.h"
#include "../../NanoGraphStore/nanograph_store_common_types.h"
#include "nanograph_const.h"      
#include "nanograph_types.h"  
#include "nanograph_extern.h"


extern void platform_init_specific(NanoGraph_init_t *data);
extern void platform_init_io(nanograph_instance_t *S, uint32_t nhwio);

#define L S->long_offset
#define G S->graph

/*
    default and specific compute services
    the table is initialized here in  platform_init_io()
*/
//extern void nanograph_services(uint32_t command, intptr_t ptr1, intptr_t ptr2, intptr_t ptr3, intptr_t n);


/**
  @brief         Translation of physical address to packed address in 27-bits
  @param[in]     instance   Global registers of the current instance
  @param[in]     address    Address to be translated to 
  @param[in]     data3      3rd parameter integer
  @return        none

  @par           Checks which of the 8 memory bank is suitable for the address
                 Translate the offset to te base address in an integer and shift
  @remark
 */

/**
  @brief        unpack a 29-bits address to physical address
  @return       inPtr_t*   address in the format of the processor
  @param[in]    offset     table of long offsets of idx_memory_base_offset
  @param[in]    data       packed address

  @par          A graph gives the address of buffers as indexes ("packed address") in a 
                way independent of the memory mapping of the processor interpreting the graph.
                The scheduler of each nanograph instance sends a physical address to the Nodes
                and translates here the indexes to physical address using a table of offsets.
 */
void pack2lin (uintptr_t *R, uint32_t x, uint8_t ** LL)
{
    uint8_t *long_base;                                     
    uint8_t extend;                                         
    int32_t signed_base;                                    
                                                        
    long_base = (uint8_t*)(LL[RD(x,ADDR_OFFSET_FMT0)]);
    signed_base = RD(x, SIGNED_SIZE_FMT0);                  
    signed_base = signed_base << (32-SIGNED_SIZE_FMT0_MSB); 
    signed_base = signed_base >> (32-SIGNED_SIZE_FMT0_MSB); 
    extend = (uint8_t)RD(x, EXTENSION_FMT0);                
    signed_base <<= (extend << 1);                          
    *R = (uintptr_t)(long_base + signed_base);
}

uint32_t * read_graph_and_copy (nanograph_instance_t *S, uint32_t *graph, uint8_t index)
{
    uint32_t i, size, offset, *table, *graph_src, *graph_dst;
    uintptr_t addr;

    /* find the section "index" */
    offset = GRAPH_HEADER_NBWORDS + index *2;
    table = &(graph[offset]);
    size = table[SECTION_SIZE];

    if (RD(table[SECTION_ADDR], COPY_IN_RAM_FMT0) == INPLACE_ACCESS_TAG)
    {   i = RD(table[SECTION_ADDR], SIZE_EXT_OFF_FMT0);
        return & (graph[i]);
    }
    else
    {   
        pack2lin(&addr, table[SECTION_ADDR], S->long_offset);

        offset = 0;
        graph_src = &(graph[GRAPH_HEADER_POINTERS_NBWORDS]);
        graph_dst = (uint32_t *)addr;

        for (i = 0; i <= index; i++)
        {   graph_src += offset;
            offset = graph[GRAPH_HEADER_NBWORDS + 2*i +1];
        }

        /* only one "main instance" can copy the graph in RAM */
        if (GLOBAL_MAIN_INSTANCE == RD(S->scheduler_control, MAININST_SCTRL))
        {   for (i = 0; i < size; i++)
            {   graph_dst[i] = graph_src[i];
            }
        }
    }
    return graph_dst;
}

/**
  @brief            (main) demonstration
  @param[in/out]    none
  @return           int

  @par              initialize a Graph interpreter  instance : share the address of the graph and
                    and the index of this instance within the application (other processors
                    or RTOS processes are using different index). 
  @remark
 */

void platform_init_nanograph_instance(nanograph_instance_t *S)
{
    uint32_t *graph_input;
    uint32_t hwnio;
    NanoGraph_init_t platform_specific_data;

    ST(S->scheduler_control, RSTSTATE_SCTRL, RSTSTATE_START);        /* instance enters RESET state */

    platform_init_specific(&platform_specific_data);

    /* set the whoamI fields */
    ST(S->scheduler_control, PROCID_SCTRL, platform_specific_data.procID);
    ST(S->scheduler_control, ARCHID_SCTRL, platform_specific_data.archID);

    if (S->graph == 0)
    {   S->graph = graph_input = platform_specific_data.graph; // the graph comes from the platform
    }
    else
    {   graph_input = S->graph;  // in the case when the graph comes from the application 
    }

    /* either in RAM or Flash section */
        
    S->long_offset = (uint8_t **)platform_specific_data.long_offset;        // software MMU
    S->platform_io = (const p_io_function_ctrl *)platform_specific_data.platform_io;
    S->node_entry_points = (const p_nanograph_node *)platform_specific_data.node_entry_points;

    S->new_parameters = platform_specific_data.new_parameters;

    S->pio_hw      = read_graph_and_copy(S, graph_input, GRAPH_PIO_HW);     // IO provided by the platform

    // number of possible HW IOs = GRAPH_PIO_HW 
    hwnio          = graph_input[GRAPH_HEADER_NBWORDS + GRAPH_PIO_HW *2 + SECTION_SIZE] / TRANSLATE_PLATFORM_HWIO_AL_IDX_SIZE_W32; 
    S->nb_graph_io = (uint8_t)graph_input[GRAPH_HEADER_NBWORDS + GRAPH_PIO_GRAPH *2 + SECTION_SIZE] / NANOGRAPH_IOFMT_SIZE_W32; // number of graph IOs
    S->pio_graph   = read_graph_and_copy(S, graph_input, GRAPH_PIO_GRAPH);  // IO used in this graph
    S->script      = read_graph_and_copy(S, graph_input, GRAPH_SCRIPTS);      
    S->linked_list = read_graph_and_copy(S, graph_input, GRAPH_LINKED_LIST);       
    S->all_formats = read_graph_and_copy(S, graph_input, GRAPH_FORMATS);         
    S->all_arcs    = read_graph_and_copy(S, graph_input, GRAPH_ARCS);               
  
    ST(S->link_offset, NODE_LINK_W32OFF, 0);      /* reset the read index in the linked list */

    /* the iomask of each instance is used to know who initializes which IO, one processor per I/O */
    platform_init_io(S, hwnio);

    /* reset done */
    ST(S->scheduler_control, RSTSTATE_SCTRL, RSTSTATE_DONE);
}

 


/*----------------------------------------------------------------------------
    convert a physical address to a portable multiprocessor address 
 *----------------------------------------------------------------------------*/
static uint32_t lin2pack (intptr_t buffer, uint8_t ** long_offset)
{
    intptr_t distance;
    uint32_t ret;
    uint8_t i;

    /* packed address range is [ long_offset[IDX]  +/- 8MB ]*/
#define MAX_PACK_ADDR_RANGE (((1<<(SIZE_FMT0_MSB - SIZE_FMT0_LSB+1))-1))

    /* find the base offset */
    distance = 0;

    for (i = 0; i < (uint8_t)MAX_PROC_MEMBANK; i++)
    {
        distance = buffer - (intptr_t)(long_offset[i]);
        if (ABS(distance) < MAX_PACK_ADDR_RANGE) 
        {   break;
        }
    }

    ret = 0;
    ST(ret, SIGNED_SIZE_FMT0, distance);
    ST(ret, DATAOFF_ARCW0, i);

    return ret;
}


/**
  @brief        Initialization and start of the IOs 
  @param[in]    instance   global data of this instance
  @return       none

  @par          Read the IOs descriptor the instance is in charge of
                Interface to platform-specific stream controls (set, start)
  @remark
 */
void platform_init_io(nanograph_instance_t *S, uint32_t hwnio)
{
    uint16_t graph_hwio_idx;
    uint32_t *pio_control;
    uint32_t iarc; 
    uint32_t *all_arcs; 
    const p_io_function_ctrl *io_func;

    all_arcs = S->all_arcs;

    S->iomask = 0;

    for (graph_hwio_idx = 0; graph_hwio_idx < hwnio; graph_hwio_idx++)
    {
        uint32_t *arc, read_hwio_control, graph_idx;
        nanograph_xdmbuffer_t io_setting;
        uint16_t hw_io_idx;

        read_hwio_control = (S->pio_hw)[graph_hwio_idx * TRANSLATE_PLATFORM_HWIO_AL_IDX_SIZE_W32]; /* IO HW index decode */
        graph_idx = RD(read_hwio_control, IDX_TO_NANOGRAPH_HWIO_CONTROL);            /* INDEX to the graph NANOGRAPH_IO_CONTROL (graph) */
        if (graph_idx == NOT_CONNECTED_TO_GRAPH)                                  /* graph_idx = 255 <> not connected */
        {   continue;
        }
        pio_control = &(S->pio_graph[graph_idx * NANOGRAPH_IOFMT_SIZE_W32]);         /* graph IO structure */

        /* build iomask from IDX_TO_NANOGRAPH_IO_CONTROL */
        S->iomask |= ((uint64_t)1 << graph_idx); 

        /* check this interpreter instance is allowed to initialize this IO */
        if (RD(S->scheduler_control, INST_IDX_SCTRL) != RD(read_hwio_control, INST_IDX_HWIO_CONTROL))
        {   continue;
        }

        //if (RD(*pio_control, SETCALLBK_IOFMT0))
        //{
        //    /* default value settings from a callback  TODO  */
        //    //S->application_callbacks(uint32_t service_command, intptr_t io_setting, 0, 0, 0);
        //}
        //else
        {
            /* default value settings */
            io_setting.address = (intptr_t)(&(pio_control[IO_SETTING_OFFSET]));
            io_setting.size = 4 * (NANOGRAPH_IOFMT_SIZE_W32 - IO_SETTING_OFFSET);
        }
        hw_io_idx = RD(*pio_control, FWIOIDX_IOFMT0);

        /* IO stream functions */
        io_func = &(S->platform_io[hw_io_idx]);
        if (*io_func == 0) 
        {   continue;
        }

        (*io_func)(NANOGRAPH_RESET, &io_setting);

        /* 
            IO-Interface expects the buffer to be declared outside of the graph
            
            set the arc base address to the IO desired buffer address and size
        */
        if (RD(*pio_control, BUFFALLOC_IOFMT0))
        {   
            nanograph_xdmbuffer_t pt_pt;
            io_func = &(S->platform_io[RD(*pio_control, FWIOIDX_IOFMT0)]);
            (*io_func)(NANOGRAPH_SET_BUFFER, &pt_pt);

            iarc = RD(*pio_control, IOARCID_IOFMT0);
            iarc = SIZEOF_ARCDESC_W32 * iarc;
            arc = &(all_arcs[iarc]);
            ST(arc[BASE_ARCW0], BASEIDXOFFARCW0, lin2pack(pt_pt.address, (uint8_t **)S->long_offset));
            ST(arc[SIZE_ARCW1], BUFF_SIZE_ARCW1, pt_pt.size);
            ST(arc[RD_ARCW2], READ_ARCW2, 0);
            ST(arc[WR_ARCW3], WRITE_ARCW3, 0);
        }
    } 
}


/*
 * -----------------------------------------------------------------------
 */
