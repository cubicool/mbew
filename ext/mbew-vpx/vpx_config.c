#include "vpx/vpx_codec.h"
static const char* const cfg = "Custom CMAKE";
const char *vpx_codec_build_config(void) {return cfg;}
