/*
 * tfa98xxParameters.h
 *
 *  Created on: Jul 22, 2013
 *      Author: NLV02095
 */

#ifndef TFA98XXPARAMETERS_H_
#define TFA98XXPARAMETERS_H_

#include <stdint.h>
#include "nxpTfa98xx.h"

// the pack pragma is required to make that the size in memory
// matches the actual variable lenghts
// This is to assure that the binary files can be transported between
// different platforms.
#pragma pack (push, 1)

/*
 * typedef for 24 bit value using 3 bytes
 */
typedef struct uint24 {
  uint8_t b[3];
} uint24_t;
/*
 * the generic header
 *   all char types are in ASCII
 */
typedef struct nxpTfaHeader {
    char id[2];          // "XX" : XX=type
    char version[2];     // "V_" : V=version, vv=subversion
    char subversion[2];  // "vv" : vv=subversion
    uint16_t size;       // data size in bytes following CRC
    uint32_t CRC;        // 32-bits CRC for following data
    char customer[8];    // “name of customer”
    char application[8]; // “application name”
    char type[8];		 // “application type name”
} nxpTfaHeader_t;

typedef enum nxpTfaSamplerate {
	fs_8k,       // 8kHz
	fs_11k025,   // 11.025kHz
	fs_12k,      // 12kHz
	fs_16k,      // 16kHz
	fs_22k05,    // 22.05kHz
	fs_24k,      // 24kHz
	fs_32k,      // 32kHz
	fs_44k1,     // 44.1kHz
	fs_48k,      // 48kHz
	fs_96k       // 96kHz
} nxpTfaSamplerate_t;

/*
 * the biquad coefficients for the API together with index in filter
 *  the biquad_index is the actual index in the equalizer +1
 */
typedef struct nxpTfaBiquad {
  float b0;
  float b1;
  float b2;
  float a1;
  float a2;
} nxpTfaBiquad_t;

typedef enum nxpTfaFilterType {
	fCustom,         //User defined biquad coefficients
	fFlat,           //Vary only gain
	fLowpass,        //2nd order Butterworth low pass
	fHighpass,       //2nd order Butterworth high pass
	fLowshelf,
	fHighshelf,
	fNotch,
	fPeak,
	fBandpass,
	f1stLP,
	f1stHP,
	fCount
} nxpTfaFilterType_t;

/*
 * filter parameters for biquad (re-)calculation
 */
typedef struct nxpTfaFilter {
  nxpTfaBiquad_t biquad;
  uint8_t enabled;
  uint8_t type; // (== enum FilterTypes, assure 8bits length)
  float frequency;
  float Q;
  float gain;
} nxpTfaFilter_t ;  //8 * float + int32 + byte == 37

#define TFA98XX_MAX_EQ 10
typedef struct nxpTfaEqualizer {
  uint8_t samplerate; 				 // ==enum samplerates, assure 8 bits
  nxpTfaFilter_t filter[TFA98XX_MAX_EQ];// note: API index counts from 1..10
} nxpTfaEqualizer_t;

/*
 * files
 */
#define HDR(c1,c2) (c2<<8|c1) // little endian
typedef enum nxpTfaHeaderType {
    paramsHdr       = HDR('P','M'),
    volstepHdr	 	= HDR('V','P'),
    patchHdr	 	= HDR('P','A'),
    speakerHdr	 	= HDR('S','P'),
    presetHdr	 	= HDR('P','R'),
    configHdr	 	= HDR('C','O'),
    equalizerHdr    = HDR('E','Q'),
    drcHdr		 	= HDR('D','R')
} nxpTfaHeaderType_t;

/*
 * equalizer file
 */
#define NXPTFA_EQ_VERSION    "1"
#define NXPTFA_EQ_SUBVERSION "00"
typedef struct nxpTfaEqualizerFile {
	nxpTfaHeader_t hdr;
	nxpTfaEqualizer_t eq; //payload
} nxpTfaEqualizerFile_t;

/*
 * preset file
 */
#define NXPTFA_PR_VERSION    "1"
#define NXPTFA_PR_SUBVERSION "00"

typedef struct nxpTfaPresetFile {
	nxpTfaHeader_t hdr;
	uint8_t preset[87]; // only works for fixed length
} nxpTfaPreset_t;
/*
 * volume step structures
 */
// VP01
#define NXPTFA_VP1_VERSION    "1"
#define NXPTFA_VP1_SUBVERSION "01"
typedef struct nxpTfaVolumeStep1 {
    float attenuation;              // IEEE single float
    uint8_t preset[TFA98XX_PRESET_LENGTH];
} nxpTfaVolumeStep1_t;

// VP02
#define NXPTFA_VP2_VERSION    "2"
#define NXPTFA_VP2_SUBVERSION "01"
typedef struct nxpTfaVolumeStep2 {
    float attenuation;              // IEEE single float
    uint8_t preset[TFA98XX_PRESET_LENGTH];
    nxpTfaEqualizer_t eq;
} nxpTfaVolumeStep2_t;

// VP03 is obsolete

// VP04
#define NXPTFA_VP4_VERSION    "4"
#define NXPTFA_VP4_SUBVERSION "01"
typedef struct nxpTfaVolumeStep4 {
    float attenuation;              // IEEE single float
    uint8_t preset[TFA98XX_PRESET_LENGTH];
    nxpTfaEqualizer_t eq;
    uint8_t drc[TFA98XX_DRC_LENGTH];
} nxpTfaVolumeStep4_t;
/*
 * volumestep file
 */
typedef struct nxpTfaVolumeStepFile {
	nxpTfaHeader_t hdr;
	uint8_t vsteps;  	// can also be calulated from size+type
	uint8_t payload; 	//start of variable length contents:N times volsteps
}nxpTfaVolumeStepFile_t;
/*
 * speaker file
 */
#define NXPTFA_SP_VERSION    "1"
#define NXPTFA_SP_SUBVERSION "00"
typedef struct nxpTfaSpeakerFile {
	nxpTfaHeader_t hdr;
	char name[8];				// speaker nick name (e.g. “dumbo”)
	char vendor[16];
	char type[8];
	//	dimensions (mm)
	uint8_t height;
	uint8_t width;
	uint8_t depth;
	uint8_t ohm;
	uint8_t data[TFA98XX_SPEAKERPARAMETER_LENGTH]; //payload
} nxpTfaSpeakerFile_t;

/*
 * parameter container file
 */
/*
 * descriptors
 */
typedef enum nxpTfaDescriptorType {
	dscDevice,		// device list
	dscProfile,		// profile list
	dscRegister,	// register patch
	dscString,		// ascii, zero terminated string
	dscFile,		// filename + file contents
	dscBitfieldBase=0x80 // start of bitfield enums
} nxpTfaDescriptorType_t;

typedef struct nxpTfaDescPtr {
	uint8_t  type; // (== enum nxpTfaDescriptorType, assure 8bits length)
	uint32_t offset:24;
	//	uint8_t offset[3];
}nxpTfaDescPtr_t;
/*
 * device descriptor list
 */
typedef struct nxpTfaDeviceList {
	uint8_t length;			// nr of items in the list
	uint8_t bus;			// bus
	uint8_t dev;			// device
	uint8_t func;			// subfunction or subdevice
	nxpTfaDescPtr_t name;	// device name
	nxpTfaDescPtr_t list[];	// items list
} nxpTfaDeviceList_t;

/*
 * profile descriptor list
 */
typedef struct nxpTfaProfileList {
	uint8_t length;			// nr of items in the list
	uint24_t ID;			// profile ID
	nxpTfaDescPtr_t name;	// profile name
	nxpTfaDescPtr_t list[];	// items list
} nxpTfaProfileList_t;
/*
 * Bitfield descriptor
 */
typedef struct nxpTfaBitfield {
	uint16_t  field; // ==datasheet defined, 16 bits
	uint16_t  value;
} nxpTfaBitfield_t;
/*
 * Bitfield enumuration bits descriptor
 */
typedef struct nxpTfaBfEnum {
	uint16_t  len:4;
	uint16_t  pos:4;
	uint16_t  address:8;
} nxpTfaBfEnum_t;

/*
 * Register patch descriptor
 */
typedef struct nxpTfaRegpatch {
	uint8_t   address;	// register address
	uint16_t  value;	// value to write
	uint16_t  mask;		// mask of bits to write
} nxpTfaRegpatch_t;

/*
 * the container file
 *   - the size field is 32bits long (generic=16)
 *   - all char types are in ASCII
 */
typedef struct nxpTfaContainer {
    char id[2];          // "XX" : XX=type
    char version[2];     // "V_" : V=version, vv=subversion
    char subversion[2];  // "vv" : vv=subversion
    uint32_t size;       // data size in bytes following CRC
    uint32_t CRC;        // 32-bits CRC for following data
    uint16_t rev;		 // "extra chars for rev nr"
    char customer[8];    // “name of customer”
    char application[8]; // “application name”
    char type[8];		 // “application type name”
    uint32_t index[];	 // start of item index table
} nxpTfaContainer_t;

#pragma pack (pop)

/*
 * bitfield enums (generated from tfa9890)
 */


#endif /* TFA98XXPARAMETERS_H_ */
