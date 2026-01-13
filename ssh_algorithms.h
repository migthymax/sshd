#pragma once

#include <stddef.h>

typedef enum {
    SSH_ALG_KEX,
    SSH_ALG_HOSTKEY,
    SSH_ALG_CIPHER,
    SSH_ALG_MAC,
    SSH_ALG_COMPRESSION
} ssh_alg_type;

typedef struct {
    ssh_alg_type type;

    const char *ssh_name;      /* SSH wire name, e.g. "aes128-ctr"         */
    const char *backend_name;  /* OpenSSL/AmiSSL name, e.g. "AES-128-CTR"  */

    int enabled_by_default;    /* 1 = offered by default */
} ssh_algorithm;

/* Tables */
extern const ssh_algorithm ssh_kex_algorithms[];
extern const size_t ssh_kex_algorithms_count;

extern const ssh_algorithm ssh_hostkey_algorithms[];
extern const size_t ssh_hostkey_algorithms_count;

extern const ssh_algorithm ssh_cipher_algorithms[];
extern const size_t ssh_cipher_algorithms_count;

extern const ssh_algorithm ssh_mac_algorithms[];
extern const size_t ssh_mac_algorithms_count;

extern const ssh_algorithm ssh_compression_algorithms[];
extern const size_t ssh_compression_algorithms_count;

/* Build SSH name-lists for SSH_MSG_KEXINIT */
int ssh_build_namelist_kex(char *out, size_t outlen);
int ssh_build_namelist_hostkey(char *out, size_t outlen);
int ssh_build_namelist_cipher(char *out, size_t outlen);
int ssh_build_namelist_mac(char *out, size_t outlen);
int ssh_build_namelist_compression(char *out, size_t outlen);

/* Generic helper (if you need it) */
int ssh_build_namelist(const ssh_algorithm *algs, size_t count,
                       char *out, size_t outlen);

