/* ----------------------------------------------------------------------
 * Project:      NanoGraph
 * Title:        nanograph_const.h
 * Description:  public references for the application using NANOGRAPH
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

#include "../top_manifest_included.h"

#ifndef cNANOGRAPH_CONST_H
#define cNANOGRAPH_CONST_H

#define U(x) ((uint32_t)(x)) /* for MISRA-2012 compliance to Rule 10.4 */


/*------ Major-Minor version numbers ------*/
#define GRAPH_INTERPRETER_VERSION 0x0100u

/* ----------------------------------------------------------------
    - Graph data format :
    -------------------SHARED FLASH (RAM POSSIBLE)-----------------
    6 header control words   +   7x2 (pointers + size) = 20 words

    [0] header : size of the graph, compression used
    [1] interpreter version (TBD)
    [2] memory consumption in bank 0-3 (0xFF = +99%, 0x40 = 25%)
    [3] bank 4-7  (banks of long_offset[4-7])
    [4] bank 8-11 
    [5] bank 12-15  */

#define GRAPH_HEADER_NBWORDS 6
#define GRAPH_HEADER_POINTERS_NBWORDS 18    // (6 + 2x6)
    /* + 6 pairs of {address + size} 
        [0] PIO HW decoding table (1 word per HWIO : index to graphIO + processor affinity Byte)
        [1] PIO Graph table, NANOGRAPH_IO_CONTROL (4 words per IO for )
        [2] Scripts (when used with indexes)
        [3] Graph Linked-list of nodes
        [4] graph formats <-- start of RAM 
        [5] arc descriptors

    if RD(table[SECTION_ADDR], COPY_IN_RAM_FMT0) == INPLACE_ACCESS_TAG
    if address[bits30] == 1 then the address is an offset in the graph
    else the address is a 29bits packed address in RAM and copy is made from graph data

    */
#define GRAPH_PIO_HW        0
#define GRAPH_PIO_GRAPH     1
#define GRAPH_SCRIPTS       2
#define GRAPH_LINKED_LIST   3
#define GRAPH_FORMATS       4
#define GRAPH_ARCS          5
#define NB_HEADER_MEMORY_FIELDS (1+GRAPH_ARCS)

#define SECTION_ADDR 0          /* first word pair */
#define COPY_IN_RAM_FMT0_MSB 30u
#define COPY_IN_RAM_FMT0_LSB 30u /*  1  */
#define INPLACE_ACCESS_TAG 1    /* COPY_IN_RAM_FMT0 field */

#define SECTION_SIZE 1          /* second word of the pair = size in Bytes */
    

/* ---------------------------
    word 0 : size of the graph
*/
#define GRAPH_HEADER_SIZE 0         // size of the binary graph in words

#define      unused_HW0_MSB U(31) 
#define      unused_HW0_LSB U(25) /*  6   */
#define COMPRESSION_HW0_MSB U(24) 
#define COMPRESSION_HW0_LSB U(24) /*  2   compression scheme of the graph */
#define  GRAPH_SIZE_HW0_MSB U(23) 
#define  GRAPH_SIZE_HW0_LSB U( 0) /* 24   graph size */


/* -----------------------------
    word 1 : interpreter version
*/
#define      unused_HW1_MSB U(31) 
#define      unused_HW1_LSB U( 0) 

/* ----------------------------
    word 2,3,4,5 : memory consumption (0xFF = 100%, 0x3F = 25%)
    -------- MEMORY CONSUMPTION  
            UQ8(-1) (0xFF = 100%, 0x3F = 25%) portion of memory consumed on each 
            long_offset[MAX_PROC_MEMBANK] to let the application taking 
            a piece of the preallocated RAM area 
*/
#define BYTE_3_MSB 31u
#define BYTE_3_LSB 24u
#define BYTE_2_MSB 23u
#define BYTE_2_LSB 16u
#define BYTE_1_MSB 15u
#define BYTE_1_LSB  8u
#define BYTE_0_MSB  7u
#define BYTE_0_LSB  0u



/* max number of instances simultaneously reading the graph
   used to synchronize the RESET sequence in platform_al() 
   smaller than 1<< NBINSTAN_SCTRL */
#define MAX_NB_NANOGRAPH_INSTANCES 4u 



/* number of NODE calls in sequence */
#define MAX_NODE_REPEAT 4u

/*
 * Maximum number of IOs used dynamically by the graph 
 * This number is lower or equal to the maximum of possible IO
 *   connexions of the platform (max {FWIOIDX_IOFMT0 -> 64k IO in the hardware })
 */
#define MAX_GRAPH_NB_HW_IO  64


#define NANOGRAPH_SECONDARY_INSTANCE 0
#define GLOBAL_MAIN_INSTANCE 1u     /* instance allowed to copy the graph in RAM  */
/* 
    ----------- instance -> scheduler_control  ------------- 
*/
#define NANOGRAPH_SCHD_RET_NO_ACTION            0u  /* the decision is made by the graph */
#define NANOGRAPH_SCHD_RET_END_EACH_NODE        1u  /* return to caller after each NODE calls */
#define NANOGRAPH_SCHD_RET_END_ALL_PARSED       2u  /* return to caller once all NODE are parsed */
#define NANOGRAPH_SCHD_RET_END_NODE_NODATA      3u  /* return to caller when all NODE are starving */
                                            
#define NANOGRAPH_SCHD_NO_SCRIPT                0u  /* no script debug */
#define NANOGRAPH_SCHD_SCRIPT_LEVEL1            1u  /* script is called before each NODE called */
#define NANOGRAPH_SCHD_SCRIPT_LEVEL2            2u  /* before & after each NODE called */
#define NANOGRAPH_SCHD_SCRIPT_LEVEL3            3u  /* + at start and the end of the loop */

#define RSTSTATE_INIT           0u  /* state = 0 : instance creation */
#define RSTSTATE_START          1u  /* state = 1 : reset started */
#define RSTSTATE_DONE           2u  /* state = 2 : reset done, IO init done (NANOGRAPH_MAIN_INSTANCE), graph RAM copied (GLOBAL_MAIN_INSTANCE) */
#define RSTSTATE_DONE_SYNC      3u  /* state = 1 : reset completed for all instances */

#define    INST_ID_SCTRL_MSB U(31)  /*  from [A]pp [P]latform [S} scheduler */
#define     WHOAMI_SCTRL_MSB U(31)
#define   PRIORITY_SCTRL_MSB U(31)  /*   different RTOS instances*/
#define   PRIORITY_SCTRL_LSB U(30)  /* 2 [0..3] up to 4 instances per processors, 0=main instance at boot */
#define     PROCID_SCTRL_MSB U(29)  
#define     PROCID_SCTRL_LSB U(27)  /* 3 processor index [1..7] for this architecture 0="commander processor" */  
#define     ARCHID_SCTRL_MSB U(26)     
#define     ARCHID_SCTRL_LSB U(24)  /* 3 [1..7] processor architectures 1="commander processor architecture" */
#define     WHOAMI_SCTRL_LSB U(24)  /*   whoami used to lock a NODE to specific processor or architecture */
#define    INST_ID_SCTRL_LSB U(24)  /*   8 bits identification for locks */
#define     U0_____SCTRL_MSB U(23)     
#define     U0_____SCTRL_LSB U(17)  /* 7   */   
#define  CLEARSWAP_SCTRL_MSB U(16)     
#define  CLEARSWAP_SCTRL_LSB U(16)  /* 1 one memory bank is using arc memory  */   
#define   RSTSTATE_SCTRL_MSB U(15)  /*   0=INIT 1=reset start 2=reset done 3=SYNC all reset done */
#define   RSTSTATE_SCTRL_LSB U(14)  /* 2 set to wait the main instance  */ 
#define   INST_IDX_SCTRL_MSB U(13)     
#define   INST_IDX_SCTRL_LSB U( 9)  /* 5 up to 32 graph interpreter instances */   
#define   MAININST_SCTRL_MSB U( 8)  /*    */   
#define   MAININST_SCTRL_LSB U( 8)  /* 1 0 slave instance 1 main instance allowed to copy in RAM (LSB bit) */
#define   NODEEXEC_SCTRL_MSB U( 7)     
#define   NODEEXEC_SCTRL_LSB U( 7)  /* 1 (working bit) node execution flag start=1, done=0 */
#define   ENDLLIST_SCTRL_MSB U( 6)     
#define   ENDLLIST_SCTRL_LSB U( 6)  /* 1 (working bit) endLinkedList detected */
#define   STILDATA_SCTRL_MSB U( 5)     
#define   STILDATA_SCTRL_LSB U( 5)  /* 1 (working bit) still some_components_have_data to process */
#define       BOOT_SCTRL_MSB U( 4)     
#define       BOOT_SCTRL_LSB U( 4)  /* 1 cold0/warm1 boot : Reset + restore memory banks from retention */
#define     SCRIPT_SCTRL_MSB U( 3)     
#define     SCRIPT_SCTRL_LSB U( 2)  /* 2 script call options bit-field (before/after SWC/loop/full) */
#define     RETURN_SCTRL_MSB U( 1)     
#define     RETURN_SCTRL_LSB U( 0)  /* 2 return options (each SWC, each parse, once starving, copy of RETURN_SCTRL_GR3 */

#define SIGNATUREIDX_MSB U(31)
#define SIGNATUREIDX_LSB U(31-(INST_IDX_SCTRL_MSB- INST_IDX_SCTRL_LSB-1))
#define SIGNATUREPAT_MSB U(23)
#define SIGNATUREPAT_LSB U(0)
#define   PACK_NANOGRAPH_PARAM(I,P,M,B,S,R) ( \
            ((I)<<INST_IDX_SCTRL_LSB) |   \
            ((P)<<PRIORITY_SCTRL_LSB) |   \
            ((M)<<MAININST_SCTRL_LSB) |   \
            ((B)<<    BOOT_SCTRL_LSB) |   \
            ((S)<<  SCRIPT_SCTRL_LSB) |   \
            ((R)<<  RETURN_SCTRL_LSB) )

/* ----------- instance -> link_offset  ------------- */
/* identification "whoami", next NODE to run*/
#define u____LINK_W32OFF_MSB U(31)   
#define u____LINK_W32OFF_LSB U(22) /* 10   unused*/ 
#define NODE_LINK_W32OFF_MSB U(21)  
#define NODE_LINK_W32OFF_LSB U( 0) /* 22   see LINKEDLISTSZW32_GR2, offset in words to the NEXT NODE to be executed */  



/* ----------------------------------------------------------------------------------------------------------------
    NANOGRAPH_IO_DOMAIN (s)    => nanograph_format + nanograph_io_control

    enum nanograph_io_domain : list of stream "domains" categories, max 15 (DOMAIN_FMT1_LSB/ IO_DOMAIN_IOFMT0_LSB) @@@@@
    each stream domain instance is controled by 3 functions and presets
    domain have common bitfields for settings (see example platform_audio_out_bit_fields[]).
*/
#define IO_DOMAIN_GENERAL                  0u /* (a)synchronous sensor + rescaling, electrical, chemical, color, .. remote data, compressed streams, JSON, SensorThings*/
#define IO_DOMAIN_AUDIO_IN                 1u /* microphone, line-in, I2S, PDM RX */
#define IO_DOMAIN_AUDIO_OUT                2u /* line-out, earphone / speaker, PDM TX, I2S, */
#define IO_DOMAIN_GPIO                     3u /* generic digital IO, control of relay, timer ticks */
#define IO_DOMAIN_MOTION                   4u /* accelerometer, combined or not with pressure and gyroscope */
#define IO_DOMAIN_2D_IN                    5u /* camera sensor */
#define IO_DOMAIN_2D_OUT                   6u /* display, led matrix, */
#define IO_DOMAIN_ANALOG_IN                7u /* analog sensor with aging/sensitivity/THR control, example : light, pressure, proximity, humidity, color, voltage */
#define IO_DOMAIN_ANALOG_OUT               8u /* D/A, position piezzo, PWM converter  */
#define IO_DOMAIN_USER_INTERFACE           9u /* button, slider, rotary button, LED, digits, display */
#define IO_DOMAIN_PLATFORM_6              10u                              
#define IO_DOMAIN_PLATFORM_5              11u                              
#define IO_DOMAIN_PLATFORM_4              12u                              
#define IO_DOMAIN_PLATFORM_3              13u
#define IO_DOMAIN_PLATFORM_2              14u /* platform-specific #2, decoded with callbacks */
#define IO_DOMAIN_PLATFORM_1              15u /* platform-specific #1, decoded with callbacks */
#define IO_DOMAIN_MAX_NB_DOMAINS          16u


/* ==========================================================================================

    IO_DOMAIN physical types and tuning : used to insert extra conversion nodes 
                                          during the graph compilation 

   ==========================================================================================
*/

/* IO_DOMAIN_GENERAL           : subtypes and tuning  SUBTYPE_FMT1  */
    #define NANOGRAPH_SUBT_GENERAL            0u
    #define NANOGRAPH_SUBT_GENERAL_COMP195X   1u /* compressed byte stream following RFC1950 / RFC1951 ("deflate") */
    #define NANOGRAPH_SUBT_GENERAL_DPCM       2u /* compressed byte stream */
    #define NANOGRAPH_SUBT_GENERAL_JSON       3u /* JSON */
    #define NANOGRAPH_SUBT_GENERAL_XFORMAT    4u /* SensorThings MultiDatastream extension */

/* IO_DOMAIN_AUDIO_IN          : subtypes and tuning  SUBTYPE_FMT1 */
    #define NANOGRAPH_SUBT_AUDIO_IN           0u  /* no subtype_units : integer/ADC format  */

/* IO_DOMAIN_AUDIO_OUT         : subtypes and tuning  SUBTYPE_FMT1 */
    #define NANOGRAPH_SUBT_AUDIO_OUT          0u  /* no subtype_units : integer/DAC format  */

    #define NANOGRAPH_SUBT_AUDIO_MPG         32u  /* compressed byte stream */

/* IO_DOMAIN_GPIO_IN           : subtypes and tuning  SUBTYPE_FMT1 */
   #define NANOGRAPH_SUBT_GPIO_IN             0u  /* no subtype_units  */

/* IO_DOMAIN_GPIO_OUT          : subtypes and tuning  SUBTYPE_FMT1 */
   #define NANOGRAPH_SUBT_GPIO_OUT            0u  /* no subtype_units  */

/* IO_DOMAIN_MOTION_IN         : subtypes and tuning  SUBTYPE_FMT1 */
   #define NANOGRAPH_SUBT_MOTION_A            1u
   #define NANOGRAPH_SUBT_MOTION_G            2u
   #define NANOGRAPH_SUBT_MOTION_B            3u
   #define NANOGRAPH_SUBT_MOTION_AG           4u
   #define NANOGRAPH_SUBT_MOTION_AB           5u
   #define NANOGRAPH_SUBT_MOTION_GB           6u
   #define NANOGRAPH_SUBT_MOTION_AGB          7u

/* IO_DOMAIN_2D_IN             : subtypes and tuning  SUBTYPE_FMT1 */
/*                      raw data is uint8 or uint16 but the subtype tells how to extract the pixel data */
/* IO_DOMAIN_2D_OUT            : subtypes and tuning  SUBTYPE_FMT1 */
   #define NANOGRAPH_SUBT_2D_YUV420P          1u /* Luminance, Blue projection, Red projection, 6 bytes per 4 pixels, reordered */
   #define NANOGRAPH_SUBT_2D_YUV422P          2u /* 8 bytes per 4 pixels, or 16bpp, Y0 Cb Y1 Cr (1 Cr & Cb sample per 2x1 Y samples) */
   #define NANOGRAPH_SUBT_2D_YUV444P          3u /* 12 bytes per 4 pixels, or 24bpp, (1 Cr & Cb sample per 1x1 Y samples) */
   #define NANOGRAPH_SUBT_2D_CYM24            4u /* cyan yellow magenta */
   #define NANOGRAPH_SUBT_2D_CYMK32           5u /* cyan yellow magenta black */
   #define NANOGRAPH_SUBT_2D_RGB8             6u /* RGB  3:3:2,  8bpp, (msb)2B 3G 3R(lsb) */
   #define NANOGRAPH_SUBT_2D_RGB16            7u /* RGB  5:6:5, 16bpp, (msb)5R 6G 5B(lsb) */
   #define NANOGRAPH_SUBT_2D_RGBA16           8u /* RGBA 4:4:4:4 32bpp (msb)4R */
   #define NANOGRAPH_SUBT_2D_RGB24            9u /* BBGGRR 24bpp (msb)8B */
   #define NANOGRAPH_SUBT_2D_RGBA32          10u /* BBGGRRAA 32bpp (msb)8B */
   #define NANOGRAPH_SUBT_2D_RGBA8888        11u /* AABBRRGG OpenGL/PNG format R=lsb A=MSB ("ABGR32" little endian) */
   #define NANOGRAPH_SUBT_2D_BW1B            12u /* Y, 1bpp, 0 is black, 1 is white */
   #define NANOGRAPH_SUBT_2D_GREY2B          13u /* Y, 2bpp, 0 is black, 3 is white, ordered from lsb to msb  */
   #define NANOGRAPH_SUBT_2D_GREY4B          14u /* Y, 4bpp, 0 is black, 15 is white, ordered from lsb to msb */
   #define NANOGRAPH_SUBT_2D_GREY8B          15u /* Grey 8b, 0 is black, 255 is white */
                                          
   #define NANOGRAPH_SUBT_2D_JPEG            64u /* legacy JPEG */
   #define NANOGRAPH_SUBT_2D_JP2             65u /* jpeg 2000  */
   #define NANOGRAPH_SUBT_2D_JXL             66u /* jpeg XL  */
   #define NANOGRAPH_SUBT_2D_GIF             67u /* Graphics Interchange Format */
   #define NANOGRAPH_SUBT_2D_PNG             68u /* Portable Network Graphics */
   #define NANOGRAPH_SUBT_2D_SVG             69u /* Scalable Vector Graphics */
   #define NANOGRAPH_SUBT_2D_TIFF            70u /* Tag Image File Format */
                                   

/* IO_DOMAIN_ANALOG_IN     : subtypes and tuning  SUBTYPE_FMT1 */
/* IO_DOMAIN_ANALOG_OUT : subtypes and tuning  SUBTYPE_FMT1 */
   #define NANOGRAPH_SUBT_ANA_ANY             0u /*        any                        */        
   #define NANOGRAPH_SUBT_ANA_METER           1u /* m         meter                   */
   #define NANOGRAPH_SUBT_ANA_KGRAM           2u /* kg        kilogram                */
   #define NANOGRAPH_SUBT_ANA_GRAM            3u /* g         gram                    */
   #define NANOGRAPH_SUBT_ANA_SECOND          4u /* s         second                  */
   #define NANOGRAPH_SUBT_ANA_AMPERE          5u /* A         ampere                  */
   #define NANOGRAPH_SUBT_ANA_KELVIB          6u /* K         kelvin                  */
   #define NANOGRAPH_SUBT_ANA_CANDELA         7u /* cd        candela                 */
   #define NANOGRAPH_SUBT_ANA_MOLE            8u /* mol       mole                    */
   #define NANOGRAPH_SUBT_ANA_HERTZ           9u /* Hz        hertz                   */
   #define NANOGRAPH_SUBT_ANA_RADIAN         10u /* rad       radian                  */
   #define NANOGRAPH_SUBT_ANA_STERADIAN      11u /* sr        steradian               */
   #define NANOGRAPH_SUBT_ANA_NEWTON         12u /* N         newton                  */
   #define NANOGRAPH_SUBT_ANA_PASCAL         13u /* Pa        pascal                  */
   #define NANOGRAPH_SUBT_ANA_JOULE          14u /* J         joule                   */
   #define NANOGRAPH_SUBT_ANA_WATT           15u /* W         watt                    */
   #define NANOGRAPH_SUBT_ANA_COULOMB        16u /* C         coulomb                 */
   #define NANOGRAPH_SUBT_ANA_VOLT           17u /* V         volt                    */
   #define NANOGRAPH_SUBT_ANA_FARAD          18u /* F         farad                   */
   #define NANOGRAPH_SUBT_ANA_OHM            19u /* Ohm       ohm                     */
   #define NANOGRAPH_SUBT_ANA_SIEMENS        20u /* S         siemens                 */
   #define NANOGRAPH_SUBT_ANA_WEBER          21u /* Wb        weber                   */
   #define NANOGRAPH_SUBT_ANA_TESLA          22u /* T         tesla                   */
   #define NANOGRAPH_SUBT_ANA_HENRY          23u /* H         henry                   */
   #define NANOGRAPH_SUBT_ANA_CELSIUSDEG     24u /* Cel       degrees Celsius         */
   #define NANOGRAPH_SUBT_ANA_LUMEN          25u /* lm        lumen                   */
   #define NANOGRAPH_SUBT_ANA_LUX            26u /* lx        lux                     */
   #define NANOGRAPH_SUBT_ANA_BQ             27u /* Bq        becquerel               */
   #define NANOGRAPH_SUBT_ANA_GRAY           28u /* Gy        gray                    */
   #define NANOGRAPH_SUBT_ANA_SIVERT         29u /* Sv        sievert                 */
   #define NANOGRAPH_SUBT_ANA_KATAL          30u /* kat       katal                   */
   #define NANOGRAPH_SUBT_ANA_METERSQUARE    31u /* m2        square meter (area)     */
   #define NANOGRAPH_SUBT_ANA_CUBICMETER     32u /* m3        cubic meter (volume)    */
   #define NANOGRAPH_SUBT_ANA_LITER          33u /* l         liter (volume)                               */
   #define NANOGRAPH_SUBT_ANA_M_PER_S        34u /* m/s       meter per second (velocity)                  */
   #define NANOGRAPH_SUBT_ANA_M_PER_S2       35u /* m/s2      meter per square second (acceleration)       */
   #define NANOGRAPH_SUBT_ANA_M3_PER_S       36u /* m3/s      cubic meter per second (flow rate)           */
   #define NANOGRAPH_SUBT_ANA_L_PER_S        37u /* l/s       liter per second (flow rate)                 */
   #define NANOGRAPH_SUBT_ANA_W_PER_M2       38u /* W/m2      watt per square meter (irradiance)           */
   #define NANOGRAPH_SUBT_ANA_CD_PER_M2      39u /* cd/m2     candela per square meter (luminance)         */
   #define NANOGRAPH_SUBT_ANA_BIT            40u /* bit       bit (information content)                    */
   #define NANOGRAPH_SUBT_ANA_BIT_PER_S      41u /* bit/s     bit per second (data rate)                   */
   #define NANOGRAPH_SUBT_ANA_LATITUDE       42u /* lat       degrees latitude[1]                          */
   #define NANOGRAPH_SUBT_ANA_LONGITUDE      43u /* lon       degrees longitude[1]                         */
   #define NANOGRAPH_SUBT_ANA_PH             44u /* pH        pH value (acidity; logarithmic quantity)     */
   #define NANOGRAPH_SUBT_ANA_DB             45u /* dB        decibel (logarithmic quantity)               */
   #define NANOGRAPH_SUBT_ANA_DBW            46u /* dBW       decibel relative to 1 W (power level)        */
   #define NANOGRAPH_SUBT_ANA_BSPL           47u /* Bspl      bel (sound pressure level; log quantity)     */
   #define NANOGRAPH_SUBT_ANA_COUNT          48u /* count     1 (counter value)                            */
   #define NANOGRAPH_SUBT_ANA_PER            49u /* /         1 (ratio e.g., value of a switch; )          */
   #define NANOGRAPH_SUBT_ANA_PERCENT        50u /* %         1 (ratio e.g., value of a switch; )          */
   #define NANOGRAPH_SUBT_ANA_PERCENTRH      51u /* %RH       Percentage (Relative Humidity)               */
   #define NANOGRAPH_SUBT_ANA_PERCENTEL      52u /* %EL       Percentage (remaining battery energy level)  */
   #define NANOGRAPH_SUBT_ANA_ENERGYLEVEL    53u /* EL        seconds (remaining battery energy level)     */
   #define NANOGRAPH_SUBT_ANA_1_PER_S        54u /* 1/s       1 per second (event rate)                    */
   #define NANOGRAPH_SUBT_ANA_1_PER_MIN      55u /* 1/min     1 per minute (event rate, "rpm")             */
   #define NANOGRAPH_SUBT_ANA_BEAT_PER_MIN   56u /* beat/min  1 per minute (heart rate in beats per minute)*/
   #define NANOGRAPH_SUBT_ANA_BEATS          57u /* beats     1 (Cumulative number of heart beats)         */
   #define NANOGRAPH_SUBT_ANA_SIEMPERMETER   58u /* S/m       Siemens per meter (conductivity)             */
   #define NANOGRAPH_SUBT_ANA_BYTE           59u /* B         Byte (information content)                   */
   #define NANOGRAPH_SUBT_ANA_VOLTAMPERE     60u /* VA        volt-ampere (Apparent Power)                 */
   #define NANOGRAPH_SUBT_ANA_VOLTAMPERESEC  61u /* VAs       volt-ampere second (Apparent Energy)         */
   #define NANOGRAPH_SUBT_ANA_VAREACTIVE     62u /* var       volt-ampere reactive (Reactive Power)        */
   #define NANOGRAPH_SUBT_ANA_VAREACTIVESEC  63u /* vars      volt-ampere-reactive second (Reactive Energy)*/
   #define NANOGRAPH_SUBT_ANA_JOULE_PER_M    64u /* J/m       joule per meter (Energy per distance)        */
   #define NANOGRAPH_SUBT_ANA_KG_PER_M3      65u /* kg/m3     kg/m3 (mass density, mass concentration)     */
   #define NANOGRAPH_SUBT_ANA_DEGREE         66u /* deg       degree (angle)                               */
   #define NANOGRAPH_SUBT_ANA_NTU            67u /* NTU       Nephelometric Turbidity Unit                 */

   // Secondary Unit (rfc8798)           Description          SenML Unit     Scale     Offset 
   #define NANOGRAPH_SUBT_ANA_MS             68u /* millisecond                  s      1/1000    0       1ms = 1s x [1/1000] */
   #define NANOGRAPH_SUBT_ANA_MIN            69u /* minute                       s      60        0        */
   #define NANOGRAPH_SUBT_ANA_H              70u /* hour                         s      3600      0        */
   #define NANOGRAPH_SUBT_ANA_MHZ            71u /* megahertz                    Hz     1000000   0        */
   #define NANOGRAPH_SUBT_ANA_KW             72u /* kilowatt                     W      1000      0        */
   #define NANOGRAPH_SUBT_ANA_KVA            73u /* kilovolt-ampere              VA     1000      0        */
   #define NANOGRAPH_SUBT_ANA_KVAR           74u /* kilovar                      var    1000      0        */
   #define NANOGRAPH_SUBT_ANA_AH             75u /* ampere-hour                  C      3600      0        */
   #define NANOGRAPH_SUBT_ANA_WH             76u /* watt-hour                    J      3600      0        */
   #define NANOGRAPH_SUBT_ANA_KWH            77u /* kilowatt-hour                J      3600000   0        */
   #define NANOGRAPH_SUBT_ANA_VARH           78u /* var-hour                     vars   3600      0        */
   #define NANOGRAPH_SUBT_ANA_KVARH          79u /* kilovar-hour                 vars   3600000   0        */
   #define NANOGRAPH_SUBT_ANA_KVAH           80u /* kilovolt-ampere-hour         VAs    3600000   0        */
   #define NANOGRAPH_SUBT_ANA_WH_PER_KM      81u /* watt-hour per kilometer      J/m    3.6       0        */
   #define NANOGRAPH_SUBT_ANA_KIB            82u /* kibibyte                     B      1024      0        */
   #define NANOGRAPH_SUBT_ANA_GB             83u /* gigabyte                     B      1e9       0        */
   #define NANOGRAPH_SUBT_ANA_MBIT_PER_S     84u /* megabit per second           bit/s  1000000   0        */
   #define NANOGRAPH_SUBT_ANA_B_PER_S        85u /* byteper second               bit/s  8         0        */
   #define NANOGRAPH_SUBT_ANA_MB_PER_S       86u /* megabyte per second          bit/s  8000000   0        */
   #define NANOGRAPH_SUBT_ANA_MV             87u /* millivolt                    V      1/1000    0        */
   #define NANOGRAPH_SUBT_ANA_MA             88u /* milliampere                  A      1/1000    0        */
   #define NANOGRAPH_SUBT_ANA_DBM            89u /* decibel rel. to 1 milliwatt  dBW    1       -30     0 dBm = -30 dBW       */
   #define NANOGRAPH_SUBT_ANA_UG_PER_M3      90u /* microgram per cubic meter    kg/m3  1e-9      0        */
   #define NANOGRAPH_SUBT_ANA_MM_PER_H       91u /* millimeter per hour          m/s    1/3600000 0        */
   #define NANOGRAPH_SUBT_ANA_M_PER_H        92u /* meterper hour                m/s    1/3600    0        */
   #define NANOGRAPH_SUBT_ANA_PPM            93u /* partsper million             /      1e-6      0        */
   #define NANOGRAPH_SUBT_ANA_PER_100        94u /* percent                      /      1/100     0        */
   #define NANOGRAPH_SUBT_ANA_PER_1000       95u /* permille                     /      1/1000    0        */
   #define NANOGRAPH_SUBT_ANA_HPA            96u /* hectopascal                  Pa     100       0        */
   #define NANOGRAPH_SUBT_ANA_MM             97u /* millimeter                   m      1/1000    0        */
   #define NANOGRAPH_SUBT_ANA_CM             98u /* centimeter                   m      1/100     0        */
   #define NANOGRAPH_SUBT_ANA_KM             99u /* kilometer                    m      1000      0        */
   #define NANOGRAPH_SUBT_ANA_KM_PER_H      100u /* kilometer per hour           m/s    1/3.6     0        */
                                                                                                
   #define NANOGRAPH_SUBT_ANA_GRAVITY       101u /* earth gravity                m/s2   9.81      0       1g = m/s2 x 9.81     */
   #define NANOGRAPH_SUBT_ANA_DPS           102u /* degrees per second           1/s    360       0     1dps = 1/s x 1/360     */   
   #define NANOGRAPH_SUBT_ANA_GAUSS         103u /* Gauss                        Tesla  10-4      0       1G = Tesla x 1/10000 */
   #define NANOGRAPH_SUBT_ANA_VRMS          104u /* Volt rms                     Volt   0.707     0    1Vrms = 1Volt (peak) x 0.707 */
   #define NANOGRAPH_SUBT_ANA_MVPGAUSS      105u /* Hall effect, mV/Gauss        millivolt 1      0    1mV/Gauss                    */

/* IO_DOMAIN_RTC               : subtypes and tuning  SUBTYPE_FMT1 */

/* IO_DOMAIN_USER_INTERFACE_IN    11 : subtypes and tuning  SUBTYPE_FMT1 */

/* IO_DOMAIN_USER_INTERFACE_OUT   12 : subtypes and tuning  SUBTYPE_FMT1 */



/*============================  ARCS =====================================================*/

#define ARC_ID_UNUSED    0x07FFu




// /*================================= ONGOING  (RAM) ==============================
      
      // The graph hold a table of uint8_t in RAM for the "on-going" flag    
// */

// #define     UNUSED_IO_MSB 7u  
// #define     UNUSED_IO_LSB 1u  /* 7 */
// #define    ONGOING_IO_MSB 0u  
// #define    ONGOING_IO_LSB 0u  /* 1 set in scheduler, reset in IO, iomask manages processor access */

/* ============================================================================================ */
/*================================= SCRIPTS =================================================== */
/* ============================================================================================ */
/* 
  *- SCRIPTS are adressed with a table_int32[128] : offset, ARC, binary format
        ARC descriptor: size regs/stack, parameters/UC, collision Byte, max cycles 
        The first are indexed with the NODE header 7b index (SCRIPT_LW0) 
        Script index #0 means "disabled"  Indexes 1..up to 63 are used for shared subroutines.
*/

#define     ARC_SCROFF0_MSB U(31) /* 11 associated arc descriptor */
#define     ARC_SCROFF0_LSB U(21) /*                   */
#define  FORMAT_SCROFF0_MSB U(20) /* 3  byte codes format = 0, 7 binary native architecture ARCHID_LW0 */
#define  FORMAT_SCROFF0_LSB U(19) /*       ARMv6-M */
#define  SHARED_SCROFF0_MSB U(18) /* 1  shareable memory for the script with other scripts in mono processor platforms */
#define  SHARED_SCROFF0_LSB U(18) /*                                    */
#define  OFFSET_SCROFF0_MSB U(17) /* 17 offset in the W32 script table */
#define  OFFSET_SCROFF0_LSB U( 0) /*    placed at    */

         
/* 
    arc descriptors used to address the working area : registers and stack
*/
#define      SCRIPT_PTR_SCRARCW0  U( 0) /* Base address + NREGS + new UC */
#define          SCRIPT_SCRARCW1  U( 1) /* LENGTH use case UC0 */
#define          RDFLOW_SCRARCW2  U( 2) /* READ use-case UC1 + ARCEXTEND_ARCW2  */
#define        WRIOCOLL_SCRARCW3  U( 3) /* WRITE + STACK LENGTH + Flag logMaxCycles8b */
#define          DBGFMT_SCRARCW4  4 

          
#define    unused____SCRARCW0_MSB U(31)    
#define    unused____SCRARCW0_LSB U(29) /*  3    base address of the working memory */
#define NEW_USE_CASE_SCRARCW0_MSB U(28) /*  1  new use-case arrived */ 
#define NEW_USE_CASE_SCRARCW0_LSB U(28) /*     */ 
#define    BASEIDXOFFSCRARCW0_MSB U(27)    
#define    BASEIDXOFFSCRARCW0_LSB U( 0) /* 28  base address of the script memory (regs + state + stack)  */

#define     CODESIZE_SCRARCW1_MSB U(31) 
#define     CODESIZE_SCRARCW1_LSB U(22) /* 10 */
#define    BUFF_SIZE_SCRARCW1_MSB U(21) /*    */
#define    BUFF_SIZE_SCRARCW1_LSB U( 0) /* 22 BYTE-acurate up to 4MBytes (up to 128GB with ARCEXTEND_ARCW2 */

//#define          READ_ARCW2_MSB U(21) /*    data read index  Byte-acurate up to 4MBytes starting from base address */
//#define          READ_ARCW2_LSB U( 0) /* 22 this is incremented by "frame_size" FRAMESIZE_FMT0  */

#define    COLLISION_SCRARCW3_MSB U(31) /*  8  */
#define    COLLISION_SCRARCW3_LSB U(24) /*     */
//#define         WRITE_ARCW3_MSB U(21) /*    write pointer is incremented by FRAMESIZE_FMT0 */
//#define         WRITE_ARCW3_LSB U( 0) /* 22 write read index  Byte-acurate up to 4MBytes starting from base address */

#define  RAMTOTALW32_SCRARCW4_MSB U(23) /*  13  Total in words = 2*(regs + stack) + heap size = 8k W32*/
#define  RAMTOTALW32_SCRARCW4_LSB U(11) /*     */
#define        NREGS_SCRARCW4_MSB U(10) /*  4   number of registers used in this script */ 
#define        NREGS_SCRARCW4_LSB U( 7) /*     */
#define       NSTACK_SCRARCW4_MSB U( 6) /*  7   max size of the FIFO/stack in register size:  128 registers */
#define       NSTACK_SCRARCW4_LSB U( 0) /*     */

/* =================
    script (SCRIPT_LW0) used to copy input test-patterns, set-parameters from the global use-case or from information 
    of an application call-back

    "analog user-interface" called (knobs / needles) give controls and visibility on NODE parameters and
    are accessed with scripts 

    use 0 and 1 like "pre0post1"
*/
#define SCRIPT_PRERUN 0u        /* executed before calling the node : the Z flag is set */
#define SCRIPT_POSTRUN 1u       /* executed after */



/* ============================================================================================ */
/* ======================================   NODES  ============================================ */ 
/* ============================================================================================ */

/*---------------- NODE NANOGRAPH_SET_PARAMETER -----------------------------
    NanoGraph_interpreter_process (NANOGRAPH_SET_PARAMETER, uint32_t *data)
        data -> list of [node idx; parameter address]..[0;0]
*/
#define MAX_NB_PENDING_PARAM_UPDATES 8      // size of the above list 


#define NanoGraph_script_index 1u     /* NanoGraph_script() is the first one in the list node_entry_points[] */
/*
  *- LINKED-LIST of SWC
       minimum 5 words/SWC
       Word0   : header processor/architecture, nb arcs, SWCID, arc
       Word00  : script index
       Word1+n : arcs * 2  + debug page
       Word2+2n: ADDR + SIZE of memory segments
       Word3   : optional 4 words User key + Platform Key
       Word4+n : 1+data Preset, New param!, Skip length, 
          byte stream: nbparams (ALLPARAM), {tag, nbWord32, params}

       list Ends with the NODE ID 0x03FF 

       
       HEADER[0] ===============================================================================
*/

#define NANOGRAPH_INSTANCE_ANY_PRIORITY    0u      /* PRIORITY_LW0 bit field */
#define NANOGRAPH_INSTANCE_LOWLATENCYTASKS 1u
#define NANOGRAPH_INSTANCE_MIDLATENCYTASKS 2u
#define NANOGRAPH_INSTANCE_BACKGROUNDTASKS 3u


                               /*   corresponds to "WHOAMI_PARCH_LSB" and must not be null */ 
#define   WHOAMI_LW0_MSB U(31) 
#define PRIORITY_LW0_MSB U(31) /*   RTOS instances. 0=NANOGRAPH_INSTANCE_ANY_PRIORITY */
#define PRIORITY_LW0_LSB U(30) /* 2 up to 3 instances per processors, see PRIORITY_SCTRL  */
#define   PROCID_LW0_MSB U(29) /*   same as PROCID_PARCH (stream instance) (1..3) */
#define   PROCID_LW0_LSB U(27) /* 3 execution reserved to this processor index  (1..7) */  
#define   ARCHID_LW0_MSB U(26) 
#define   ARCHID_LW0_LSB U(24) /* 3 execution reserved to this processor architecture (1..7)  */
#define   WHOAMI_LW0_LSB U(24) 
/*---------------------------*/
#define un_______LW0_MSB U(23) 
#define un_______LW0_LSB U(20) /*  4   */
#define    ALLOC_LW0_MSB U(19) /*     graph compilation do not manage the memory allocation */  
#define    ALLOC_LW0_LSB U(19) /*  1  the SWC dynamically returns the amount of memory it needs */
#define      KEY_LW0_MSB U(18) 
#define      KEY_LW0_LSB U(18) /*  1  two 64b KEYs are inserted after the memory pointers (word 2+2n)  */
#define   NBARCW_LW0_MSB U(17) 
#define   NBARCW_LW0_LSB U(13) /*  5  total nb arcs, streaming and metadata/control {0 .. MAX_NB_NANOGRAPH_PER_NODE} */
#define NALLOCM1_LW0_MSB U(12) /*    number of memory segments (pointer + size) to give at RESET [0..MAX_NB_MEM_REQ_PER_NODE-1] */  
#define NALLOCM1_LW0_LSB U(10) /*  3   2 words each  */
#define NODE_IDX_LW0_MSB U( 9) 
#define NODE_IDX_LW0_LSB U( 0) /* 10 0=nothing, node index of node_entry_points[] */

/* maximum number of processors = nb_proc x nb_arch */
#define MAX_GRAPH_NB_PROCESSORS ((1<<(PROCID_LW0_MSB-PROCID_LW0_LSB+1))*(1<<(ARCHID_LW0_MSB-ARCHID_LW0_LSB+1)))


        /*  HEADER[00] extension */
#define  un_______LW00_MSB U(31) 
#define  un_______LW00_LSB U(9) 
#define   PROTECT_LW00_MSB U(8)  /*    memory protection : activate MPU before calling the node TODO @@@@@ */
#define   PROTECT_LW00_LSB U(8)  /*  1  graph command node_memory_isolation 1 */
#define SMP_FLUSH_LW00_MSB U(7)  /*    reload non-working memory at start and flush at stop */
#define SMP_FLUSH_LW00_LSB U(7)  /*  1  SW cache coherency  */
#define    SCRIPT_LW00_MSB U(6)  /*    script called Before/After (debug, verbose trace control) */
#define    SCRIPT_LW00_LSB U(0)  /*  7 script ID to call before and after calling NODE    */



        /*
           ============================================================================== =
        */
        /* HEADER[1 +n] -arcs

            starting with the one used for locking, the streaming arcs, then the metadata arcs 
            arc(tx) used for locking is ARC0_LW1

            input parameter arcs are declared as TX-arcs, 
            preset "empty" and let untouched by the node
        */

#define ARCOFF 2            /* arc data offset from Header start : LW0 + LW00 */

#define ARC_RX0TX1_TEST  0x0800u /* MSB gives the direction of the arc */
#define ARC_RX0TX1_CLEAR 0x07FFu 

#define un__1_LW1_MSB 31u 
#define un__1_LW1_LSB 28u /*  4   */
#define  ARC1_LW1_MSB 27u
#define ARC1D_LW1_LSB 27u /*  1  ARC1 direction */
#define ARC1D_LW1_MSB 27u
#define  ARC1_LW1_LSB 16u /* 11  ARC1  11 usefull bits + 1 MSB to tell rx0tx1 */

#define un__0_LW1_MSB 15u 
#define un__0_LW1_LSB 12u /*  4   */
#define  ARC0_LW1_MSB 11u
#define ARC0D_LW1_LSB 11u /*  1  ARC0 direction */
#define ARC0D_LW1_MSB 11u
#define  ARC0_LW1_LSB  0u /* 11  ARC0, (10 + 1 rx0tx1) up to 2K ARCs */


        /*
           ============================================================================== =
        */
        /* word2 pair - FIRST WORD : memory banks address + SECOND WORD = size */
#define NBW32_MEMREQ_LW2  2u    /* there are two words per memory segments, to help programing the memory protection unit (MPU) */
#define ADDR_LW2 0u             /* 29b format address + SWAP + WORK indicators */ 
#define SIZE_LW2 1u             /* 29b format size of the segment */ 


        /*  Memory Protection Unit (MPU)  has 8 memory segments : 4 memory segments per NODE (1 instance + 
                3 segments) + 1 code + 1 stack + 1 IRQ + 1 Stream/services/script
        */
#define MAX_NB_MEM_REQ_PER_NODE U(6)    /* TO_SWAP_LW2S limits to 6 MAX and 4 MAX if memory protection is used */

        /* S->node_memory_banks_offset 
           memreq = &(S->node_header[S->node_memory_banks_offset]); 

            = size of the memory segment  + control on the first segment 
                            if the segment is swapped, the 12-LSB bits give the ARC ID of the buffer 
                            and the memory size is given by the FIFO descriptor (BUFF_SIZE_ARCW1) */
#define LW2S_NOSWAP 0u
#define LW2S_SWAP 1u

#define     SWAP_LW2S0_MSB U(31) /*      0= normal memory segment, 1 = swap before execute  */
#define     SWAP_LW2S0_LSB U(31) /*  1                                                      */
#define     WORK_LW2S0_MSB U(30) /*      scratch/working area                               */
#define     WORK_LW2S0_LSB U(30) /*  1                                                      */
#define    CLEAR_LW2S0_MSB U(29) /*      working are always cleared, static only at RESET   */
#define    CLEAR_LW2S0_LSB U(29) /*  1                                                      */
#define  SIZE_EXT_OFF_LW2S_MSB SIZE_EXT_OFF_FMT0_MSB  /* 29 = offsets(5) + EXT(3) + sign(1) + size(20) */
#define   ADDR_OFFSET_LW2S_MSB  ADDR_OFFSET_FMT0_MSB 
#define   ADDR_OFFSET_LW2S_LSB  ADDR_OFFSET_FMT0_LSB 
#define      SIZE_EXT_LW2S_MSB     SIZE_EXT_FMT0_MSB 
#define     EXTENSION_LW2S_MSB    EXTENSION_FMT0_MSB 
#define     EXTENSION_LW2S_LSB    EXTENSION_FMT0_LSB 
#define   SIGNED_SIZE_LW2S_MSB  SIGNED_SIZE_FMT0_MSB 
#define     SIZE_SIGN_LW2S_MSB    SIZE_SIGN_FMT0_MSB 
#define     SIZE_SIGN_LW2S_LSB    SIZE_SIGN_FMT0_LSB 
#define          SIZE_LW2S_MSB         SIZE_FMT0_MSB 
#define          SIZE_LW2S_LSB         SIZE_FMT0_LSB 
#define   SIGNED_SIZE_LW2S_LSB  SIGNED_SIZE_FMT0_LSB 
#define     SWAPBUFID_LW2S_MSB ARC0_LW1_MSB /*     ARC => swap source address in slow memory + swap length */
#define     SWAPBUFID_LW2S_LSB ARC0_LW1_LSB /* 12  ARC0, (11 + 1) up to 2K FIFO */


        /*
           ============================================================================== =
        */
        /* Word3   : optional 4 words User key + Platform Key
                      if KEY_LW1 == 1
           0x11223344 User key word 0
           0x11223344 User key word 1
           0x11223344 platform key word 0
           0x11223344 platform key word 1
        */
#define NBWORDS_KEY_USER_PLATFORM 4



        /*
           ============================================================================== =
        */
        /* word 4+n - parameters 
          NODE header can be in RAM (to patch the parameter area, cancel the component..)

        BOOTPARAMS (if W32LENGTH_LW4>0 )

        PARAM_TAG : 4  index to parameter (0='all parameters')
        PRESET    : 4  preset index (SWC delivery)
        TRACEID   : 8  
        W32LENGTH :16  nb of WORD32 to skip at run time, 0 means NO PARAMETER, max=256kB

        Example with 
        1  u8:  0                           trace ID
        1  h8;  01                          TAG = "all parameters"_0 + preset_1
                      parameters 
        1  u8;  2                           Two biquads
        1  u8;  0                           postShift
        5 h16; 5678 2E5B 71DD 2166 70B0     b0/b1/b2/a1/a2 
        5 h16; 5678 2E5B 71DD 2166 70B0     second biquad

        NODE can declare an extra input arc to receive a huge set of parameters (when >256kB), for example a 
        NN model. This is a fake arc and the read pointer is never incremented during the execution of the node.
        */
        //#define MAX_TMP_PARAMETERS 30   /* temporary buffer (words32) of parameters to send to the Node */


#define PARAM_TAG_LW4_MSB U(31) 
#define PARAM_TAG_LW4_LSB U(28) /* 4  for PARAM_TAG_CMD (15='all parameters')  */
#define    PRESET_LW4_MSB U(27)
#define    PRESET_LW4_LSB U(24) /* 4  preset   16 precomputed configurations */
#define   TRACEID_LW4_MSB U(23)       
#define   TRACEID_LW4_LSB U(16) /* 8  TraceID used to route the trace to the corresponding peripheral/display-line */
#define W32LENGTH_LW4_MSB U(15) /*    if >256kB are needed then use an arc to a buffer */
#define W32LENGTH_LW4_LSB U( 0) /* 16 skip this : number of uint32 to skip the boot parameters */



/* =================================================================================== */

/* last word of the graph has NODE index 0b11111..111 */
#define GRAPH_LAST_WORD_MSB NODE_IDX_LW0_MSB
#define GRAPH_LAST_WORD_LSB NODE_IDX_LW0_LSB
#define GRAPH_LAST_WORD U((U(1)<<U(NODE_IDX_LW0_MSB- NODE_IDX_LW0_LSB+1U))-1U)


/*
 *  NODE manifest :
 */ 

//enum nanograph_node_status {
#define NODE_BUFFERS_PROCESSED 0u
#define NODE_NEED_RUN_AGAIN 1u        /* NODE completion type */

//enum mem_mapping_type {
#define MEM_TYPE_STATIC          0u   /* (LSB) memory content is preserved (default ) */
#define MEM_TYPE_WORKING         1u   /* scratch memory content is not preserved between two calls */
#define MEM_TYPE_PERIODIC_BACKUP 2u   /* persistent static parameters to reload for warm boot after a crash */
#define MEM_TYPE_PSEUDO_WORKING  3u   /* static only during the uncompleted execution state of the SWC, see NODE_RUN 

                periodic backup use-case : long-term estimators. This memory area is cleared at cold NODE_RESET and 
                refreshed for warm NODE_RESET. The NODE should not reset it (there is 
                no "warm-boot reset" entry point. The period of backup depends on platform capabilities 
                When MBANK_BACKUP is a retention-RAM there is nothing to do, when it is standard RAM area then on periodic
                basis the AL will be call to transfer data to Flash 
            */
              

//enum mem_speed_type                         /* memory requirements associated to enum memory_banks */
#define MEM_SPEED_REQ_ANY           0u   /* best effort */
#define MEM_SPEED_REQ_FAST          1u   /* will be internal SRAM when possible */
#define MEM_SPEED_REQ_CRITICAL_FAST 2u   /* will be TCM when possible
           When a NODE is declaring this segment as relocatable ("RELOC_MEMREQ") it will use 
           physical address different from one TCM to an other depending on the processor running the SWC.
           The scheduler shares the TCM address dynamically before calling the SWC. 
           This TCM address is provided as a pointer after the XDM in/out pointer
           The TCM address is placed at the end (CRITICAL_FAST_SEGMENT_IDX) of long_offset[] 
           */
                                

//enum buffer_alignment_type            
#define MEM_REQ_4BYTES_ALIGNMENT   2u    /*   mask = ~((1 << (7 & mem_req_2bytes_alignment) -1) */
#define MEM_REQ_8BYTES_ALIGNMENT   3u
#define MEM_REQ_16BYTES_ALIGNMENT  4u
#define MEM_REQ_32BYTES_ALIGNMENT  5u
#define MEM_REQ_64BYTES_ALIGNMENT  6u
#define MEM_REQ_128BYTES_ALIGNMENT 7u


#define NODE_CONTROLS U(4u)
#define NODE_CONTROLS_NAME U(8u)



/* ====================================================================================================================================== */
/* ============================================================= ARC =====================================================================*/
/* ====================================================================================================================================== */
/*
  arc descriptions : 
                             
      - arc_descriptor_ring : R/W are used to manage the alignment of data to the base address and notify the SWC
                              debug pattern, statistics on data, time-stamps of access, 
                              realignment of data to base-address when READ > (SIZE) - consumer frame-size
                              deinterleave multichannel have the same read/write offset but the base address starts 
                              from the end of the previous channel boundary of the graph
*/

// enum debug_arc_computation_1D { COMPUTCMD_ARCW4 

/* COMPUTCMD_ARCW4  7bits 6543210
                 command  ----XXX increment : log timestamps
                                  wakeup instance : platform knows how to wakeup instance xxxx
                                  callback : application callback
                                  compute : signal processing / peak detection
                parameter xxxx--- instance ID to wakeup, callback, increment, data average, time-stamp 
*/
//#define COMPUTCMD_ARCW4_NO_ACTION                0u   
//#define COMPUTCMD_ARCW4_INCREMENT_REG            1u  /* increment DEBUG_REG_ARCW4 with the number of RAW samples */      
//#define COMPUTCMD_ARCW4_SET_ZERO_ADDR            2u  /* set a 0 in to *DEBUG_REG_ARCW4, 5 MSB gives the bit to clear */      
//#define COMPUTCMD_ARCW4_SET_ONE_ADDR             3u  /* set a 1 in to *DEBUG_REG_ARCW4, 5 MSB gives the bit to set */       
//#define COMPUTCMD_ARCW4_INCREMENT_REG_ADDR       4u  /* increment *DEBUG_REG_ARCW4 */ 
//#define COMPUTCMD_ARCW4__5                       5u 
//#define COMPUTCMD_ARCW4_APP_CALLBACK1            6u  /* call-back in the application side, data rate estimate in DEBUG_REG_ARCW4 */      
//#define COMPUTCMD_ARCW4_APP_CALLBACK2            7u  /* second call-back : wake-up processor from DEBUG_REG_ARCW4=[ProcID, command]  */      
//#define COMPUTCMD_ARCW4_TIME_STAMP_READ          8u  /* log a time-stamp of the last read access to FIFO */
//#define COMPUTCMD_ARCW4_TIME_STAMP_WRITE         9u  /* log a time-stamp of the last write access to FIFO */
//#define COMPUTCMD_ARCW4_PEAK_DATA               10u  /* peak/mean/min with forgeting factor 1/256 in DEBUG_REG_ARCW4 */          
//#define COMPUTCMD_ARCW4_MEAN_DATA               11u 
//#define COMPUTCMD_ARCW4_MIN_DATA                12u 
//#define COMPUTCMD_ARCW4_ABSMEAN_DATA            13u 
//#define COMPUTCMD_ARCW4_DATA_TO_OTHER_ARC       14u  /* when data is changing the new data is push to another arc DEBUG_REG_ARCW4=[ArcID] */
//#define COMPUTCMD_ARCW4_LOOPBACK                15u  /* automatic rewind read/write */           


/* 
    scripts associated to arcs : 
*/
#define COMPUTCMD_ARCW4_SCRIPT_INDEX_OFFSET     20u  /* index of the scripts = (CMD - xx_OFFSET) */           
#define COMPUTCMD_ARCW4_LAST ((1<<(COMPUTCMD_ARCW4_MSB-COMPUTCMD_ARCW4_LSB+1))-1) /* 7bits CMD bit-field */

#define ARC_DBG_REGISTER_SIZE_W32 2u                 /* debug registers on 64 bits */


/* increment DEBUG_REG_ARCW4 with the number of RAW samples */
/* set a 0 in to *DEBUG_REG_ARCW4, 5 MSB gives the bit to clear */
/* set a 1 in to *DEBUG_REG_ARCW4, 5 MSB gives the bit to set */
/* increment *DEBUG_REG_ARCW4 */

/* call-back in the application side, data rate estimate in DEBUG_REG_ARCW4 */
/* second call-back : wake-up processor from DEBUG_REG_ARCW4=[ProcID, command]  */

/* peak/mean/min with forgeting factor 1/256 in DEBUG_REG_ARCW4 */

/* when data is changing the new data is push to another arc DEBUG_REG_ARCW4=[ArcID] */
/* automatic rewind read/write */

/* debug registers on 64 bits */

/* Arcs used for synchronized streams : for example I/V capture must be synchronized     */
/*  the arc CMD is used to instert a time-stamp before before raw data is pushed in the FIFO    */
/*  the stream consumer check the time-stamps and rejects the data outside of a predefined time-window */

/*
*   Flow error management with FLOW_RD_ARCW2 / FLOW_WR_ARCW2 = let an arc stay with 25% .. 75% of data
*       process done on "router" node when using HQOS arc and IO master interfaces
* 
*  The arc is initialized with 50% of null data
*  The processing is frame-based, there are minimum 3 frames in the buffer

*   When a IO-master writes in an arc with FLOW_WR_ARCW2=1 and the arc is full at +75%, the new data 
*       is extrapolated and the arc stays at 75% full 
*      Buff  xxxxxxxx|xxxx|xxxx|xxxx|bbbb|aaaa|  buffer full after NewData was push by the IO-master
*              R_ptr                      W_ptr
*            The previous frame (bbb) is filled (bbb x win_rampDown) + (newData_aaa x win_rampUp)
*            W_ptr steps back 
*            xxxxxxxx|xxxx|xxxx|xxxx|bbaa|----|  buffer full
*              R_ptr                      W_ptr
* 
*   When a IO-master read from an arc with FLOW_RD_ARCW2=1 and the arc is empty at -25%, the new data 
*       is extrapolated and the arc stays at 25% empty
* 
*      Buff  |bbbb| is read by the IO-master
*            |bbbb|aaaa|----|  buffer hold only ONE frame |aaaa| after the previous read
*                  R_ptr W_ptr
*            The previous frame (bbb) is filled (aaa x win_rampDown) + (bbb x win_rampUp)
*               and R_ptr steps back
*            |aabb|aaaa|----|  bbbb
*             R_ptr      W_ptr
*/

#define arc_read_address                1u
#define arc_write_address               2u
#define arc_data_amount                 3u
#define arc_free_area                   4u
#define data_swapped_with_arc           5u
#define arc_data_realignment_to_base    6u

#define PRODNODE_RESET_COMPLETED        1u      /* used in NODESTATE_ARCW2 */

#if CACHE_LINE_BYTE_LENGTH == 0
#define SIZEOF_ARCDESC_W32 (5u)                         /* ARC DESCRIPTORS SIZE=5, 4-bytes aligned */
#define ARC_DESCRIPTOR_ALIGNMENT MEM_REQ_4BYTES_ALIGNMENT  /* bit alignment 4bits  (1<<2) */
#else
#define SIZEOF_ARCDESC_W32 ((2*CACHE_LINE_BYTE_LENGTH)/4)  /* ARC DESCRIPTORS 32/64-bytes aligned */
#if CACHE_LINE_BYTE_LENGTH == 32
    #define ARC_DESCRIPTOR_ALIGNMENT MEM_REQ_32BYTES_ALIGNMENT   /* Bytes alignment  (1<<5) */
#endif
#if CACHE_LINE_BYTE_LENGTH == 64
    #define ARC_DESCRIPTOR_ALIGNMENT MEM_REQ_64BYTES_ALIGNMENT   /* Bytes alignment  (1<<6) */
#endif
#endif

#define          BASE_ARCW0    U(0)  
#define  HIGH_QOS_ARCW0_MSB U(31) /*     arc with high QoS */
#define  HIGH_QOS_ARCW0_LSB U(31) /*  1  data in the arc is processed whatever the content of the other arcs */
#define   MPFLUSH_ARCW0_MSB U(30) /*     0: single proc, 1: multiproc > invalidate / clean cache */
#define   MPFLUSH_ARCW0_LSB U(29) /*  2  */
#define BASEIDXOFFARCW0_MSB SIZE_EXT_OFF_FMT0_MSB /* 29 bits = offsets(5) + EXT(3) + sign(1) + size(20) */
#define   DATAOFF_ARCW0_MSB ADDR_OFFSET_FMT0_MSB  /*      address = offset[DATAOFF] + BASEIDX[Bytes] */
#define   DATAOFF_ARCW0_LSB ADDR_OFFSET_FMT0_LSB  /*      16 long offsets  */
#define  BUFFBASE_ARCW0_MSB SIZE_EXT_FMT0_MSB     /* 24 bits = extended(3) + sign(1) + size(20)   */
#define  BUFFBASE_ARCW0_LSB SIZE_EXT_FMT0_LSB
#define BASEIDXOFFARCW0_LSB SIZE_EXT_OFF_FMT0_LSB
#define   MPFLUSH_CTRL0     1     /*  reload/flush buffer and arc descriptor */

#define          SIZE_ARCW1    U(1 + CACHE_LINE_BYTE_LENGTH/4)
#define    unused_ARCW1_MSB U(31) /*     */
#define    unused_ARCW1_LSB U(26) /*  6  */
#define RESETDONE_ARCW1_MSB U(25) /*     */
#define RESETDONE_ARCW1_LSB U(25) /*  1  */
#define NEW_PARAM_ARCW1_MSB U(24) /*     */
#define NEW_PARAM_ARCW1_LSB U(24) /*  1  notify a new parameter setting, script waits NEW_PARAM_ARCW =0 before loading new param */
#define BUFF_SIZE_ARCW1_MSB SIZE_EXT_FMT0_MSB /*     */
#define BUFF_SIZE_ARCW1_LSB SIZE_EXT_FMT0_LSB /* 24  */

#define NEW_PARAM_ARCW1_BIT_LSB U(NEW_PARAM_ARCW1_LSB-24) /* bit-field access in a Byte */
#define COLL2NEWPARAM_BYTES  (-4) /* -4 bytes offset to go from COLLISION_ARCW2 to NEW_PARAM_ARCW1 */ 
#define COLLISION2CTRL_BYTES (-4)
#define NEW_RESET_ARCW1_BIT_LSB U(NEW_RESET_ARCW1_LSB-24) /* bit-field access in a Byte */


#define            RD_ARCW2    U(2 + CACHE_LINE_BYTE_LENGTH/4)  
#define COLLISION_ARCW2_BYTE U(3) /*     pt8b_collision_arc byte offset in the word = signature of the producer core */
#define COLLISION_ARCW2_MSB U(31) /*  8  MSB byte used to lock the SWC, loaded with arch+proc+instance ID */ 
#define COLLISION_ARCW2_LSB U(24) /*       to check node-access collision from an other processor */
#define      READ_ARCW2_MSB SIZE_EXT_FMT0_MSB /*     */
#define      READ_ARCW2_LSB SIZE_EXT_FMT0_LSB /* 24  */


#define            WR_ARCW3    U(3)    
#define    unused_ARCW3_MSB U(31) /*     */
#define    unused_ARCW3_LSB U(25) /*  7  */
#define ALIGNBLCK_ARCW3_MSB U(24) /*     producer blocked sets "I need data realignement from the consumer because the buffer is full" */
#define ALIGNBLCK_ARCW3_LSB U(24) /*  1   a full buffer can have the Write index = BUFF_SIZE, there is no space lost */
#define     WRITE_ARCW3_MSB SIZE_EXT_FMT0_MSB /*    write pointer is incremented by FRAMESIZE_FMT0 */
#define     WRITE_ARCW3_LSB SIZE_EXT_FMT0_LSB /* 24 write read index  Byte-acurate up to 4MBytes starting from base address */


#define             FMT_ARCW4   U(4)
#define    unused_ARCW4_MSB U(31) /*       */ 
#define    unused_ARCW4_LSB U(26) /*  6    */
#define    SCRIPT_ARCW4_MSB U(25) /*     log timestamps , wakeup instance application callback */
#define    SCRIPT_ARCW4_LSB U(16) /* 10  compute : signal processing / peak detection */
#define CONSUMFMT_ARCW4_MSB U(15) /*      */
#define CONSUMFMT_ARCW4_LSB U( 8) /*  8 bits CONSUMER format  */ 
#define PRODUCFMT_ARCW4_MSB U( 7) /*  8 bits PRODUCER format  (intptr_t) +[i x NANOGRAPH_FORMAT_SIZE_W32]  */ 
#define PRODUCFMT_ARCW4_LSB U( 0) /*    Graph generator gives IN/OUT arc's frame size to be the LCM of NODE "grains" */



/* ============================================================================================ */
/*================================= NANOGRAPH_HWIO_CONTROL (FLASH) =============================== */
/* ======================================== pio_control ======================================= 
    NBHWIOIDX_GR0_LSB/MSB = size of platform_io_al_idx_to_stream[] in W32 used for the translation to 
        IO graph indexes

    4 bytes for the translation table HW->graph (platform_io_al_idx_to_stream)
     LSB 16b is a word index to NANOGRAPH_IO_CONTROL[]
     MSB byte is the IO affinity selection, used to make the "iomask" of the interpreter instance

*/
#define      TRANSLATE_PLATFORM_HWIO_AL_IDX_SIZE_W32 1 
#define      INST_IDX_HWIO_CONTROL_MSB      U(20)  
#define      INST_IDX_HWIO_CONTROL_LSB      U(16)          /*  5 instance allowed to initialize this IO */
#define IDX_TO_NANOGRAPH_HWIO_CONTROL_MSB      U(15)  
#define IDX_TO_NANOGRAPH_HWIO_CONTROL_LSB      U( 0)          /* 16 index to graph nanograph_io_control[] and ongoing[] */  
#define NOT_CONNECTED_TO_GRAPH ((1<<(IDX_TO_NANOGRAPH_HWIO_CONTROL_MSB-IDX_TO_NANOGRAPH_HWIO_CONTROL_LSB+1))-1)

    /*
    The graph hold a table of uint32_t "nanograph_io_control" 
      arcID, direction, servant/commander, 
      set pointer/copy, buffer allocation, Domain, index to the AL
    
      example with a platform using maximum 10 IOs and a graph using "SENSOR_0" and "_DATA_OUT_0"
          #define IO_PLATFORM_data_sink        0 
          #define IO_PLATFORM_DATA_IN_1        1 
          #define IO_PLATFORM_ANALOG_SENSOR_0  2   X
          #define IO_PLATFORM_MOTION_IN_0      3 
          #define IO_PLATFORM_AUDIO_IN_0       4 
          #define IO_PLATFORM_2D_IN_0          5 
          #define IO_PLATFORM_LINE_OUT_0       6 
          #define IO_PLATFORM_GPIO_OUT_0       7 
          #define IO_PLATFORM_GPIO_OUT_1       8 
          #define IO_PLATFORM_DATA_OUT_0       9   X
      The binary grap will have nanograph_io_control with 2 indexes "SENSOR_0" and "_DATA_OUT_0"
    */ 

#define NANOGRAPH_IOFMT_SIZE_W32 4u    /* four word for IO controls : one for the scheduler, three for IO settings */

#define MAX_IO_FUNCTION_PLATFORM 127u /* table of functions : platform_io[MAX_IO_FUNCTION_PLATFORM] */
                              
#define IO_COMMAND_SET_BUFFER 0u    /* graph arc base address is set by the IO driver's buffer address */
#define IO_COMMAND_DATA_COPY  1u    /* copy to graph arc */
                              
#define RX0_TO_GRAPH          0u
#define TX1_FROM_GRAPH        1u
                              
#define IO_IS_COMMANDER0      0u
#define IO_IS_SERVANT1        1u

#define IOFMT0 0u                   /* first word used by the scheduler */

#define    FWIOIDX_IOFMT0_MSB 31u   /*                    tmpi = RD(*pio_control, FWIOIDX_IOFMT0) */
#define    FWIOIDX_IOFMT0_LSB 16u   /* 16 HW FW IDX  */
#define  BUFFALLOC_IOFMT0_MSB 15u   
#define  BUFFALLOC_IOFMT0_LSB 15u   /* 1  declare buffer in graph memory space */
#define  SETCALLBK_IOFMT0_MSB 14u   
#define  SETCALLBK_IOFMT0_LSB 14u   /* 1  IO settings parameters are used as callback parameters */
#define  SET0COPY1_IOFMT0_MSB 13u   
#define  SET0COPY1_IOFMT0_LSB 13u   /* 1  command_id IO_COMMAND_SET_BUFFER / IO_COMMAND_DATA_COPY */
#define   SERVANT1_IOFMT0_MSB 12u   
#define   SERVANT1_IOFMT0_LSB 12u   /* 1  1=IO_IS_SERVANT1 */
#define     RX0TX1_IOFMT0_MSB 11u   /*    direction of the stream */
#define     RX0TX1_IOFMT0_LSB 11u   /* 1  0 : to the graph    1 : from the graph */
#define       _____IOFMT0_MSB 10u 
#define       _____IOFMT0_LSB  6u   /*  5  */
#define    IOARCID_IOFMT0_MSB  5u 
#define    IOARCID_IOFMT0_LSB  0u   /*  6  Graph IO ARC*/

#define MAX_GRAPH_IO_IDX (1 << (IOARCID_IOFMT0_MSB - IOARCID_IOFMT0_LSB + 1))
#define MAX_IO_ONGOING_BYTES (MAX_GRAPH_IO_IDX/8) // asynchronous/slave IOs managed by this instance/processor

#define IO_SETTING_OFFSET 1         /* IO settings are starting on index [IOFMT1] */
#define IOFMT1 1u                   /* domain-specific controls */
#define     unused_IOFMT1_MSB 11u   
#define     unused_IOFMT1_LSB  6u   /* 26  */
#define SAMPLINGRT_IOFMT1_MSB  5u 
#define SAMPLINGRT_IOFMT1_LSB  0u   /* 6  sampling rate selection from the manifest options */

#define IOFMT2 2u                   /* domain-specific controls */
#define IOFMT3 3u                   /* domain-specific controls */

/*================================================================================================================*/    

#define U(x) ((uint32_t)(x))
#define U8(x) ((uint8_t)(x))


#define NANOGRAPH_PTRPHY      4u    
#define NANOGRAPH_PTR27B      5u    


/* constants for uint8_t itoa(char *s, int32_t n, int base) 
* string conversions to decimal and hexadecimal */

#define C_BASE2 2u
#define C_BASE10 10u
#define C_BASE16 16u

//#if __GNUC__ || __clang__ || __INTEL_LLVM_COMPILER
//#define UNUSED __attribute__((unused))
//#else
//#define UNUSED /**/
//#endif

/*================================================================================================================*/    
/*                                            FLOAT                                                               */
/*================================================================================================================*/    

#define FADD(tmp, src1, src2)  tmp=src1+src2
#define FSUB(tmp, src1, src2)  
#define FMUL(tmp, src1, src2)  
#define FDIV(tmp, src1, src2)  
#define FMAX(tmp, src2, src1)  
#define FMIN(tmp, src2, src1)  
#define FAMAX(tmp, src2, src1) 
#define FAMAX(tmp, src2, src1) 
#define FTESTEQU(tmp, dst) 0
#define FTESTLEQ(tmp, dst) 0
#define FTESTLT(tmp, dst)  0
#define FTESTNEQ(tmp, dst) 0
#define FTESTGEQ(tmp, dst) 0
#define FTESTGT(tmp, dst)  0

#define F2I(src) ((uint32_t)(src))
#define I2F(src) ((float_t)(src))

/*================================================================================================================*/    

#endif /* cNANOGRAPH_CONST_H */
/*
 * -----------------------------------------------------------------------
 */
#ifdef __cplusplus
}
#endif
    

