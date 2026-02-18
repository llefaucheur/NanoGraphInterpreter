/* ----------------------------------------------------------------------
 * Project:      NanoGraph
 * Title:        nanograph_graph_interpreter.c
 * Description:  
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

#include "../nanograph_store_included.h"
#include "nanograph_const.h"      
#include "nanograph_types.h"  
#include "nanograph_extern.h"


/*
   Main entry point, used by the application s

   Commands :
   - reset, run, end
   - interpreter of a FIFO of commands

 */

/**
  @brief            Graph interpreter entry point 
  @param[in]        command     operation to do (reset, run, stop)
  @param[in]        instance    Graph interpreter instance pointer
  @param[in]        data        graph to process
  @return           none

  @par              reset, run and stop a graph
                      
  @remark
 */
void nanograph_interpreter (uint32_t command,  nanograph_instance_t *S, uintptr_t ptr1, uintptr_t ptr2)
{   
	switch (RD(command, COMMAND_CMD))
    {
        /* usage: NanoGraph_interpreter(NANOGRAPH_RESET, &instance,graph_input, 0); */
	    case NANOGRAPH_RESET: 
	    {   platform_init_nanograph_instance (S);
            
            nanograph_interpreter_process (S, NANOGRAPH_RESET, 0);
            break;
        }


        /* usage: NanoGraph_interpreter(NANOGRAPH_RUN, &instance,0, 0); */
	    case NANOGRAPH_RUN:   
        {
            /* are there some instance still in reset ? */
            if (RSTSTATE_DONE_SYNC != RD(S->scheduler_control, RSTSTATE_SCTRL))
            {
                extern void check_instance_left_reset(void);
                check_instance_left_reset();
            }
            else
            {
                nanograph_interpreter_process(S, NANOGRAPH_RUN, 0);
            }
            
            break;
        }   


        /* change the parameters of a node  : 
            usage: 
                1) update the table of node_offset + parameters 
                2) call nano_graph_interpreter (NANOGRAPH_SET_PARAMETER, &instance, node offset, 0); 
         */
        case NANOGRAPH_SET_PARAMETER:
	    {
            nanograph_interpreter_process(S, NANOGRAPH_SET_PARAMETER, ptr1);
            break;
        }


        /* usage: nano_graph_interpreter (NANOGRAPH_STOP, &instance, 0, 0); */
        case NANOGRAPH_STOP:
	    {
            nanograph_interpreter_process(S, NANOGRAPH_STOP, 0);
            break;
        }

        default:
            if (ptr2) break;
            break;
    }
}

/*--------------------------------------------------------------------------- */

#ifdef __cplusplus

}

#endif

