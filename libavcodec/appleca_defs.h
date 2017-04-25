/*
 * Apple Core Audio API Definitions
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef AVCODEC_APPLECA_DEFS_H
#define AVCODEC_APPLECA_DEFS_H

#include <stddef.h>
#include <stdint.h>
#include <float.h>
#include "libavutil/avconfig.h"

//TODO - portable way?
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmultichar"

#if AV_HAVE_BIGENDIAN
#define FF_APPLECA_BIGENDIAN 1
#endif

#ifdef __cplusplus
extern "C" {
#endif

//base types
typedef int8_t SInt8;
typedef uint8_t UInt8;
typedef int16_t SInt16;
typedef uint16_t UInt16;
typedef int32_t SInt32;
typedef uint32_t UInt32;
typedef int64_t SInt64;
typedef uint64_t UInt64;
typedef float Float32;
typedef double Float64;
typedef UInt8 Boolean;
typedef UInt32 FourCharCode;
typedef FourCharCode OSType;
typedef SInt32 OSStatus;

//core foundation
typedef UInt32 CFStringEncoding;
typedef const struct __CFString* CFStringRef;
typedef CFStringRef CFMutableStringRef;
//typedef CFIndex CFStringEncodings;
//typedef CFOptionFlags CFStringCompareFlags;
typedef struct __CFBundle* CFPlugInRef;
typedef const struct __CFAllocator* CFAllocatorRef;
typedef const struct __CFUUID* CFUUIDRef;

//core foundation callbacks
typedef void (*CFPlugInDynamicRegisterFunction) (CFPlugInRef plugIn);
typedef void* (*CFPlugInFactoryFunction) (CFAllocatorRef allocator, CFUUIDRef typeUUID);
typedef void (*CFPlugInUnloadFunction) (CFPlugInRef plugIn);

//core audio
typedef UInt32 AudioConverterPropertyID;
typedef UInt32 AudioChannelLabel;
typedef UInt32 AudioChannelLayoutTag;
typedef SInt16 AudioSampleType;
typedef SInt32 AudioUnitSampleType;
typedef struct OpaqueAudioConverter* AudioConverterRef;
typedef struct OpaqueAudioComponent* AudioComponent;
typedef struct OpaqueAudioComponentInstance* AudioComponentInstance;
typedef struct ComponentInstanceRecord { intptr_t data[1]; } ComponentInstanceRecord;
typedef ComponentInstanceRecord* ComponentInstance;
typedef ComponentInstance AudioCodec;
typedef UInt32 AudioCodecPropertyID;
typedef UInt32 AudioFormatPropertyID;

enum CoreConsts {

    //cherry picked
    kAudioStreamAnyRate = 0,
    kVariableLengthArray = 1,

    //return codes
    kAudio_UnimplementedError = -4,
    kAudio_FileNotFoundError = -43,
    kAudio_ParamError = -50,
    kAudio_MemFullError = -108,
    kAudioConverterErr_FormatNotSupported = 'fmt?',
    kAudioConverterErr_OperationNotSupported = 0x6F703F3F,
    kAudioConverterErr_PropertyNotSupported = 'prop',
    kAudioConverterErr_InvalidInputSize = 'insz',
    kAudioConverterErr_InvalidOutputSize = 'otsz',
    kAudioConverterErr_UnspecifiedError = 'what',
    kAudioConverterErr_BadPropertySizeError = '!siz',
    kAudioConverterErr_RequiresPacketDescriptionsError = '!pkd',
    kAudioConverterErr_InputSampleRateOutOfRange = '!isr',
    kAudioConverterErr_OutputSampleRateOutOfRange = '!osr',
    kAudioCodecNoError = 0,
    kAudioCodecUnspecifiedError = 'what',
    kAudioCodecUnknownPropertyError = 'who?',
    kAudioCodecBadPropertySizeError = '!siz',
    kAudioCodecIllegalOperationError = 'nope',
    kAudioCodecUnsupportedFormatError = '!dat',
    kAudioCodecStateError = '!stt',
    kAudioCodecNotEnoughBufferSpaceError = '!buf',
    kAudioFormatUnspecifiedError = 'what',
    kAudioFormatUnsupportedPropertyError = 'prop',
    kAudioFormatBadPropertySizeError = '!siz',
    kAudioFormatBadSpecifierSizeError = '!spc',
    kAudioFormatUnsupportedDataFormatError = 'fmt?',
    kAudioFormatUnknownFormatError = '!fmt',

    //sample type constants
    kAudioUnitSampleFractionBits = 24,

    //Audio Data Format Identifiers
    kAudioFormatLinearPCM = 'lpcm',
    kAudioFormatAC3 = 'ac-3',
    kAudioFormat60958AC3 = 'cac3',
    kAudioFormatAppleIMA4 = 'ima4',
    kAudioFormatMPEG4AAC = 'aac ',
    kAudioFormatMPEG4CELP = 'celp',
    kAudioFormatMPEG4HVXC = 'hvxc',
    kAudioFormatMPEG4TwinVQ = 'twvq',
    kAudioFormatMACE3 = 'MAC3',
    kAudioFormatMACE6 = 'MAC6',
    kAudioFormatULaw = 'ulaw',
    kAudioFormatALaw = 'alaw',
    kAudioFormatQDesign = 'QDMC',
    kAudioFormatQDesign2 = 'QDM2',
    kAudioFormatQUALCOMM = 'Qclp',
    kAudioFormatMPEGLayer1 = '.mp1',
    kAudioFormatMPEGLayer2 = '.mp2',
    kAudioFormatMPEGLayer3 = '.mp3',
    kAudioFormatTimeCode = 'time',
    kAudioFormatMIDIStream = 'midi',
    kAudioFormatParameterValueStream = 'apvs',
    kAudioFormatAppleLossless = 'alac',
    kAudioFormatMPEG4AAC_HE = 'aach',
    kAudioFormatMPEG4AAC_LD = 'aacl',
    kAudioFormatMPEG4AAC_ELD = 'aace',
    kAudioFormatMPEG4AAC_ELD_SBR = 'aacf',
    kAudioFormatMPEG4AAC_HE_V2 = 'aacp',
    kAudioFormatMPEG4AAC_Spatial = 'aacs',
    kAudioFormatAMR = 'samr',
    kAudioFormatAudible = 'AUDB',
    kAudioFormatiLBC = 'ilbc',
    kAudioFormatDVIIntelIMA = 0x6D730011,
    kAudioFormatMicrosoftGSM = 0x6D730031,
    kAudioFormatAES3 = 'aes3',

    //AudioStreamBasicDescription Flags
    kAudioFormatFlagIsFloat = (1 << 0),
    kAudioFormatFlagIsBigEndian = (1 << 1),
    kAudioFormatFlagIsSignedInteger = (1 << 2),
    kAudioFormatFlagIsPacked = (1 << 3),
    kAudioFormatFlagIsAlignedHigh = (1 << 4),
    kAudioFormatFlagIsNonInterleaved = (1 << 5),
    kAudioFormatFlagIsNonMixable = (1 << 6),
    kAudioFormatFlagsAreAllClear = (1 << 31),
    kLinearPCMFormatFlagIsFloat = kAudioFormatFlagIsFloat,
    kLinearPCMFormatFlagIsBigEndian = kAudioFormatFlagIsBigEndian,
    kLinearPCMFormatFlagIsSignedInteger = kAudioFormatFlagIsSignedInteger,
    kLinearPCMFormatFlagIsPacked = kAudioFormatFlagIsPacked,
    kLinearPCMFormatFlagIsAlignedHigh = kAudioFormatFlagIsAlignedHigh,
    kLinearPCMFormatFlagIsNonInterleaved = kAudioFormatFlagIsNonInterleaved,
    kLinearPCMFormatFlagIsNonMixable = kAudioFormatFlagIsNonMixable,
    kLinearPCMFormatFlagsSampleFractionShift = 7,
    kLinearPCMFormatFlagsSampleFractionMask = (0x3F << kLinearPCMFormatFlagsSampleFractionShift),
    kLinearPCMFormatFlagsAreAllClear = kAudioFormatFlagsAreAllClear,
    kAppleLosslessFormatFlag_16BitSourceData = 1,
    kAppleLosslessFormatFlag_20BitSourceData = 2,
    kAppleLosslessFormatFlag_24BitSourceData = 3,
    kAppleLosslessFormatFlag_32BitSourceData = 4,

#ifdef FF_APPLECA_BIGENDIAN
    kAudioFormatFlagsNativeEndian = kAudioFormatFlagIsBigEndian,
#else
    kAudioFormatFlagsNativeEndian = 0,
#endif

    //some combos
    kAudioFormatFlagsNativeIntPacked = kAudioFormatFlagIsSignedInteger | kAudioFormatFlagsNativeEndian | kAudioFormatFlagIsPacked,
    kAudioFormatFlagsNativeFloatPacked = kAudioFormatFlagIsFloat | kAudioFormatFlagsNativeEndian | kAudioFormatFlagIsPacked,

    //MPEG-4 Audio Object Type Constants
    kMPEG4Object_AAC_Main = 1,
    kMPEG4Object_AAC_LC = 2,
    kMPEG4Object_AAC_SSR = 3,
    kMPEG4Object_AAC_LTP = 4,
    kMPEG4Object_AAC_SBR = 5,
    kMPEG4Object_AAC_Scalable = 6,
    kMPEG4Object_TwinVQ = 7,
    kMPEG4Object_CELP = 8,
    kMPEG4Object_HVXC = 9,

    //SMPTE Timecode Type Constants
    kSMPTETimeType24 = 0,
    kSMPTETimeType25 = 1,
    kSMPTETimeType30Drop = 2,
    kSMPTETimeType30 = 3,
    kSMPTETimeType2997 = 4,
    kSMPTETimeType2997Drop = 5,
    kSMPTETimeType60 = 6,
    kSMPTETimeType5994 = 7,
    kSMPTETimeType60Drop = 8,
    kSMPTETimeType5994Drop = 9,
    kSMPTETimeType50 = 10,
    kSMPTETimeType2398 = 11,

    //SMPTE State Flags
    kSMPTETimeValid = (1 << 0),
    kSMPTETimeRunning = (1 << 1),

    //Audio Time Stamp Flags
    kAudioTimeStampSampleTimeValid = (1 << 0),
    kAudioTimeStampHostTimeValid = (1 << 1),
    kAudioTimeStampRateScalarValid = (1 << 2),
    kAudioTimeStampWordClockTimeValid = (1 << 3),
    kAudioTimeStampSMPTETimeValid = (1 << 4),
    kAudioTimeStampSampleHostTimeValid = (kAudioTimeStampSampleTimeValid | kAudioTimeStampHostTimeValid),

    //Audio Channel Label Constants
    kAudioChannelLabel_Unknown = 0xFFFFFFFF,
    kAudioChannelLabel_Unused = 0,
    kAudioChannelLabel_UseCoordinates = 100,
    kAudioChannelLabel_Left = 1,
    kAudioChannelLabel_Right = 2,
    kAudioChannelLabel_Center = 3,
    kAudioChannelLabel_LFEScreen = 4,
    kAudioChannelLabel_LeftSurround = 5,
    kAudioChannelLabel_RightSurround = 6,
    kAudioChannelLabel_LeftCenter = 7,
    kAudioChannelLabel_RightCenter = 8,
    kAudioChannelLabel_CenterSurround = 9,
    kAudioChannelLabel_LeftSurroundDirect = 10,
    kAudioChannelLabel_RightSurroundDirect = 11,
    kAudioChannelLabel_TopCenterSurround = 12,
    kAudioChannelLabel_VerticalHeightLeft = 13,
    kAudioChannelLabel_VerticalHeightCenter = 14,
    kAudioChannelLabel_VerticalHeightRight = 15,
    kAudioChannelLabel_TopBackLeft = 16,
    kAudioChannelLabel_TopBackCenter = 17,
    kAudioChannelLabel_TopBackRight = 18,
    kAudioChannelLabel_RearSurroundLeft = 33,
    kAudioChannelLabel_RearSurroundRight = 34,
    kAudioChannelLabel_LeftWide = 35,
    kAudioChannelLabel_RightWide = 36,
    kAudioChannelLabel_LFE2 = 37,
    kAudioChannelLabel_LeftTotal = 38,
    kAudioChannelLabel_RightTotal = 39,
    kAudioChannelLabel_HearingImpaired = 40,
    kAudioChannelLabel_Narration = 41,
    kAudioChannelLabel_Mono = 42,
    kAudioChannelLabel_DialogCentricMix = 43,
    kAudioChannelLabel_CenterSurroundDirect = 44,
    kAudioChannelLabel_Haptic = 45,
    kAudioChannelLabel_Ambisonic_W = 200,
    kAudioChannelLabel_Ambisonic_X = 201,
    kAudioChannelLabel_Ambisonic_Y = 202,
    kAudioChannelLabel_Ambisonic_Z = 203,
    kAudioChannelLabel_MS_Mid = 204,
    kAudioChannelLabel_MS_Side = 205,
    kAudioChannelLabel_XY_X = 206,
    kAudioChannelLabel_XY_Y = 207,
    kAudioChannelLabel_HeadphonesLeft = 301,
    kAudioChannelLabel_HeadphonesRight = 302,
    kAudioChannelLabel_ClickTrack = 304,
    kAudioChannelLabel_ForeignLanguage = 305,
    kAudioChannelLabel_Discrete = 400,
    kAudioChannelLabel_Discrete_0 = (1 << 16) | 0,
    kAudioChannelLabel_Discrete_1 = (1 << 16) | 1,
    kAudioChannelLabel_Discrete_2 = (1 << 16) | 2,
    kAudioChannelLabel_Discrete_3 = (1 << 16) | 3,
    kAudioChannelLabel_Discrete_4 = (1 << 16) | 4,
    kAudioChannelLabel_Discrete_5 = (1 << 16) | 5,
    kAudioChannelLabel_Discrete_6 = (1 << 16) | 6,
    kAudioChannelLabel_Discrete_7 = (1 << 16) | 7,
    kAudioChannelLabel_Discrete_8 = (1 << 16) | 8,
    kAudioChannelLabel_Discrete_9 = (1 << 16) | 9,
    kAudioChannelLabel_Discrete_10 = (1 << 16) | 10,
    kAudioChannelLabel_Discrete_11 = (1 << 16) | 11,
    kAudioChannelLabel_Discrete_12 = (1 << 16) | 12,
    kAudioChannelLabel_Discrete_13 = (1 << 16) | 13,
    kAudioChannelLabel_Discrete_14 = (1 << 16) | 14,
    kAudioChannelLabel_Discrete_15 = (1 << 16) | 15,
    kAudioChannelLabel_Discrete_65535 = (1 << 16) | 65535,

    //Channel Bitmap Constants
    kAudioChannelBit_Left = (1 << 0),
    kAudioChannelBit_Right = (1 << 1),
    kAudioChannelBit_Center = (1 << 2),
    kAudioChannelBit_LFEScreen = (1 << 3),
    kAudioChannelBit_LeftSurround = (1 << 4),
    kAudioChannelBit_RightSurround = (1 << 5),
    kAudioChannelBit_LeftCenter = (1 << 6),
    kAudioChannelBit_RightCenter = (1 << 7),
    kAudioChannelBit_CenterSurround = (1 << 8),
    kAudioChannelBit_LeftSurroundDirect = (1 << 9),
    kAudioChannelBit_RightSurroundDirect = (1 << 10),
    kAudioChannelBit_TopCenterSurround = (1 << 11),
    kAudioChannelBit_VerticalHeightLeft = (1 << 12),
    kAudioChannelBit_VerticalHeightCenter = (1 << 13),
    kAudioChannelBit_VerticalHeightRight = (1 << 14),
    kAudioChannelBit_TopBackLeft = (1 << 15),
    kAudioChannelBit_TopBackCenter = (1 << 16),
    kAudioChannelBit_TopBackRight = (1 << 17),

    //Channel Coordinate Flags
    kAudioChannelFlags_AllOff = 0,
    kAudioChannelFlags_RectangularCoordinates = (1 << 0),
    kAudioChannelFlags_SphericalCoordinates = (1 << 1),
    kAudioChannelFlags_Meters = (1 << 2),

    //Channel Coordinate Index Constants
    kAudioChannelCoordinates_LeftRight = 0,
    kAudioChannelCoordinates_BackFront = 1,
    kAudioChannelCoordinates_DownUp = 2,
    kAudioChannelCoordinates_Azimuth = 0,
    kAudioChannelCoordinates_Elevation = 1,
    kAudioChannelCoordinates_Distance = 2,

    //Audio Channel Layout Tags
    kAudioChannelLayoutTag_UseChannelDescriptions = (0 << 16) | 0,
    kAudioChannelLayoutTag_UseChannelBitmap = (1 << 16) | 0,
    kAudioChannelLayoutTag_Mono = (100 << 16) | 1,
    kAudioChannelLayoutTag_Stereo = (101 << 16) | 2,
    kAudioChannelLayoutTag_StereoHeadphones = (102 << 16) | 2,
    kAudioChannelLayoutTag_MatrixStereo = (103 << 16) | 2,
    kAudioChannelLayoutTag_MidSide = (104 << 16) | 2,
    kAudioChannelLayoutTag_XY = (105 << 16) | 2,
    kAudioChannelLayoutTag_Binaural = (106 << 16) | 2,
    kAudioChannelLayoutTag_Ambisonic_B_Format = (107 << 16) | 4,
    kAudioChannelLayoutTag_Quadraphonic = (108 << 16) | 4,
    kAudioChannelLayoutTag_Pentagonal = (109 << 16) | 5,
    kAudioChannelLayoutTag_Hexagonal = (110 << 16) | 6,
    kAudioChannelLayoutTag_Octagonal = (111 << 16) | 8,
    kAudioChannelLayoutTag_Cube = (112 << 16) | 8,
    kAudioChannelLayoutTag_MPEG_1_0 = kAudioChannelLayoutTag_Mono,
    kAudioChannelLayoutTag_MPEG_2_0 = kAudioChannelLayoutTag_Stereo,
    kAudioChannelLayoutTag_MPEG_3_0_A = (113 << 16) | 3,
    kAudioChannelLayoutTag_MPEG_3_0_B = (114 << 16) | 3,
    kAudioChannelLayoutTag_MPEG_4_0_A = (115 << 16) | 4,
    kAudioChannelLayoutTag_MPEG_4_0_B = (116 << 16) | 4,
    kAudioChannelLayoutTag_MPEG_5_0_A = (117 << 16) | 5,
    kAudioChannelLayoutTag_MPEG_5_0_B = (118 << 16) | 5,
    kAudioChannelLayoutTag_MPEG_5_0_C = (119 << 16) | 5,
    kAudioChannelLayoutTag_MPEG_5_0_D = (120 << 16) | 5,
    kAudioChannelLayoutTag_MPEG_5_1_A = (121 << 16) | 6,
    kAudioChannelLayoutTag_MPEG_5_1_B = (122 << 16) | 6,
    kAudioChannelLayoutTag_MPEG_5_1_C = (123 << 16) | 6,
    kAudioChannelLayoutTag_MPEG_5_1_D = (124 << 16) | 6,
    kAudioChannelLayoutTag_MPEG_6_1_A = (125 << 16) | 7,
    kAudioChannelLayoutTag_MPEG_7_1_A = (126 << 16) | 8,
    kAudioChannelLayoutTag_MPEG_7_1_B = (127 << 16) | 8,
    kAudioChannelLayoutTag_MPEG_7_1_C = (128 << 16) | 8,
    kAudioChannelLayoutTag_Emagic_Default_7_1 = (129 << 16) | 8,
    kAudioChannelLayoutTag_SMPTE_DTV = (130 << 16) | 8,
    kAudioChannelLayoutTag_ITU_1_0 = kAudioChannelLayoutTag_Mono,
    kAudioChannelLayoutTag_ITU_2_0 = kAudioChannelLayoutTag_Stereo,
    kAudioChannelLayoutTag_ITU_2_1 = (131 << 16) | 3,
    kAudioChannelLayoutTag_ITU_2_2 = (132 << 16) | 4,
    kAudioChannelLayoutTag_ITU_3_0 = kAudioChannelLayoutTag_MPEG_3_0_A,
    kAudioChannelLayoutTag_ITU_3_1 = kAudioChannelLayoutTag_MPEG_4_0_A,
    kAudioChannelLayoutTag_ITU_3_2 = kAudioChannelLayoutTag_MPEG_5_0_A,
    kAudioChannelLayoutTag_ITU_3_2_1 = kAudioChannelLayoutTag_MPEG_5_1_A,
    kAudioChannelLayoutTag_ITU_3_4_1 = kAudioChannelLayoutTag_MPEG_7_1_C,
    kAudioChannelLayoutTag_DVD_0 = kAudioChannelLayoutTag_Mono,
    kAudioChannelLayoutTag_DVD_1 = kAudioChannelLayoutTag_Stereo,
    kAudioChannelLayoutTag_DVD_2 = kAudioChannelLayoutTag_ITU_2_1,
    kAudioChannelLayoutTag_DVD_3 = kAudioChannelLayoutTag_ITU_2_2,
    kAudioChannelLayoutTag_DVD_4 = (133 << 16) | 3,     // L R LFE
    kAudioChannelLayoutTag_DVD_5 = (134 << 16) | 4,
    kAudioChannelLayoutTag_DVD_6 = (135 << 16) | 5,
    kAudioChannelLayoutTag_DVD_7 = kAudioChannelLayoutTag_MPEG_3_0_A,
    kAudioChannelLayoutTag_DVD_8 = kAudioChannelLayoutTag_MPEG_4_0_A,
    kAudioChannelLayoutTag_DVD_9 = kAudioChannelLayoutTag_MPEG_5_0_A,
    kAudioChannelLayoutTag_DVD_10 = (136 << 16) | 4,
    kAudioChannelLayoutTag_DVD_11 = (137 << 16) | 5,
    kAudioChannelLayoutTag_DVD_12 = kAudioChannelLayoutTag_MPEG_5_1_A,
    kAudioChannelLayoutTag_DVD_13 = kAudioChannelLayoutTag_DVD_8,
    kAudioChannelLayoutTag_DVD_14 = kAudioChannelLayoutTag_DVD_9,
    kAudioChannelLayoutTag_DVD_15 = kAudioChannelLayoutTag_DVD_10,
    kAudioChannelLayoutTag_DVD_16 = kAudioChannelLayoutTag_DVD_11,
    kAudioChannelLayoutTag_DVD_17 = kAudioChannelLayoutTag_DVD_12,
    kAudioChannelLayoutTag_DVD_18 = (138 << 16) | 5,
    kAudioChannelLayoutTag_DVD_19 = kAudioChannelLayoutTag_MPEG_5_0_B,
    kAudioChannelLayoutTag_DVD_20 = kAudioChannelLayoutTag_MPEG_5_1_B,
    kAudioChannelLayoutTag_AudioUnit_4 = kAudioChannelLayoutTag_Quadraphonic,
    kAudioChannelLayoutTag_AudioUnit_5 = kAudioChannelLayoutTag_Pentagonal,
    kAudioChannelLayoutTag_AudioUnit_6 = kAudioChannelLayoutTag_Hexagonal,
    kAudioChannelLayoutTag_AudioUnit_8 = kAudioChannelLayoutTag_Octagonal,
    kAudioChannelLayoutTag_AudioUnit_5_0 = kAudioChannelLayoutTag_MPEG_5_0_B,
    kAudioChannelLayoutTag_AudioUnit_6_0 = (139 << 16) | 6,
    kAudioChannelLayoutTag_AudioUnit_7_0 = (140 << 16) | 7,
    kAudioChannelLayoutTag_AudioUnit_7_0_Front = (148 << 16) | 7,
    kAudioChannelLayoutTag_AudioUnit_5_1 = kAudioChannelLayoutTag_MPEG_5_1_A,
    kAudioChannelLayoutTag_AudioUnit_6_1 = kAudioChannelLayoutTag_MPEG_6_1_A,
    kAudioChannelLayoutTag_AudioUnit_7_1 = kAudioChannelLayoutTag_MPEG_7_1_C,
    kAudioChannelLayoutTag_AudioUnit_7_1_Front = kAudioChannelLayoutTag_MPEG_7_1_A,
    kAudioChannelLayoutTag_AAC_3_0 = kAudioChannelLayoutTag_MPEG_3_0_B,                 // C L R
    kAudioChannelLayoutTag_AAC_Quadraphonic = kAudioChannelLayoutTag_Quadraphonic,      // L R Ls Rs
    kAudioChannelLayoutTag_AAC_4_0 = kAudioChannelLayoutTag_MPEG_4_0_B,                 // C L R Cs
    kAudioChannelLayoutTag_AAC_5_0 = kAudioChannelLayoutTag_MPEG_5_0_D,                 // C L R Ls Rs
    kAudioChannelLayoutTag_AAC_5_1 = kAudioChannelLayoutTag_MPEG_5_1_D,                 // C L R Ls Rs Lfe
    kAudioChannelLayoutTag_AAC_6_0 = (141 << 16) | 6,                                   // C L R Ls Rs Cs
    kAudioChannelLayoutTag_AAC_6_1 = (142 << 16) | 7,                                   // C L R Ls Rs Cs Lfe
    kAudioChannelLayoutTag_AAC_7_0 = (143 << 16) | 7,                                   // C L R Ls Rs Rls Rrs
    kAudioChannelLayoutTag_AAC_7_1 = kAudioChannelLayoutTag_MPEG_7_1_B,                 // C Lc Rc L R Ls Rs Lfe
    kAudioChannelLayoutTag_AAC_Octagonal = (144 << 16) | 8,                             // C L R Ls Rs Rls Rrs Cs
    kAudioChannelLayoutTag_TMH_10_2_std = (145 << 16) | 16,
    kAudioChannelLayoutTag_TMH_10_2_full = (146 << 16) | 21,
    kAudioChannelLayoutTag_AC3_1_0_1 = (149 << 16) | 2,
    kAudioChannelLayoutTag_AC3_3_0 = (150 << 16) | 3,
    kAudioChannelLayoutTag_AC3_3_1 = (151 << 16) | 4,
    kAudioChannelLayoutTag_AC3_3_0_1 = (152 << 16) | 4,
    kAudioChannelLayoutTag_AC3_2_1_1 = (153 << 16) | 4,
    kAudioChannelLayoutTag_AC3_3_1_1 = (154 << 16) | 5,
    kAudioChannelLayoutTag_EAC_6_0_A = (155 << 16) | 6,
    kAudioChannelLayoutTag_EAC_7_0_A = (156 << 16) | 7,
    kAudioChannelLayoutTag_EAC3_6_1_A = (157 << 16) | 7,
    kAudioChannelLayoutTag_EAC3_6_1_B = (158 << 16) | 7,
    kAudioChannelLayoutTag_EAC3_6_1_C = (159 << 16) | 7,
    kAudioChannelLayoutTag_EAC3_7_1_A = (160 << 16) | 8,
    kAudioChannelLayoutTag_EAC3_7_1_B = (161 << 16) | 8,
    kAudioChannelLayoutTag_EAC3_7_1_C = (162 << 16) | 8,
    kAudioChannelLayoutTag_EAC3_7_1_D = (163 << 16) | 8,
    kAudioChannelLayoutTag_EAC3_7_1_E = (164 << 16) | 8,
    kAudioChannelLayoutTag_EAC3_7_1_F = (165 << 16) | 8,
    kAudioChannelLayoutTag_EAC3_7_1_G = (166 << 16) | 8,
    kAudioChannelLayoutTag_EAC3_7_1_H = (167 << 16) | 8,
    kAudioChannelLayoutTag_DTS_3_1 = (168 << 16) | 4,
    kAudioChannelLayoutTag_DTS_4_1 = (169 << 16) | 5,
    kAudioChannelLayoutTag_DTS_6_0_A = (170 << 16) | 6,
    kAudioChannelLayoutTag_DTS_6_0_B = (171 << 16) | 6,
    kAudioChannelLayoutTag_DTS_6_0_C = (172 << 16) | 6,
    kAudioChannelLayoutTag_DTS_6_1_A = (173 << 16) | 7,
    kAudioChannelLayoutTag_DTS_6_1_B = (174 << 16) | 7,
    kAudioChannelLayoutTag_DTS_6_1_C = (175 << 16) | 7,
    kAudioChannelLayoutTag_DTS_6_1_D = (182 << 16) | 7,
    kAudioChannelLayoutTag_DTS_7_0 = (176 << 16) | 7,
    kAudioChannelLayoutTag_DTS_7_1 = (177 << 16) | 8,
    kAudioChannelLayoutTag_DTS_8_0_A = (178 << 16) | 8,
    kAudioChannelLayoutTag_DTS_8_0_B = (179 << 16) | 8,
    kAudioChannelLayoutTag_DTS_8_1_A = (180 << 16) | 9,
    kAudioChannelLayoutTag_DTS_8_1_B = (181 << 16) | 9,
    kAudioChannelLayoutTag_DiscreteInOrder = (147 << 16) | 0,   // needs to be ORed with the actual number of channels
    kAudioChannelLayoutTag_Unknown = 0xFFFF0000,                // needs to be ORed with the actual number of channels

    //ALAC tags
    kALACChannelLayoutTag_Mono = kAudioChannelLayoutTag_Mono,               // C
    kALACChannelLayoutTag_Stereo = kAudioChannelLayoutTag_Stereo,           // L R
    kALACChannelLayoutTag_MPEG_3_0_B = kAudioChannelLayoutTag_MPEG_3_0_B,   // C L R
    kALACChannelLayoutTag_MPEG_4_0_B = kAudioChannelLayoutTag_MPEG_4_0_B,   // C L R Cs
    kALACChannelLayoutTag_MPEG_5_0_D = kAudioChannelLayoutTag_MPEG_5_0_D,   // C L R Ls Rs
    kALACChannelLayoutTag_MPEG_5_1_D = kAudioChannelLayoutTag_MPEG_5_1_D,   // C L R Ls Rs LFE
    kALACChannelLayoutTag_AAC_6_1 = kAudioChannelLayoutTag_AAC_6_1,         // C L R Ls Rs Cs LFE
    kALACChannelLayoutTag_MPEG_7_1_B = kAudioChannelLayoutTag_MPEG_7_1_B,   // C Lc Rc L R Ls Rs LFE

    //Audio Converter Properties
    kAudioConverterPropertyMinimumInputBufferSize = 'mibs',
    kAudioConverterPropertyMinimumOutputBufferSize = 'mobs',
    kAudioConverterPropertyMaximumInputBufferSize = 'xibs',
    kAudioConverterPropertyMaximumInputPacketSize = 'xips',
    kAudioConverterPropertyMaximumOutputPacketSize = 'xops',
    kAudioConverterPropertyCalculateInputBufferSize = 'cibs',
    kAudioConverterPropertyCalculateOutputBufferSize = 'cobs',
    kAudioConverterPropertyInputCodecParameters = 'icdp',
    kAudioConverterPropertyOutputCodecParameters = 'ocdp',
    kAudioConverterSampleRateConverterAlgorithm = 'srci',
    kAudioConverterSampleRateConverterComplexity = 'srca',
    kAudioConverterSampleRateConverterQuality = 'srcq',
    kAudioConverterSampleRateConverterInitialPhase = 'srcp',
    kAudioConverterCodecQuality = 'cdqu',
    kAudioConverterPrimeMethod = 'prmm',
    kAudioConverterPrimeInfo = 'prim',
    kAudioConverterChannelMap = 'chmp',
    kAudioConverterDecompressionMagicCookie = 'dmgc',
    kAudioConverterCompressionMagicCookie = 'cmgc',
    kAudioConverterEncodeBitRate = 'brat',
    kAudioConverterEncodeAdjustableSampleRate = 'ajsr',
    kAudioConverterInputChannelLayout = 'icl ',
    kAudioConverterOutputChannelLayout = 'ocl ',
    kAudioConverterApplicableEncodeBitRates = 'aebr',
    kAudioConverterAvailableEncodeBitRates = 'vebr',
    kAudioConverterApplicableEncodeSampleRates = 'aesr',
    kAudioConverterAvailableEncodeSampleRates = 'vesr',
    kAudioConverterAvailableEncodeChannelLayoutTags = 'aecl',
    kAudioConverterCurrentOutputStreamDescription = 'acod',
    kAudioConverterCurrentInputStreamDescription = 'acid',
    kAudioConverterPropertySettings = 'acps',
    kAudioConverterPropertyBitDepthHint = 'acbd',
    kAudioConverterPropertyFormatList = 'flst',

    //Converter Priming Constants
    kConverterPrimeMethod_Pre = 0,
    kConverterPrimeMethod_Normal = 1,
    kConverterPrimeMethod_None = 2,

    //Sample Rate Conversion Quality Identifiers
    kAudioConverterQuality_Max = 0x7F,
    kAudioConverterQuality_High = 0x60,
    kAudioConverterQuality_Medium = 0x40,
    kAudioConverterQuality_Low = 0x20,
    kAudioConverterQuality_Min = 0,

    //Sample Rate Conversion Complexity Identifiers
    kAudioConverterSampleRateConverterComplexity_Linear = 'line',
    kAudioConverterSampleRateConverterComplexity_Normal = 'norm',
    kAudioConverterSampleRateConverterComplexity_Mastering = 'bats',

    //Audio Codec Component Constants
    kAudioDecoderComponentType = 'adec',
    kAudioEncoderComponentType = 'aenc',    //also in 'Audio Codec Conversion Types'
    kAudioUnityCodecComponentType = 'acdc', //also in 'Audio Codec Conversion Types'

    //Global Codec Properties
    kAudioCodecPropertyNameCFString = 'lnam',
    kAudioCodecPropertyManufacturerCFString = 'lmak',
    kAudioCodecPropertyFormatCFString = 'lfor',
    kAudioCodecPropertySupportedInputFormats = 'ifm#',
    kAudioCodecPropertySupportedOutputFormats = 'ofm#',
    kAudioCodecPropertyAvailableInputSampleRates = 'aisr',
    kAudioCodecPropertyAvailableOutputSampleRates = 'aosr',
    kAudioCodecPropertyAvailableBitRateRange = 'abrt',
    kAudioCodecPropertyMinimumNumberInputPackets = 'mnip',
    kAudioCodecPropertyMinimumNumberOutputPackets = 'mnop',
    kAudioCodecPropertyAvailableNumberChannels = 'cmnc',
    kAudioCodecPropertyDoesSampleRateConversion = 'lmrc',
    kAudioCodecPropertyAvailableInputChannelLayoutTags = 'aicl',
    kAudioCodecPropertyAvailableOutputChannelLayoutTags = 'aocl',
    kAudioCodecPropertyInputFormatsForOutputFormat = 'if4o',
    kAudioCodecPropertyOutputFormatsForInputFormat = 'of4i',
    kAudioCodecPropertyFormatInfo = 'acfi',

    //Instance Codec Properties
    kAudioCodecPropertyInputBufferSize = 'tbuf',
    kAudioCodecPropertyPacketFrameSize = 'pakf',
    kAudioCodecPropertyHasVariablePacketByteSizes = 'vpk?',
    kAudioCodecPropertyMaximumPacketByteSize = 'pakb',
    kAudioCodecPropertyCurrentInputFormat = 'ifmt',
    kAudioCodecPropertyCurrentOutputFormat = 'ofmt',
    kAudioCodecPropertyMagicCookie = 'kuki',
    kAudioCodecPropertyUsedInputBufferSize = 'ubuf',
    kAudioCodecPropertyIsInitialized = 'init',
    kAudioCodecPropertyCurrentTargetBitRate = 'brat',
    kAudioCodecPropertyCurrentInputSampleRate = 'cisr',
    kAudioCodecPropertyCurrentOutputSampleRate = 'cosr',
    kAudioCodecPropertyQualitySetting = 'srcq',
    kAudioCodecPropertyApplicableBitRateRange = 'brta',
    kAudioCodecPropertyApplicableInputSampleRates = 'isra',
    kAudioCodecPropertyApplicableOutputSampleRates = 'osra',
    kAudioCodecPropertyPaddedZeros = 'pad0',
    kAudioCodecPropertyPrimeMethod = 'prmm',
    kAudioCodecPropertyPrimeInfo = 'prim',
    kAudioCodecPropertyCurrentInputChannelLayout = 'icl ',
    kAudioCodecPropertyCurrentOutputChannelLayout = 'ocl ',
    kAudioCodecPropertySettings = 'acs ',
    kAudioCodecPropertyFormatList = 'acfl',
    kAudioCodecPropertyBitRateControlMode = 'acbf',
    kAudioCodecPropertySoundQualityForVBR = 'vbrq',
    kAudioCodecPropertyMinimumDelayMode = 'mdel',

    //Audio Codec Quality Constants
    kAudioCodecQuality_Max = 0x7F,
    kAudioCodecQuality_High = 0x60,
    kAudioCodecQuality_Medium = 0x40,
    kAudioCodecQuality_Low = 0x20,
    kAudioCodecQuality_Min = 0,

    //Audio Codec Priming Method Constants
    kAudioCodecPrimeMethod_Pre = 0,
    kAudioCodecPrimeMethod_Normal = 1,
    kAudioCodecPrimeMethod_None = 2,

    //Bit Rate Control Mode Constants
    kAudioCodecBitRateControlMode_Constant = 0,
    kAudioCodecBitRateControlMode_LongTermAverage = 1,
    kAudioCodecBitRateControlMode_VariableConstrained = 2,
    kAudioCodecBitRateControlMode_Variable = 3,

    //Audio Settings Flags
    kAudioSettingsFlags_ExpertParameter = (1L << 0),
    kAudioSettingsFlags_InvisibleParameter = (1L << 1),
    kAudioSettingsFlags_MetaParameter = (1L << 2),
    kAudioSettingsFlags_UserInterfaceParameter = (1L << 3),

    //Output Status Constants
    kAudioCodecProduceOutputPacketFailure = 1,
    kAudioCodecProduceOutputPacketSuccess = 2,
    kAudioCodecProduceOutputPacketSuccessHasMore = 3,
    kAudioCodecProduceOutputPacketNeedsMoreInputData = 4,
    kAudioCodecProduceOutputPacketAtEOF = 5,

    //Audio Codec Routine Selectors
    kAudioCodecGetPropertyInfoSelect = 0x0001,
    kAudioCodecGetPropertySelect = 0x0002,
    kAudioCodecSetPropertySelect = 0x0003,
    kAudioCodecInitializeSelect = 0x0004,
    kAudioCodecUninitializeSelect = 0x0005,
    kAudioCodecAppendInputDataSelect = 0x0006,
    kAudioCodecProduceOutputDataSelect = 0x0007,
    kAudioCodecResetSelect = 0x0008,

    //Audio Balance Fade Types
    kAudioBalanceFadeType_MaxUnityGain = 0,
    kAudioBalanceFadeType_EqualPower = 1,

    //Audio Format Property Identifiers
    // AudioStreamBasicDescription structure properties
    kAudioFormatProperty_FormatInfo = 'fmti',
    kAudioFormatProperty_FormatName = 'fnam',
    kAudioFormatProperty_EncodeFormatIDs = 'acof',
    kAudioFormatProperty_DecodeFormatIDs = 'acif',
    kAudioFormatProperty_FormatList = 'flst',
    kAudioFormatProperty_ASBDFromESDS = 'essd',
    kAudioFormatProperty_ChannelLayoutFromESDS = 'escl',
    kAudioFormatProperty_OutputFormatList = 'ofls',
    kAudioFormatProperty_Encoders = 'aven',
    kAudioFormatProperty_Decoders = 'avde',
    kAudioFormatProperty_FormatIsVBR = 'fvbr',
    kAudioFormatProperty_FormatIsExternallyFramed = 'fexf',
    kAudioFormatProperty_AvailableEncodeBitRates = 'aebr',
    kAudioFormatProperty_AvailableEncodeSampleRates = 'aesr',
    kAudioFormatProperty_AvailableEncodeChannelLayoutTags = 'aecl',
    kAudioFormatProperty_AvailableEncodeNumberChannels = 'avnc',
    kAudioFormatProperty_ASBDFromMPEGPacket = 'admp',

    // AudioChannelLayout structure properties
    kAudioFormatProperty_BitmapForLayoutTag = 'bmtg',
    kAudioFormatProperty_MatrixMixMap = 'mmap',
    kAudioFormatProperty_ChannelMap = 'chmp',
    kAudioFormatProperty_NumberOfChannelsForLayout = 'nchm',
    kAudioFormatProperty_ValidateChannelLayout = 'vacl',
    kAudioFormatProperty_ChannelLayoutForTag = 'cmpl',
    kAudioFormatProperty_TagForChannelLayout = 'cmpt',
    kAudioFormatProperty_ChannelLayoutName = 'lonm',
    kAudioFormatProperty_ChannelLayoutSimpleName = 'lsnm',
    kAudioFormatProperty_ChannelLayoutForBitmap = 'cmpb',
    kAudioFormatProperty_ChannelName = 'cnam',
    kAudioFormatProperty_ChannelShortName = 'csnm',
    kAudioFormatProperty_TagsForNumberOfChannels = 'tagc',
    kAudioFormatProperty_PanningMatrix = 'panm',
    kAudioFormatProperty_BalanceFade = 'balf',

    // ID3 tag (MP3 metadata ) properties
    kAudioFormatProperty_ID3TagSize = 'id3s',
    kAudioFormatProperty_ID3TagToDictionary = 'id3d',

    //Hardware Codec Capabilities
    kAudioFormatProperty_HardwareCodecCapabilities = 'hwcc',

    //Audio Codec Manufacturer and Implementation Types
    kAppleSoftwareAudioCodecManufacturer = 'appl',
    kAppleHardwareAudioCodecManufacturer = 'aphw',

    //Audio Panning Modes
    kPanningMode_SoundField = 3,
    kPanningMode_VectorBasedPanning = 4,
};

//Constants for kAudioCodecPropertySettings
#define kAudioSettings_TopLevelKey        "name"
#define kAudioSettings_Version            "version"
#define kAudioSettings_Parameters         "parameters"
#define kAudioSettings_SettingKey         "key"
#define kAudioSettings_SettingName        "name"
#define kAudioSettings_ValueType          "value type"
#define kAudioSettings_AvailableValues    "available values"
#define kAudioSettings_LimitedValues      "limited values"
#define kAudioSettings_CurrentValue       "current value"
#define kAudioSettings_Summary            "summary"
#define kAudioSettings_Hint               "hint"
#define kAudioSettings_Unit               "unit"

typedef struct SMPTETime
{
    SInt16 mSubframes;
    SInt16 mSubframeDivisor;
    UInt32 mCounter;
    UInt32 mType;
    UInt32 mFlags;
    SInt16 mHours;
    SInt16 mMinutes;
    SInt16 mSeconds;
    SInt16 mFrames;
} SMPTETime;

typedef struct AudioBuffer
{
    UInt32 mNumberChannels;
    UInt32 mDataByteSize;
    void* mData;
} AudioBuffer;

typedef struct AudioBufferList
{
    UInt32 mNumberBuffers;
    AudioBuffer mBuffers[kVariableLengthArray];
} AudioBufferList;

typedef struct AudioChannelDescription
{
    AudioChannelLabel mChannelLabel;
    UInt32 mChannelFlags;
    Float32 mCoordinates[3];
} AudioChannelDescription;

typedef struct AudioChannelLayout
{
    AudioChannelLayoutTag mChannelLayoutTag;
    UInt32 mChannelBitmap;
    UInt32 mNumberChannelDescriptions;
    //AudioChannelDescription mChannelDescriptions[kVariableLengthArray];
    AudioChannelDescription mChannelDescriptions[8];
} AudioChannelLayout;

typedef struct AudioClassDescription
{
    OSType mType;
    OSType mSubType;
    OSType mManufacturer;
} AudioClassDescription;

typedef struct AudioStreamBasicDescription
{
    Float64 mSampleRate;
    UInt32 mFormatID;
    UInt32 mFormatFlags;
    UInt32 mBytesPerPacket;
    UInt32 mFramesPerPacket;
    UInt32 mBytesPerFrame;
    UInt32 mChannelsPerFrame;
    UInt32 mBitsPerChannel;
    UInt32 mReserved;
} AudioStreamBasicDescription;

typedef struct AudioStreamPacketDescription
{
    SInt64 mStartOffset;
    UInt32 mVariableFramesInPacket;
    UInt32 mDataByteSize;
} AudioStreamPacketDescription;

typedef struct AudioTimeStamp
{
    Float64 mSampleTime;
    UInt64 mHostTime;
    Float64 mRateScalar;
    UInt64 mWordClockTime;
    SMPTETime mSMPTETime;
    UInt32 mFlags;
    UInt32 mReserved;
} AudioTimeStamp;

typedef struct AudioValueRange
{
    Float64 mMinimum;
    Float64 mMaximum;
} AudioValueRange;

typedef struct AudioValueTranslation
{
    void* mInputData;
    UInt32 mInputDataSize;
    void* mOutputData;
    UInt32 mOutputDataSize;
} AudioValueTranslation;

typedef struct AudioConverterPrimeInfo
{
    UInt32 leadingFrames;
    UInt32 trailingFrames;
} AudioConverterPrimeInfo;

typedef struct AudioCodecMagicCookieInfo
{
    UInt32 mMagicCookieSize;
    const void* mMagicCookie;
} AudioCodecMagicCookieInfo;

typedef struct AudioCodecMagicCookieInfo MagicCookieInfo;

typedef struct AudioCodecPrimeInfo
{
    UInt32 leadingFrames;
    UInt32 trailingFrames;
} AudioCodecPrimeInfo;

typedef struct ExtendedAudioFormatInfo
{
    AudioStreamBasicDescription mASBD;
    const void* mMagicCookie;
    UInt32 mMagicCookieSize;
    AudioClassDescription mClassDescription;
} ExtendedAudioFormatInfo;

typedef struct AudioPanningInfo
{
    UInt32 mPanningMode;
    UInt32 mCoordinateFlags;
    Float32 mCoordinates[3];
    Float32 mGainScale;
    const AudioChannelLayout* mOutputChannelMap;
} AudioPanningInfo;

typedef struct AudioFormatListItem
{
    AudioStreamBasicDescription mASBD;
    AudioChannelLayoutTag mChannelLayoutTag;
} AudioFormatListItem;

typedef struct AudioFormatInfo
{
    AudioStreamBasicDescription mASBD;
    const void* mMagicCookie;
    UInt32 mMagicCookieSize;
} AudioFormatInfo;

typedef struct AudioBalanceFade
{
    Float32 mLeftRightBalance;
    Float32 mBackFrontFade;
    UInt32 mType;
    const AudioChannelLayout* mChannelLayout;
} AudioBalanceFade;

typedef struct AudioComponentDescription
{
    OSType componentType;
    OSType componentSubType;
    OSType componentManufacturer;
    UInt32 componentFlags;
    UInt32 componentFlagsMask;
} AudioComponentDescription;

typedef OSStatus (*AudioConverterComplexInputDataProc) (
    AudioConverterRef inAudioConverter,
    UInt32* ioNumberDataPackets,
    AudioBufferList* ioData,
    AudioStreamPacketDescription** outDataPacketDescription,
    void* inUserData);

typedef OSStatus (*AudioConverterDisposeProc) (
    AudioConverterRef inAudioConverter);

typedef OSStatus (*AudioConverterNewProc) (
    const AudioStreamBasicDescription* inSourceFormat,
    const AudioStreamBasicDescription* inDestinationFormat,
    AudioConverterRef* outAudioConverter);

typedef OSStatus (*AudioConverterNewSpecificProc) (
    const AudioStreamBasicDescription* inSourceFormat,
    const AudioStreamBasicDescription* inDestinationFormat,
    UInt32 inNumberClassDescriptions,
    const AudioClassDescription* inClassDescriptions,
    AudioConverterRef* outAudioConverter);

typedef OSStatus (*AudioConverterResetProc) (
    AudioConverterRef inAudioConverter);

typedef OSStatus (*AudioConverterGetPropertyProc) (
    AudioConverterRef inAudioConverter,
    AudioConverterPropertyID inPropertyID,
    UInt32* ioPropertyDataSize,
    void* outPropertyData);

typedef OSStatus (*AudioConverterGetPropertyInfoProc) (
    AudioConverterRef inAudioConverter,
    AudioConverterPropertyID inPropertyID,
    UInt32* outSize,
    Boolean* outWritable);

typedef OSStatus (*AudioConverterSetPropertyProc) (
    AudioConverterRef inAudioConverter,
    AudioConverterPropertyID inPropertyID,
    UInt32 inPropertyDataSize,
    const void* inPropertyData);

typedef OSStatus (*AudioConverterConvertBufferProc) (
    AudioConverterRef inAudioConverter,
    UInt32 inInputDataSize,
    const void* inInputData,
    UInt32* ioOutputDataSize,
    void* outOutputData);

typedef OSStatus (*AudioConverterFillComplexBufferProc) (
    AudioConverterRef inAudioConverter,
    AudioConverterComplexInputDataProc inInputDataProc,
    void* inInputDataProcUserData,
    UInt32* ioOutputDataPacketSize,
    AudioBufferList* outOutputData,
    AudioStreamPacketDescription* outPacketDescription);

//typedef OSStatus (*AudioConverterConvertComplexBufferProc) (
//  AudioConverterRef inAudioConverter,
//  UInt32 inNumberPCMFrames,
//  const AudioBufferList* inInputData,
//  AudioBufferList* outOutputData);

typedef OSStatus (*AudioCodecGetPropertyInfoProc) (
    AudioCodec inCodec,
    AudioCodecPropertyID inPropertyID,
    UInt32* outSize,
    Boolean* outWritable);

typedef OSStatus (*AudioCodecGetPropertyProc) (
    AudioCodec inCodec,
    AudioCodecPropertyID inPropertyID,
    UInt32* ioPropertyDataSize,
    void* outPropertyData);

typedef OSStatus (*AudioCodecSetPropertyProc) (
    AudioCodec inCodec,
    AudioCodecPropertyID inPropertyID,
    UInt32 inPropertyDataSize,
    const void* inPropertyData);

typedef OSStatus (*AudioCodecInitializeProc) (
    AudioCodec inCodec,
    const AudioStreamBasicDescription* inInputFormat,
    const AudioStreamBasicDescription* inOutputFormat,
    const void* inMagicCookie,
    UInt32 inMagicCookieByteSize);

typedef OSStatus (*AudioCodecUninitializeProc) (
    AudioCodec inCodec);

typedef OSStatus (*AudioCodecAppendInputDataProc) (
    AudioCodec inCodec,
    const void* inInputData,
    UInt32* ioInputDataByteSize,
    UInt32* ioNumberPackets,
    const AudioStreamPacketDescription* inPacketDescription);

typedef OSStatus (*AudioCodecProduceOutputPacketsProc) (
    AudioCodec inCodec,
    void* outOutputData,
    UInt32* ioOutputDataByteSize,
    UInt32* ioNumberPackets,
    AudioStreamPacketDescription* outPacketDescription,
    UInt32* outStatus);

typedef OSStatus (*AudioCodecResetProc) (
    AudioCodec inCodec);

typedef OSStatus (*AudioFormatGetPropertyInfoProc) (
    AudioFormatPropertyID inPropertyID,
    UInt32 inSpecifierSize,
    const void* inSpecifier,
    UInt32* outPropertyDataSize);

typedef OSStatus (*AudioFormatGetPropertyProc) (
    AudioFormatPropertyID inPropertyID,
    UInt32 inSpecifierSize,
    const void* inSpecifier,
    UInt32* ioPropertyDataSize,
    void* outPropertyData);

typedef OSStatus (*AudioComponentCopyNameProc) (
    AudioComponent inComponent,
    CFStringRef* outName);

typedef UInt32 (*AudioComponentCountProc) (
    const AudioComponentDescription *inDesc);

typedef AudioComponent (*AudioComponentFindNextProc) (
    AudioComponent inComponent,
    const AudioComponentDescription *inDesc);

typedef OSStatus (*AudioComponentGetDescriptionProc) (
    AudioComponent inComponent,
    AudioComponentDescription *outDesc);

typedef Boolean (*AudioComponentInstanceCanDoProc) (
    AudioComponentInstance inInstance,
    SInt16 inSelectorID);

typedef OSStatus (*AudioComponentGetVersionProc) (
    AudioComponent inComponent,
    UInt32 *outVersion);

typedef OSStatus (*AudioComponentInstanceDisposeProc) (
    AudioComponentInstance inInstance);

typedef AudioComponent (*AudioComponentInstanceGetComponentProc) (
    AudioComponentInstance inInstance);

typedef OSStatus (*AudioComponentInstanceNewProc) (
    AudioComponent inComponent,
    AudioComponentInstance* outInstance);

typedef AudioComponent(*AudioComponentRegisterProc) (
    const AudioComponentDescription* inDesc,
    CFStringRef inName,
    UInt32 inVersion,
    CFPlugInFactoryFunction inFactory);

#ifdef __cplusplus
}
#endif

#pragma GCC diagnostic pop

#endif //#ifndef AVCODEC_APPLECA_DEFS_H