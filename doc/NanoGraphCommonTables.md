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



# Common tables

## Stream format Words 0,1,2

Words 0, 1 and 2 are common to all domains :

| Word | Bits   | Comments                                                     |
| ---- | ------ | :----------------------------------------------------------- |
| 0    | 0..24  | frame size in Bytes (including the time-stamp field) + extension |
| 0    | 25..31 | reserved                                                     |
| 1    | 0..4   | nb channels-1 [1..32 channels]                               |
| 1    | 5      | 0 for raw data interleaving (for example L/R audio or IMU stream), 1 for a pointer to the first channel, next channel address is computed by adding the frame size divided by the number of channels |
| 1    | 6..7   | time-stamp format of the stream applied to each frame :<br />0: no time-stamp <br />1: absolute time reference  <br />2: relative time from previous frame  <br />3: simple counter |
| 1    | 8..9   | time-stamp size on 16bits 32/64/64-ISO format                |
| 1    | 10..15 | raw data format                                              |
| 1    | 16..19 | domain of operations (see list below)                        |
| 1    | 20..21 | extension of the size and arc descriptor indexes by a factor 1/64/1024/16k |
| 1    | 22..26 | sub-type (see below) for pixel type and analog formats       |
| 2    | 0..7   | reserved                                                     |
| 2    | 8..31  | IEEE-754 FP32 truncated to 24bits (S-E8-M15), 0 means "asynchronous" |

## Stream format Word 1

Word 3 of "Formats" holds specific information of each domain.

### Audio stream format

Audio channel mapping is encoded on 20 bits. For example a stereo channel holding "Back Left" and "Back Right" will be encoded as 0x0030.

| Channel name          | Name | Bit  |
| --------------------- | ---- | ---- |
| Front Left            | FL   | 0    |
| Front Right           | FR   | 1    |
| Front Center          | FC   | 2    |
| Low Frequency         | LFE  | 3    |
| Back Left             | BL   | 4    |
| Back Right            | BR   | 5    |
| Front Left of Center  | FLC  | 6    |
| Front Right of Center | FRC  | 7    |
| Back Center           | BC   | 8    |
| Side Left             | SL   | 9    |
| Side Right            | SR   | 10   |
| Top Center            | TC   | 11   |
| Front Left Height     | TFL  | 12   |
| Front Center Height   | TFC  | 13   |
| Front Right Height    | TFR  | 14   |
| Rear Left Height      | TBL  | 15   |
| Rear Center Height    | TBC  | 16   |
| Rear Right Height     | TBR  | 17   |
| Channel 19            | C19  | 18   |
| Channel 20            | C20  | 19   |

### Motion

Motion sensor channel mapping (w/wo the temperature)

| Motion sensor data | Code |
| ------------------ | ---- |
| only accelerometer | 1    |
| only gyroscope     | 2    |
| only magnetometer  | 3    |
| A + G              | 4    |
| A + M              | 5    |
| G + M              | 6    |
| A + G + M          | 7    |

### 2D

Format of the images in pixels: height, width, border. The "extension" bit-field of the word -1 allow managing larger images.

| 2D  data            | bits range | comments                                                     |
| ------------------- | ---------- | ------------------------------------------------------------ |
| smallest dimension  | 0 - 11     | the largest dimension is computed with (frame_size - time_stamp_size)/smallest_dimension |
| image ratio         | 12 - 14    | TBD =0, 1/1 =1, 4/3 =2, 16/9 =3, 3/2=4                       |
| image format        | 15         | 0 for horizontal, 1 for vertical                             |
| image sensor border | 17 - 18    | 0 .. 3 pixels border                                         |
| interlace mode      | 2          | progressive, interleaved, mixed, alternate                   |
| chroma              | 2          | jpeg, mpeg2, dv, none                                        |
| color space         | 2          | ITU-BT.601, ITU-BT.709, SMPTE 240M                           |
| invert pixels       | 1          | for  test/debug                                              |
| brightness          | 4          | display control                                              |
| contrast            | 4          | display control                                              |



------

## Data Types

Raw data types

| TYPE              | CODE | COMMENTS                                                |
| ----------------- | ---- | :------------------------------------------------------ |
| STREAM_DATA_ARRAY |  0   | stream_array : `{ 0NNN TT 00 }` number, type            |
| STREAM_FP32       |  1   | ` Seeeeeee.mmmmmmmm.mmmmmmmm..`  FP32                   |
| STREAM_FP64       |  2   | ` Seeeeeee.eeemmmmm.mmmmmmm ...`  double                |
| STREAM_S16        |  3   | ` Sxxxxxxx.xxxxxxxx` 2 bytes per data                   |
| STREAM_S32        |  4   | one long word                                           |
| STREAM_S2         |  5   | `Sx` two bits per data                                  |
| STREAM_U2         |  6   | `uu`                                                    |
| STREAM_S4         |  7   | `Sxxx` four bits per data                               |
| STREAM_U4         |  8   | `xxxx`                                                  |
| STREAM_FP4_E2M1   |  9   | `Seem`  micro-float [8 .. 64]                           |
| STREAM_FP4_E3M0   | 10   | `Seee`   [8 .. 512]                                     |
| STREAM_S8         | 11   | ` Sxxxxxxx`  eight bits per data                        |
| STREAM_U8         | 12   | ` xxxxxxxx`  ASCII char, numbers..                      |
| STREAM_FP8_E4M3   | 13   | ` Seeeemmm`  NV tiny-float [0.02 .. 448]                |
| STREAM_FP8_E5M2   | 14   | ` Seeeeemm`  IEEE-754 [0.0001 .. 57344]                 |
| STREAM_U16        | 15   | ` xxxxxxxx.xxxxxxxx`  Numbers, UTF-16 characters        |
| STREAM_FP16       | 16   | ` Seeeeemm.mmmmmmmm`  half-precision float              |
| STREAM_BF16       | 17   | ` Seeeeeee.mmmmmmmm`  bfloat                            |
| STREAM_S23        | 18   | ` Sxxxxxxx.xxxxxxxx.xxxxxxxx`  24bits 3 bytes per data  |
| STREAM_S23_32     | 19   | ` SSSSSSSS.Sxxxxxxx.xxxxxxxx.xxxxxxx`  4 bytes per data |
| STREAM_U32        | 20   | ` xxxxxxxx.xxxxxxxx.xxxxxxxx.xxxxxxxx`  UTF-32, ..      |
| STREAM_CS16       | 21   | ` Sxxxxxxx.xxxxxxxx+Sxxxxxxx.xxxxxxxx (I Q)`            |
| STREAM_CFP16      | 22   | ` Seeeeemm.mmmmmmmm+Seeeeemm.. (I Q)`                   |
| STREAM_S64        | 23   | long long 8 bytes per data                              |
| STREAM_U64        | 24   | unsigned 64 bits                                        |
| STREAM_CS32       | 25   | ` Sxxxxxxx.xxxxxxxx.xxxxxxxx.xxxxxxxx Sxxxx..`          |
| STREAM_CFP32      | 26   | ` Seeeeeee.mmmmmmmm.mmmmmmmm.m..+Seee..`  (I Q)         |
| STREAM_FP128      | 27   | ` Seeeeeee.eeeeeeee.mmmmmmm ...`  quadruple precision   |
| STREAM_CFP64      | 28   | fp64 + fp64 (I Q)                                       |
| STREAM_FP256      | 29   | ` Seeeeeee.eeeeeeee.eeeeemm ...`  octuple precision     |
| STREAM_WGS84      | 30   | `<--LAT 32B--><--LONG 32B-->`                           |
| STREAM_HEXBINARY  | 31   | UTF-8 lower case hexadecimal byte stream                |
| STREAM_BASE64     | 32   | RFC-2045 base64 for xsd:base64Binary XML data           |
| STREAM_STRING8    | 33   | UTF-8 string of char terminated by 0                    |
| STREAM_STRING16   | 34   | UTF-16 string of char terminated by 0                   |

------



## Units

| NAME                | CODE | UNIT                              | COMMENT                                        |
| ------------------- | ---- | --------------------------------- | ---------------------------------------------- |
| _ANY                | 0    |                                   | any                                            |
| _METER              | 1    | m                                 | meter                                          |
| _KGRAM              | 2    | kg                                | kilogram                                       |
| _GRAM               | 3    | g                                 | gram                                           |
| _SECOND             | 4    | s                                 | second                                         |
| _AMPERE             | 5    | A                                 | ampere                                         |
| _KELVIB             | 6    | K                                 | kelvin                                         |
| _CANDELA            | 7    | cd                                | candela                                        |
| _MOLE               | 8    | mol                               | mole                                           |
| _HERTZ              | 9    | Hz                                | hertz                                          |
| _RADIAN             | 10   | rad                               | radian                                         |
| _STERADIAN          | 11   | sr                                | steradian                                      |
| _NEWTON             | 12   | N                                 | newton                                         |
| _PASCAL             | 13   | Pa                                | pascal                                         |
| _JOULE              | 14   | J                                 | joule                                          |
| _WATT               | 15   | W                                 | watt                                           |
| _COULOMB            | 16   | C                                 | coulomb                                        |
| _VOLT               | 17   | V                                 | volt                                           |
| _FARAD              | 18   | F                                 | farad                                          |
| _OHM                | 19   | Ohm                               | ohm                                            |
| _SIEMENS            | 20   | S                                 | siemens                                        |
| _WEBER              | 21   | Wb                                | weber                                          |
| _TESLA              | 22   | T                                 | tesla                                          |
| _HENRY              | 23   | H                                 | henry                                          |
| _CELSIUSDEG         | 24   | Cel                               | degrees Celsius                                |
| _LUMEN              | 25   | lm                                | lumen                                          |
| _LUX                | 26   | lx                                | lux                                            |
| _BQ                 | 27   | Bq                                | becquerel                                      |
| _GRAY               | 28   | Gy                                | gray                                           |
| _SIVERT             | 29   | Sv                                | sievert                                        |
| _KATAL              | 30   | kat                               | katal                                          |
| _SQUAREMETER        | 31   | m2                                | square meter (area)                            |
| _CUBICMETER         | 32   | m3                                | cubic meter (volume)                           |
| _LITER              | 33   | l                                 | liter (volume)                                 |
| _M_PER_S            | 34   | m/s                               | meter per second (velocity)                    |
| _M_PER_S2           | 35   | m/s2                              | meter per square second (acceleration)         |
| _M3_PER_S           | 36   | m3/s                              | cubic meter per second (flow rate)             |
| _L_PER_S            | 37   | l/s                               | liter per second (flow rate)                   |
| _W_PER_M2           | 38   | W/m2                              | watt per square meter (irradiance)             |
| _CD_PER_M2          | 39   | cd/m2                             | candela per square meter (luminance)           |
| _BIT                | 40   | bit                               | bit (information content)                      |
| _BIT_PER_S          | 41   | bit/s                             | bit per second (data rate)                     |
| _LATITUDE           | 42   | lat                               | degrees latitude[1]                            |
| _LONGITUDE          | 43   | lon                               | degrees longitude[1]                           |
| _PH                 | 44   | pH                                | pH value (acidity; logarithmic quantity)       |
| _DB                 | 45   | dB                                | decibel (logarithmic quantity)                 |
| _DBW                | 46   | dBW                               | decibel relative to 1 W (power level)          |
| _BSPL               | 47   | Bspl                              | bel (sound pressure level; log quantity)       |
| _COUNT              | 48   | count                             | 1 (counter value)                              |
| _PER                | 49   | /                                 | 1 (ratio e.g., value of a switch; )            |
| _PERCENT            | 50   | %                                 | 1 (ratio e.g., value of a switch; )            |
| _PERCENTRH          | 51   | %RH                               | Percentage (Relative Humidity)                 |
| _PERCENTEL          | 52   | %EL                               | Percentage (remaining battery energy level)    |
| _ENERGYLEVEL        | 53   | EL                                | seconds (remaining battery energy level)       |
| _1_PER_S            | 54   | 1/s                               | 1 per second (event rate)                      |
| _1_PER_MIN          | 55   | 1/min                             | 1 per minute (event rate, "rpm")               |
| _BEAT_PER_MIN       | 56   | beat/min                          | 1 per minute (heart rate in beats per minute)  |
| _BEATS              | 57   | beats                             | 1 (Cumulative number of heart beats)           |
| _SIEMPERMETER       | 58   | S/m                               | Siemens per meter (conductivity)               |
| _BYTE               | 59   | B                                 | Byte (information content)                     |
| _VOLTAMPERE         | 60   | VA                                | volt-ampere (Apparent Power)                   |
| _VOLTAMPERESEC      | 61   | VAs                               | volt-ampere second (Apparent Energy)           |
| _VAREACTIVE         | 62   | var                               | volt-ampere reactive (Reactive Power)          |
| _VAREACTIVESEC      | 63   | vars                              | volt-ampere-reactive second (Reactive Energy)  |
| _JOULE_PER_M        | 64   | J/m                               | joule per meter (Energy per distance)          |
| _KG_PER_M3          | 65   | kg/m3                             | kg/m3 (mass density, mass concentration)       |
| _DEGREE             | 66   | deg                               | degree (angle)                                 |
| _NTU                | 67   | NTU                               | Nephelometric Turbidity Unit                   |
| ----- rfc8798 ----- |      | Secondary Unit   (SenML Unit)     | Scale and Offset                               |
| _MS                 | 68   | s     millisecond                 | scale = 1/1000    1ms = 1s x [1/1000]          |
| _MIN                | 69   | s     minute                      | scale = 60                                     |
| _H                  | 70   | s     hour                        | scale = 3600                                   |
| _MHZ                | 71   | Hz    megahertz                   | scale = 1000000                                |
| _KW                 | 72   | W     kilowatt                    | scale = 1000                                   |
| _KVA                | 73   | VA    kilovolt-ampere             | scale = 1000                                   |
| _KVAR               | 74   | var   kilovar                     | scale = 1000                                   |
| _AH                 | 75   | C     ampere-hour                 | scale = 3600                                   |
| _WH                 | 76   | J     watt-hour                   | scale = 3600                                   |
| _KWH                | 77   | J     kilowatt-hour               | scale = 3600000                                |
| _VARH               | 78   | vars  var-hour                    | scale = 3600                                   |
| _KVARH              | 79   | vars  kilovar-hour                | scale = 3600000                                |
| _KVAH               | 80   | VAs   kilovolt-ampere-hour        | scale = 3600000                                |
| _WH_PER_KM          | 81   | J/m   watt-hour per kilometer     | scale = 3.6                                    |
| _KIB                | 82   | B     kibibyte                    | scale = 1024                                   |
| _GB                 | 83   | B     gigabyte                    | scale = 1e9                                    |
| _MBIT_PER_S         | 84   | bit/s megabit per second          | scale = 1000000                                |
| _B_PER_S            | 85   | bit/s byteper second              | scale = 8                                      |
| _MB_PER_S           | 86   | bit/s megabyte per second         | scale = 8000000                                |
| _MV                 | 87   | V     millivolt                   | scale = 1/1000                                 |
| _MA                 | 88   | A     milliampere                 | scale = 1/1000                                 |
| _DBM                | 89   | dBW   decibel rel. to 1 milliwatt | scale = 1       Offset = -30   0 dBm = -30 dBW |
| _UG_PER_M3          | 90   | kg/m3 microgram per cubic meter   | scale = 1e-9                                   |
| _MM_PER_H           | 91   | m/s   millimeter per hour         | scale = 1/3600000                              |
| _M_PER_H            | 92   | m/s   meterper hour               | scale = 1/3600                                 |
| _PPM                | 93   | /     partsper million            | scale = 1e-6                                   |
| _PER_100            | 94   | /     percent                     | scale = 1/100                                  |
| _PER_1000           | 95   | /     permille                    | scale = 1/1000                                 |
| _HPA                | 96   | Pa    hectopascal                 | scale = 100                                    |
| _MM                 | 97   | m     millimeter                  | scale = 1/1000                                 |
| _CM                 | 98   | m     centimeter                  | scale = 1/100                                  |
| _KM                 | 99   | m     kilometer                   | scale = 1000                                   |
| _KM_PER_H           | 100  | m/s   kilometer per hour          | scale = 1/3.6                                  |
| _GRAVITY            | 101  | m/s2  earth gravity               | scale = 9.81         1g = m/s2 x 9.81          |
| _DPS                | 102  | 1/s   degrees per second          | scale = 360        1dps = 1/s x 1/360          |
| _GAUSS              | 103  | Tesla Gauss                       | scale = 10-4         1G = Tesla x 1/10000      |
| _VRMS               | 104  | Volt  Volt rms                    | scale = 0.707     1Vrms = 1Volt (peak) x 0.707 |
| _MVPGAUSS           | 105  | millivolt Hall effect, mV/Gauss   | scale = 1    1mV/Gauss                         |
| _DBSPL              | 106  | Bspl versus dB SPL(A)             | scale = 1/10                                   |

## Stream format "domains"

| Domain name       | Code | Comments                                                     |
| ----------------- | ---- | ------------------------------------------------------------ |
| GENERAL           | 0    | (a)synchronous sensor + rescaling, electrical, chemical, color, .. remote data, compressed streams, JSON, SensorThings |
| AUDIO_IN          | 1    | microphone, line-in, I2S, PDM RX                             |
| AUDIO_OUT         | 2    | line-out, earphone / speaker, PDM TX, I2S,                   |
| GPIO              | 3    | generic digital IO, programmable timer ticks, control of relay |
| MOTION            | 4    | accelerometer, combined or not with pressure and gyroscope   |
| 2D_IN             | 5    | camera sensor                                                |
| 2D_OUT            | 6    | display, led matrix,                                         |
| ANALOG_IN         | 7    | analog sensor with aging/sensitivity/THR control, example : light, pressure, proximity, humidity, color, voltage |
| ANALOG_OUT        | 8    | D/A, position piezzo, PWM converter                          |
| USER_INTERFACE_IO | 9    | button, slider, rotary button, LED, digits, display,         |
| PLATFORM_6        | 10   | platform-specific #6                                         |
| PLATFORM_5        | 11   | platform-specific #5                                         |
| PLATFORM_4        | 12   | platform-specific #4                                         |
| PLATFORM_3        | 13   | platform-specific #3                                         |
| PLATFORM_2        | 14   | platform-specific #2                                         |
| PLATFORM_1        | 15   | platform-specific #1                                         |



## Architectures codes of platform manifest

Architecture codes (https://sourceware.org/binutils/docs/as/ARM-Options.html)  armv1, armv2, armv2a, armv2s, armv3, armv3m, armv4, armv4xm, armv4t, armv4txm, armv5, armv5t, armv5txm, armv5te, armv5texp, armv6, armv6j, armv6k, armv6z, armv6kz, armv6-m, armv6s-m, armv7, armv7-a, armv7ve, armv7-r, armv7-m, armv7e-m, armv8-a, armv8.1-a, armv8.2-a, armv8.3-a, armv8-r, armv8.4-a, armv8.5-a, armv8-m.base, armv8-m.main, armv8.1-m.main, armv8.6-a, armv8.7-a, armv8.8-a, armv8.9-a, armv9-a, armv9.1-a, armv9.2-a, armv9.3-a, armv9.4-a, armv9.5-a

