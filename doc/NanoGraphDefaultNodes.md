| Table of content |
| --------------------- |
| [How to start](#How-to-start) |
|                               |
|                               |



# List of pre-installed nodes (development)



---------------------------------------------------------------------------------------

| ID   | Name                     | Comments                                               |
| ---- | ------------------------ | ------------------------------------------------------ |
| 1    | arm_stream_script        | byte-code interpreter index "arm_stream_script_INDEX"  |
| 2    | arm_stream_router        | router, mixer, rate and format converter               |
| 3    | arm_stream_amplifier     | amplifier mute and un-mute with ramp and delay control |
| 4    | arm_stream_filter        | cascade of filters                                     |
| 5    | arm_stream_modulator     | signal generator with modulation                       |
| 6    | arm_stream_demodulator   | signal demodulator frequency estimator                 |
| 7    | arm_stream_filter2D      | filter / rescale / zoom / extract / merge / rotate     |
| 8    | sigp_stream_detector     | signal detection in noise                              |
| 9    | sigp_stream_detector2D   | image activity detection                               |
| 10   | sigp_stream_resampler    | asynchronous high-quality sample-rate converter        |
| 11   | sigp_stream_compressor   | raw data compression with adaptive prediction          |
| 12   | sigp_stream_decompressor | raw data decompression                                 |
| 13   | bitbank_jpg_encoder      | jpeg encoder                                           |
| 14   | elm_jpg_decoder          | TjpgDec                                                |


## arm_stream_script (tbd)

Scripts are nodes interpreted from byte codes declared in the indexed SCRIPTS section of the graph or inlined in the parameter section of the node arm_stream_script.  
The first scripts are simple code sequences used as subroutines or called using the node_script index.

Script nodes manage data RAM locations in a shared arc used by all scripts.  
Instance registers, stack parameters, and constants are placed after the byte codes.

The default memory configuration is shared, meaning that buffers associated with scripts share the same memory buffer.

To assign individual static memory to a script, the script_mem_shared parameter must be set to 0.

Special functions activated through Syscall and conditional instructions include the following.

- If-then execution, which conditionally executes blocks of nodes based on script decisions such as FIFO content or debug register values.  
- Loop execution, which repeats a list of nodes multiple times for cache efficiency and small frame sizes.  
- End-thread execution, where the script is the last executed before a list of low-priority nodes.  In this case, ongoing IO operations can be flushed and control can return to the application.



```
node arm_stream_script 1  ; script (instance) index           
    script_stack      12  ; size of the stack in word64      
    script_parameter  30  ; size of the parameter/heap in word32
    script_mem_shared  1  ; private memory (0) or shared(1)  
    script_mem_map     0  ; mapping to VID #0 (default)      

    script_code  
        r1 = add r2 3       ; r1 = add r2 3
      label AAA         
        set r2 graph sigp_stream_detector_0         
        r0 = 0x412              ; r0 = STREAM_SET_PARAMETER(2)
        set r3 param BBB        ; set r3 param BBB 
        sp0 = 1                 ; push 1 Byte (threshold size in BBB)
        Syscall 1 r2 r0 r3 sp0  ; Syscall NODE(1) r2(cmd=set_param) r0(set) r3(data) 
        return              ; return
    end
        
    script_parameters      0    
        1  u8 ;  34             
        2  u32; 0x33333333 0x444444444 
        label BBB              
        1  u8 ;  0x55           
        1  u32;  0x66666666     
    end

```

----------------------------------------------------------------------------------------

## arm_stream_router (tbd)

**Operation** 

This node receives up to four input streams (arcs) and generates up to four output streams.  
Each stream may be multichannel.

The format of the streams is known by the node during the reset and set-parameter commands.

Input streams may be asynchronous and include time stamps.  
Output streams are isochronous with other graph streams and have a defined sampling rate.

The first parameters specify the number of arcs and identify the input arc to be used with high quality of service (HQoS), or −1 if none is used.  
These parameters are followed by a list of routing and mixing rules.

When an HQoS arc is defined, data movement is aligned to that arc in the time domain.  
Other arcs are synchronized accordingly, and data is interpolated or zeroed in case of flow issues.

When no HQoS arc is defined, the node evaluates all input and output arcs and determines the minimum amount of data that can be processed consistently across all arcs in the time domain.

**Use-cases**

The following use-cases can be combined:

1. Router, deinterleaving, interleaving, channels recombination: the input arc data is processed deinterleaved, and the output arc is the result of recombination of any input arc. Audio example with two stereo input arcs using 5ms and 12ms frame lengths, recombined to create a stereo stream interleaved output using 20ms frame length, the left channel from the first arc and the left channel of the second arc.

2. Router and mixer with smoothed gain control: the output arc data can result from the weighted mix of input arcs. The applied gain can be changed on the fly. The slope of the time taken to the desired gain is controlled. Audio example: a mono output arc is computed from the combination of two stereo input arc, by mixing the four input channels with a factor 0.25 applied in the mixer.

3. Router and raw data conversion. The raw formats can be converted to any other format in this list : int16, int32, int64, float16, float32, float64.

4. Router and sampling-rate conversion of isochronous streams (input streams have a determined and independent sampling-rate). Audio example: input streams sampled at 44100Hz is converted to 48000Hz + 0.026Hz for drift compensation (the sampling-rate information, and all the details of the arc's data format, is shared by the graph scheduler during the reset phase of the nodes).

5. Router and conversion of asynchronous streams using time-stamps to an isochronous stream with a determined sampling-rate. Motion sensor example: an accelerometer is sampled at 200Hz (5ms period)  with +/- 1ms jitter sampling time uncertainty. The samples are provided with an accurate time-stamp in float32 format of time differences between samples (or float64 for absolute time reference to Jan 1st 2025). The output samples are delivered resampled at 410Hz with no jitter.

6. Router of data needing a time synchronization at sample or frame level. In this use-case the node waits the input samples are arriving within a time window before delivering an output frame. Example with motor control and the capture of current and voltage on two input arcs: it is important to send time-synchronized pairs of data. The command [node_script “index”](node_script-"index") is used to call a script checking the arrival of current and voltage with their respective time-stamps (logged in the arc descriptors), the scripts check the arrival of data within a time and release execution of the router when conditions are met.

7. Router of streams generated from different threads. The problem is to avoid on multiprocessing devices one channel to be delivered to the final mixer ahead and desynchronized from the others. This problem is solved with an external script controlling the system time like in the use-case 6.

   

**Parameters**

The list of routing and mixing information is :

- index of the input arc (<= 4)
- index of the channels (1 Byte to 31 Bytes)
- index of the output destination arc (<=4)
- index of the channels (1 Byte to 31 Bytes) 
- mixer gain to apply (fp32) and convergence speed (fp32)



Example with the router with two stereo input arcs and two output arcs at 16kHz sampling rate. The first output arc is mono and the sum of the input channels of the first input, upsampled to 48kHz in float32. The second arc is a stereo combination of the two left channels of the input arcs, at the same sampling rate but with 32bits/sample instead of 16bits for the input.

```
format_index            0               ; router input
format_frame_length     16              ; 
format_raw_data         3               ; STREAM_S16
format_nbchan           2               ; stereo
format_sampling_rate    hertz 16000     ; 
format_interleaving     0               ; raw data interleaving
;
format_index            1               ; router output
format_frame_length     96              ; 
format_raw_data         1               ; STREAM_FP32
format_nbchan           1               ; mono
format_sampling_rate    hertz 48000     ; 
format_interleaving     0               ; raw data interleaving
;
format_index            2               ; router output
format_frame_length     32              ; 
format_raw_data         4               ; STREAM_S32
format_nbchan           2               ; stereo
format_sampling_rate    hertz 16000     ; 
format_interleaving     0               ; raw data interleaving
;----------------------------------------------------------------------
stream_io_graph         0   2           ; Platform index 2
stream_io_format        0               ; 
stream_io_setting       2 0 0           ; io_sampling_rate hertz {1 8e3 16e3 48e3}, '2' to set 16kHz
;
stream_io_graph         1   4           ; Platform index 4 microphone 
stream_io_format        0               ; 
stream_io_setting       2 0 0           ; io_sampling_rate hertz {1 8e3 16e3 48e3}, '2' to set 16kHz

stream_io_graph         2   9           ; 
stream_io_format        1               ; 
stream_io_setting       3 0 0           ; io_sampling_rate hertz {2 16e3 44.1e3 48000}, '3' to set 48kHz

stream_io_graph         3   8           ; Platform index 8 
stream_io_format        2               ; 
stream_io_setting       3 0 0           ; io_sampling_rate hertz {2 16e3 44.1e3 48000}, '3' to set 16kHz
;----------------------------------------------------------------------
; 
                 +----------------------------+                    
  Stereo s16     |F0 ---.   F7 +---------+  F4|  Mono 48kHz fp32 Format 1
-----arc0------->|F1    |   F8 |  Mixer  +----+---arc2-------------------> 
Format 0 16kHz   |      |      +---------+    | sum of Left arc0+Right arc0
                 |      |                     |
  Stereo s16     |F2    '-------------------F5| Stereo 16kHz, s32 Format 2 
-----arc1------->|F3 -----------------------F6+---arc3-------------------> 
Format 0 16kHz   +----------------------------+   Left(arc0)   Left(arc1) 

```



    node arm_stream_router  0 
        node_malloc_add    680 1    ; 2 internal FIFO (20 bytes each) on Segment-1 (default = 1 FIFO)
                                    ; 9 FIFOs (64Bytes each) 
                                    ; + 1 MIXERs (40Bytes) 
                                    ; + 2 INTERPOLATORs (32Bytes) 
                                    ; = 680
        node_preset         0
        node_parameters     0       ; TAG = "all parameters"
            1  u8;  2               ; nb input arcs
            1  u8;  2               ; nb output arcs
            2  u8;  255 255         ; arc (in/out) used with HQOS: none
            1  u8;  9               ; nb FIFO
    ;
    ; FIFO declarations
    ;   DIRECT ACCESS TO INPUT FIFO
            4  u8;  0 0   0 0         ; FIFO 0  input  left  arc-0 sub-channel-0 
            4  u8;  1 0   0 1         ; FIFO 1  input  right arc-0 sub-channel-1
            4  u8;  2 0   1 0         ; FIFO 2  input  left  arc-1 sub-channel-0 
            4  u8;  3 0   1 1         ; FIFO 3  input  right arc-1 sub-channel-1 
    ;   DIRECT ACCESS TO OUTPUT FIFO
            4  u8;  4 1   2 0         ; FIFO 4  output mono  arc-2 
            4  u8;  5 1   3 0         ; FIFO 5  output left  arc-3 sub-channel-0 
            4  u8;  6 1   3 1         ; FIFO 6  output right arc-3 sub-channel-1 
    ;   STATIC INTERNAL FIFO
            4  u8;  7 2   2 0         ; FIFO 7  Mixer input0, arc-2(FS) FP32
            4  u8;  8 2   2 0         ; FIFO 8  Mixer input1, arc-2(FS) FP32
    ;
    ; MICROCODE OPERATORS: copy to output(0), interpolate(1), mixer(2), copy to internal fifo(3) 
    ;   move data to prepare {Left arc0 + Right arc0}     
    ;              OP  PARAM
            4  u8;  17 0 7  0       ; interpolate FIFO 0 => FIFO 7 don't increment read index 
            4  u8;  17 1 8  1       ; interpolate FIFO 1 => FIFO 8 increment read index 
            4  u8;  33 7 8 4        ; Mixer FIFO 7 + FIFO 8 => FIFO 4 (mono output)
    
    ;   direct move to arc3 stereo with raw format conversion
            4  u8;  1  0 2  1       ; copy FIFO 0 => FIFO 5, Left arc0  now increment read index
            4  u8;  1  3 6  1       ; copy FIFO 3 => FIFO 6, Right arc1 increment read index
    ;
    end
Operations :

- when receiving the reset command: compute the time granularity for the processing, check if bypass are possible (identical sampling rate on input and output arcs).
- check all input and output arcs to know which is the amount of data (in the time domain) which can me routed and split in "time granularity" chunks. Clear the mixer buffers.

Loop with "time granularity" increments :

- copy the input arcs data in internal FIFO in fp32 format, deinterleaved, with time-stamps attached to each samples.
- use Lagrange polynomial interpolation to resample the FIFO to the output rate. The interpolator is preceded by a an adaptive low-pass filter removing high-frequency content when the estimated input sampling rate higher than the output rate.
- recombination and mixing of resampled data using a programmable gain and smoothed ramp-up to the target value.
- raw data format conversion to the output arc

----------------------------------------------------------------------------------------

## arm_stream_amplifier (TBD)

Operation : rescale and control of the amplitude of the input stream with controlled time of ramp-up/ramp-down. 
The gain control “mute” is used to store the current gain setting, being reloaded with the command “unmute”
Option : either the same gain/controls for all channels or list of parameters for each channel

Parameters :  new gain/mute/unmute, ramp-up/down slope, delay before starting the slope. 
Use-cases :
    Features : adaptive gain control (compressor, expander, AGC) under a script control with energy polling 
    Metadata features : "saturation occurred" "energy"
    Mixed-Signal glitches : remove the first seconds of an IR sensor until it was self-calibrated (same for audio Class-D)

parameters of amplifier (variable size): 
TAG_CMD = 1, uint8_t, 1st-order shifter slope time (as stream_mixer, 0..75k samples)
TAG_CMD = 2, uint16_t, desired gain FP_8m4e, 0dB=0x0805
TAG_CMD = 3, uint8_t, set/reset mute state
TAG_CMD = 4, uint16_t, delay before applying unmute, in samples
TAG_CMD = 5, uint16_t, delay before applying mute, in samples

Slopes of rising and falling gains, identical to all channels
slope coefficient = 0..15 (iir_coef = 1-1/2^coef = 0 .. 0.99)
Convergence time to 90% of the target in samples:
 slope   nb of samples to converge
     0           0
     1           3
     2           8
     3          17
     4          36
     5          73
     6         146
     7         294
     8         588
     9        1178
    10        2357
    11        4715
    12        9430
    13       18862
    14       37724
    15       75450
    convergence in samples = abs(round(1./abs(log10(1-1./2.^[0:15])'))

 Operation : applies vq = **interp1**(x,v,xq) 
 Following https://fr.mathworks.com/help/matlab/ref/interp1.html
   linear of polynomial interpolation (implementation)
 **Parameters : X,V vectors, size max = 32 points**

no preset ('0')

Or used as compressor / expander using long-term estimators instead of sample-based estimator above.

```
node arm_stream_rescaler 0

    parameters     0             ; TAG   "load all parameters"
        
;               input   output
        2; f32; -1      1
        2; f32;  0      0       ; this table creates the abs(x) conversion
        2; f32;  1      1
    end  
end
```

---------------------------------------------------------------------------------------

## 

```
node  arm_stream_amplifier 0


    parameters     0             ; TAG   "load all parameters"
        1  i8;  1           load only rising/falling coefficient slope
        1 h16;  805         gain -100dB .. +36dB (+/- 1%)
        1  i8;  0           muted state
        2 i16;  0 0         delay-up/down
    end  
end
```

----------------------------------------------------------------------------------------



## arm_stream_filter

Operation : receives one multichannel stream and produces one filtered multichannel stream. 
Parameters : biquad filters coefficients used in cascade. Implementation is 2 Biquads max.
(see www.w3.org/TR/audio-eq-cookbook)
Presets:
#0 : bypass
#1 : offset removal filter 
#2 : Median filter, 5 points
#3 : Low pass filter
#4 : High pass filter 
#5 : Peaking  filter
#6 : Bandpass filter
#7 : Notch filter
#8 : Low shelf filter
#9 : High shelf filter
#10: All pass filter
#11: Dithering filter

parameter of filter : 

Normalized frequency f0/FS default = 0.25, 

Q factor default = 1.414

Default gain = 4  (12dB)



```
node arm_stream_filter 0         node subroutine name + instance ID
    node_preset         1        ; parameter preset used at boot time, default = #0
    parameters          0        ; TAG   "load all parameters"
        1  u8;  2       Two biquads
        1  i8;  0       unused
        5 f32; 0.284277f 0.455582f 0.284277f 0.780535f -0.340176f  
        5 f32; 0.284277f 0.175059f 0.284277f 0.284669f -0.811514f 
        ; or  include    1 arm_stream_filter_parameters_x.txt      (path + file-name)
    end
end
```

----------------------------------------------------------------------------------------

## arm_stream_modulator (TBD)

 Operation : sine, noise, square, saw tooth with amplitude or frequency modulation
 use-case : ring modulator, sweep generation with a cascade of a ramp generator and
    a frequency modulator

see https://www.pjrc.com/teensy/gui/index.html?info=AudioSynthWaveform 

```
u8  wave type 1=cosine 2=square 3=white noise 4=pink noise 
    5=sawtooth 6=triangle 7=pulse
    8=prerecorded pattern playback from arc 
    9=sigma-delta with OSR control for audio on PWM ports or 8b DAC
    10=PWM 11=ramp 12=step
u8  modulation type, 0:amplitude, 1:frequency, 2:FSK 
u8  modulation, 0:none 1=from arc bit stream

f32 modulation amplitude
f32 offset
f32 wave frequency [Hz]
f32 starting phase,[-pi .. +pi]
f32 modulation y=ax+b, x=input data, index (a) and offset (b)
f32 modulation frequency [Hz] separating two data bits/samples from the arc
```

```
node arm_stream_modulator (i)

    parameters     0             ; TAG   "load all parameters"
        
        1  u8;  1       sinewave
        2 h16;  FFFF 0  full-scale, no offset
        1 f32;  1200    1200Hz
        1 s16;  0       initial phase
        2  u8;  1 1     frequency modulation from bit-stream
        2 h16;  8000 0  full amplitude modulation with sign inversion of the bit-stream
        1 f32;  300     300Hz modulation => (900Hz .. 1500Hz modulation)
    end
end
```

----------------------------------------------------------------------------------------

## arm_stream_demodulator (TBD)

 Operation : decode a bit-stream from analog data. Use-case: IR decoder, CAN/UART on SPI/I2S audio.

Next: sinewave frequency recognition for alarm detector.

 Parameters : clock and parity setting or let the algorithm discover the frame setting after some time. https://en.wikipedia.org/wiki/Universal_asynchronous_receiver-transmitter and https://github.com/Arduino-IRremote/Arduino-IRremote https://www.arduinolibraries.info/categories/display https://www.arduinolibraries.info/categories/display

presets control :
#1 .. 10: provision for  demodulators 

Metadata information can be extracted with the command "parameter-read":
TAG_CMD = 1 read the signal amplitude
TAG_CMD = 2 read the signal to noise ratio

```
node 
    arm_stream_demodulator (i)
    parameters     0             ; TAG   "load all parameters"
        
        2  i8; 2 2          nb input/output arcs
        4 i16; 0 0 2 0      move arc0,chan0, to arc2,chan0
    end
end
```


----------------------------------------------------------------------------------------

## arm_stream_filter2D (TBD)

Filter, rescale/zoom/extract, rotate, exposure compensation, background removal. 

Channel mixer : insert a portion of the processed image in a larger frame buffer.

Operation : 2D filters 
Parameters : spatial and temporal filtering, decimation, distortion, color mapping/log-effect

presets:
#1 : bypass

parameter of filter : 

```
node arm_stream_filter2D   (i)

	TBD
end
```


----------------------------------------------------------------------------------------



## sigp_stream_detector

Operation : provides a boolean output stream from the detection of a rising 
edge above a tunable signal to noise ratio. 
A tunable delay allows to maintain the boolean value for a minimum amount of time 
Use-case example 1: debouncing analog input and LED / user-interface.
Use-case example 2: IMU and voice activity detection (VAD)
Parameters : time-constant to gate the output, sensitivity of the use-case

presets control
#1 : no HPF pre-filtering, fast and high sensitivity detection (button debouncing)
#2 : VAD with HPF pre-filtering, time constants tuned for ~10kHz
#3 : VAD with HPF pre-filtering, time constants tuned for ~44.1kHz
#4 : IMU detector : HPF, slow reaction time constants
#5 : IMU detector : HPF, fast reaction time constants

Metadata information can be extracted with the command "TAG_CMD" from parameter-read:
0 read the floor noise level
1 read the current signal peak
2 read the signal to noise ratio

```
node arm_stream_detector 0               node name  + instance ID
    preset              1               parameter preset used at boot time, default = #0
end
```


----------------------------------------------------------------------------------------

## sigp_stream_detector2D (TBD)

Motion and pattern detector. Counting moving object and build a text report (object sizes, shape, speed, color, ..) to be later processed by SLM on the application processor.

Operation : detection of movement(s) and computation of the movement map
Parameters : sensitivity, floor-noise smoothing factors
Metadata : decimated map of movement detection

```
node arm_stream_detector2D (i)

TBD

end
```

----------------------------------------------------------------------------------------

## sigp_stream_resampler (TBD)

Operation : high quality conversion of multichannel input data rate to the rate of the output arcs 

  + asynchronous rate conversion within +/- 1% adjustment
  + 

SSRC synchronous rate converter, FS in/out are exchanged during STREAM_RESET
ASRC asynchronous rate converter using time-stamps (in) to synchronous FS (out) pre-LP-filtering tuned from Fout/Fin ratio + Lagrange polynomial interpolator

drift compensation managed with STREAM_SET_PARAMETER command:
TAG_CMD = 0 bypass
TAG_CMD = 1 rate conversion

The script associated to the node is used to read the in/out arcs filling state
    to tune the drift control

``` 
node arm_stream_resampler (i)

    parameters     0             ; TAG   "load all parameters"
        
        2  i8; 2 2          nb input/output arcs
        4 i16; 0 0 2 0      move arc0,chan0, to arc2,chan0
    end
end

```

----------------------------------------------------------------------------------------

## sigp_stream_compressor (TBD)

Operation : wave compression using IMADPCM(4bits/sample)
Parameters : coding scheme 

presets (provision codes):

- 1 : coder IMADPCM
- 2 : coder LPC
- 4 : coder CVSD for BT speech 
- 5 : coder SBC
- 6 : coder MP3

```
node 
    arm_stream_compressor 0

    parameters     0             ; TAG   "load all parameters"
        4; i32; 0 0 0 0     provision for extra parameters in other codecs
    end
end
```

----------------------------------------------------------------------------------------

## sigp_stream_decompressor (TBD)

Operation : decompression of encoded data
Parameters : coding scheme and a block of 16 parameter bytes for codecs, VAD threshold and silence frame format (w/wo time-stamps)

​	dynamic parameters : pause, stop, fast-forward x2 and x4.



    WARNING : if the output format can change (mono/stereo, sampling-rate, ..)
        the variation is detected by the node and reported to the scheduler with 
        "STREAM_SERVICE_INTERNAL_FORMAT_UPDATE", the "uint32_t *all_formats" must be 
        mapped in a RAM for dynamic updates with "COPY_CONF_GRAPH0_COPY_ALL_IN_RAM"
    
    Example of data to share with the application
        outputFormat: AndroidOutputFormat.MPEG_4,
        audioEncoder: AndroidAudioEncoder.AAC,
        sampleRate: 44100,
        numberOfChannels: 2,
        bitRate: 128000,

presets provision

- 1 : decoder IMADPCM
- 2 : decoder LPC
- 3 : MIDI player
- 4 : decoder CVSD for BT speech 
- 5 : decoder SBC
- 6 : decoder MP3

```
node arm_stream_decompressor 0

    parameters     0             ; TAG   "load all parameters"
        4; i32; 0 0 0 0     provision for extra parameters in other codecs
    end
end
```

----------------------------------------------------------------------------------------

## bitbank_jpg_encoder (TBD)

From "bitbank"

https://github.com/google/jpegli/tree/main

https://opensource.googleblog.com/2024/04/introducing-jpegli-new-jpeg-coding-library.html

## eml_tjpg_decoder (TBD)

From "EML"

Use-case : images decompression, pattern generation.



