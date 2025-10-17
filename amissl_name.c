#include "amissl_name.h"

static const char *AmiSSLVersionNames[] = {
	"Invalid",        // index 0 placeholder (enum starts at 1)
	"AmiSSLv2",
	"AmiSSLv3.6/3.7",
	"AmiSSLv097m(Unrelease)",
	"AmiSSLv098y(Unrelease)",
	"AmiSSLv102f(Unrelease)",
	"AmiSSLvV110c(Unrelease)",
	"AmiSSLv4.0",
	"AmiSSLv4.1",
	"AmiSSLv4.2",
	"AmiSSLv4.3",
	"AmiSSLv4.4/4.5",
	"AmiSSLv4.6",
	"AmiSSLv4.7",
	"AmiSSLv4.8",
	"AmiSSLv4.9",
	"AmiSSLv4.10/4.11",
	"AmiSSLv4.12",
	"AmiSSLv300(Unrelease)",
	"AmiSSLv301(Unrelease)",
	"AmiSSLv302(Unrelease)",
	"AmiSSLv5.1",
	"AmiSSLv5.2",
	"AmiSSLv5.3",
	"AmiSSLv5.4",
	"AmiSSLv5.5/5.6",
	"AmiSSLv5.7",
	"AmiSSLv5.8",
	"AmiSSLv5.9",
	"AmiSSLv5.10",
	"AmiSSLv5.11",
	"AmiSSLv5.12",
	"AmiSSLv5.13",
	"AmiSSLv5.14",
	"AmiSSLv5.15",
	"AmiSSLv5.16",
	"AmiSSLv5.17",
	"AmiSSLv5.18",
	"AmiSSLv5.19",
	"AmiSSLv5.20"
};

const char *getAmiSSLVersionName( enum AmiSSLVersion version ) {
    if (version > 0 && version < AMISSL_VMAX)
        return AmiSSLVersionNames[version];
    return "Unknow";
}