#pragma once

#include "pvr_texture_encoder.h"

// Write a Dreamcast KMG container.
// Format is compatible with KallistiOS kmg loader expectations:
// - 64-byte little-endian header
// - then raw texture payload (codebook + indices for VQ)
void fKmgWrite(const PvrTexEncoder *pte, const char *outfname);
