#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "file_kmg.h"
#include "file_common.h"
#include "pvr_texture.h"

// KMG format constants (KallistiOS)
#define KMG_MAGIC        0x00474d4b  /* 'KMG\0' */
#define KMG_VERSION      1
#define KMG_PLAT_DC      1

#define KMG_DCFMT_4BPP_PAL   0x01
#define KMG_DCFMT_8BPP_PAL   0x02
#define KMG_DCFMT_RGB565     0x03
#define KMG_DCFMT_ARGB4444   0x04
#define KMG_DCFMT_ARGB1555   0x05
#define KMG_DCFMT_YUV422     0x06
#define KMG_DCFMT_BUMP       0x07

#define KMG_DCFMT_VQ         0x0100
#define KMG_DCFMT_TWIDDLED   0x0200
#define KMG_DCFMT_MIPMAP     0x0400

static unsigned kmg_dc_base_format(ptPixelFormat fmt) {
	switch(fmt) {
	case PT_RGB565:     return KMG_DCFMT_RGB565;
	case PT_ARGB4444:   return KMG_DCFMT_ARGB4444;
	case PT_ARGB1555:   return KMG_DCFMT_ARGB1555;
	case PT_YUV:        return KMG_DCFMT_YUV422;
	case PT_NORMAL:     return KMG_DCFMT_BUMP;
	case PT_PALETTE_4B: return KMG_DCFMT_4BPP_PAL;
	case PT_PALETTE_8B: return KMG_DCFMT_8BPP_PAL;
	default:
		// hw_pixel_format should never be pseudo here.
		return 0;
	}
}

void fKmgWrite(const PvrTexEncoder *pte, const char *outfname) {
	assert(pte);
	assert(outfname);
	assert(pte->pvr_tex);

	FILE *f = fopen(outfname, "wb");
	assert(f);

	unsigned fmt = kmg_dc_base_format(pte->hw_pixel_format);
	assert(fmt != 0);

	if (pteIsCompressed(pte))
		fmt |= KMG_DCFMT_VQ;
	if (!pteIsStrided(pte))
		fmt |= KMG_DCFMT_TWIDDLED;
	if (pteHasMips(pte))
		fmt |= KMG_DCFMT_MIPMAP;

	// Payload sizing:
	// - Uncompressed: just the PVR texture payload bytes.
	// - Compressed: full 2048-byte codebook + indices.
	//   KOS vqenc also writes a dummy 0 byte when mipmaps are present.
	size_t payload_bytes = 0;
	if (pteIsCompressed(pte)) {
		size_t idx_bytes = CalcTextureSize(pte->w, pte->h, pte->pixel_format, pteHasMips(pte), 1, 0);
		payload_bytes = PVR_CODEBOOK_SIZE_BYTES + idx_bytes + (pteHasMips(pte) ? 1 : 0);
	} else {
		payload_bytes = CalcTextureSize(pte->w, pte->h, pte->pixel_format, pteHasMips(pte), 0, 0);
	}

	// Header (64 bytes, all little-endian)
	Write32LE(KMG_MAGIC, f);
	Write32LE(KMG_VERSION, f);
	Write32LE(KMG_PLAT_DC, f);
	Write32LE(fmt, f);
	Write32LE(pte->w, f);
	Write32LE(pte->h, f);
	Write32LE((unsigned)payload_bytes, f);
	WritePadZero(36, f);

	// Payload
	if (pteIsCompressed(pte)) {
		assert(pte->pvr_codebook);
		CheckedFwrite(pte->pvr_codebook, PVR_CODEBOOK_SIZE_BYTES, f);
		if (pteHasMips(pte)) {
			Write8(0, f);
		}
		{
			size_t idx_bytes = CalcTextureSize(pte->w, pte->h, pte->pixel_format, pteHasMips(pte), 1, 0);
			CheckedFwrite(pte->pvr_tex, idx_bytes, f);
		}
	} else {
		size_t bytes = CalcTextureSize(pte->w, pte->h, pte->pixel_format, pteHasMips(pte), 0, 0);
		CheckedFwrite(pte->pvr_tex, bytes, f);
	}

	fclose(f);
}
