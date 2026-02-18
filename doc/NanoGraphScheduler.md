| Table of content |
| --------------------- |
| [How to start](#How-to-start) |
| [Graph Interpreter instance](#Graph-Interpreter-instance) |
| [Manifest Files](#Top-Manifest)                             |
| [Platform Manifest](#Platform-Manifest)      |
| [Interfaces Manifests](#IO-Manifest)                        |
| [Nodes Manifests](#Node-manifest)                           |
| [Design of Nodes](#Node-design)                             |
| [Designing a graph](#Graph-design)                          |
| [Formats and Domains](#Common-tables)                       |
| [Common Nodes](#List-of-pre-installed-nodes-(development))  |



# Graph Interpreter instance

The Graph Interpreter exposes two functions.  
One entry point is `void arm_graph_interpreter()`, and one function is used to confirm that data transfers with the outside of the graph have completed, `void arm_graph_interpreter_io_ack()`.  
Both functions can be called once the instance has been created by the platform abstraction layer using `void platform_init_stream_instance(arm_stream_instance_t *S)`.

Interpreter instance structure: 

| name of the field     | comments                                                     |
| --------------------- | ------------------------------------------------------------ |
| long_offset           | A pointer to the table of physical addresses of the memory banks (up to 64).  <br/>The table is located in the abstraction layer of each processor.  <br/>The graph does not use physical memory addresses directly, but offsets into one of these 64 memory banks, as defined in the platform manifest.  <br/>This table enables memory translation between processors without requiring an MMU. |
| linked_list           | pointer to the linked-list of nodes of the graph             |
| platform_io           | table of functions ([IO_AL_idx](#Top-Manifest)) associated to each IO stream of the platform |
| node_entry_points     | table of entry points to each node (see "TOP" manifest)      |
| application_callbacks | The application can provide a list of functions to be called from scripts.  <br/>One callback serves as an entry point for nodes generating metadata information in natural-language text, ready to be processed by specialized small language models. |
| al_services           | pointer to the function proposing services (time, script, stdlib, compute) |
| iomask                | bit-field of the allowed IOs this interpreter instance can trigger |
| scheduler_control     | Execution options of the scheduler, such as returning to the application after each node execution, after a full graph traversal, or when no data is available to process, as well as processor identification. |
| script_offsets        | pointer to scripts used as subroutines for the other scripts placed in the node "arm_stream_script" parameter section. |
| all_arcs              | A pointer to the list of arc descriptors.  <br/>These structures provide the base address, buffer size, read and write indexes, data formats of consumers and producers, and debug or trace information. |
| all_formats           | pointer to the section of the graph describing the stream formats. This section is in RAM. |
| ongoing               | pointer to a table of bytes associated to each IO ports of the graph. Each byte tells if a transfer is on-going. |

Graphical view of the memory mapping

![ ](Graph_mapping.png)

--------------------------------------

## Node memory allocation

A node can request **up to six memory banks** with configurable fields.

These fields include the memory type (static, working or scratch, static with periodic backup),  
the speed requirement (normal, fast, or critical fast),  
whether the memory is relocatable,  
whether it is used for program or data storage,  
and the size in bytes.

The size may be a fixed number of bytes or a computed value based on stream format parameters such as number of channels, sampling rate, or frame size, combined with a flexible parameter defined in the graph.

The total memory allocation size in bytes is computed as follows.

```
   A + 							   (fixed memory allocation in Bytes)
   B x frame_size(mono) of arc(i) or max(input/output arcs) or their sum +
   C x frame_size(mono) x nb_channel of arc(j) + 
   D x nb_channels of arc(k) 
      + parameter from the graph optional field "node_malloc_add" 

```

The first memory block is the node instance, followed by other blocks. This first block has the index #0.

### node_mem "index"           

This command starts the declaration of a memory block with the specified index.
Example :

```
 node_mem 0    ; starts the declaration section of memory block #0 (its instance)
```

### node_mem_alloc  "A"

This parameter specifies the fixed memory allocation value A in bytes.
Example :

```
node_mem_alloc  32    ; add 32 bytes to the current node_mem
```

### node_mem_frame_size_mono "B" "type" ("i")   

Declaration of extra memory in proportion with the mono frame size of the stream flowing through a specified arc index. 

- t = arc : tells the index of arc to consider is "i"
- t = maxin : tells to take the maximum of all input arcs
- t = maxout : the maximum of all output arcs
- t = maxall : the maximum of all arcs
- t = sumin : the sum of frame size of all input arcs
- t = sumout : the sum of frame size of all output arcs
- t = sumall : the sum of frame size of all arcs

Example :

```
node_mem_frame_size_mono 2 maxin   ; declare the 2x maximum size of all input arcs
node_mem_frame_size_mono 5.3 arc 0 ;   add 5.3x mono frame size of arc #0
```

### node_mem_frame_size "C" "type" ("j")   

Declaration of extra memory in proportion with the multichannel frame size of the stream flowing through a specified arc index. 

```
node_mem_frame_size 2 arc 0 ; declare the 2x multichannel frame size of arc #0
```

### node_mem_nbchan "D" "type" ("k")

Declaration of extra memory in proportion to the number of channel of arcs 

```
node_mem_nbchan 44 maxin ; add 44 x maximum nb of channels of input arcs
```

### node_mem_alignment "n"

Declaration of the memory Byte alignment
Example :

```
node_mem_alignement     4           ; 4 bytes to (default) ` 
```

### node_mem_type "n"

Definition of the dynamic characteristics of the memory block :

0 STATIC : memory content is preserved between two calls (default )

1 WORKING : scratch memory content is not preserved between two calls 

2 PERIODIC_BACKUP static memory to reload during a warm reboot 

3 PSEUDO_WORKING static only during the uncompleted execution state of the NODE

Example :

```
node_mem_type 3   ; memory block put in a backup memory area when possible
```

### node_mem_speed "n"

Declaration of the memory desired for the memory block.

0 for 'best effort' or 'no constraint' on speed access

1 for 'fast' memory selection when possible

2 for 'critical fast' section, to be in I/DTCM when available

Example :

```
node_mem_speed 0   ; relax speed constraint for this block
```

### node_mem_relocatable "0/1"

Declares if the pointer to this memory block is relocatable, or assigned a fixed address at reset (default, parameter = '0').
When the memory block is relocatable a command 'STREAM_UPDATE_RELOCATABLE' is used with address changes:  void (node) (command, .. ); This is done with the associated script of the node (TBD).

Example :

```
node_mem_relocatable    1   ; the address of the block can change
```

### node_mem_data0prog1 "0/1"

This command specifies whether the memory block is used for data or program access.

A value of 0 specifies data access.  
A value of 1 specifies program access.

RAM program memory segments are loaded by the node.  
If memory allocation is not possible, the address provided to the node is set to −1.

Example :

```
  node_mem_data0prog1  1 ; program memory block 
```



