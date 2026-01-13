#include "ssh_algorithms.h"

#include <proto/amissl.h>

#include <string.h>
#include <openssl/evp.h>

/* ================================
 * Minimal modern algorithm sets
 * ================================ */

/* --- KEX --- */
/* Only modern curve25519 + optional alias. */

const ssh_algorithm ssh_kex_algorithms[] = {
    { SSH_ALG_KEX, "curve25519-sha256",              "X25519", 1 },
    { SSH_ALG_KEX, "curve25519-sha256@libssh.org",   "X25519", 0 }, /* optional alias */
};
const size_t ssh_kex_algorithms_count =
    sizeof(ssh_kex_algorithms) / sizeof(ssh_kex_algorithms[0]);

/* --- Host keys --- */
/* Ed25519 + RSA with SHA-256 (no ssh-rsa/SHA1). */

const ssh_algorithm ssh_hostkey_algorithms[] = {
    { SSH_ALG_HOSTKEY, "ssh-ed25519",   "ED25519", 1 },
    { SSH_ALG_HOSTKEY, "rsa-sha2-256",  "RSA",     1 },
};
const size_t ssh_hostkey_algorithms_count =
    sizeof(ssh_hostkey_algorithms) / sizeof(ssh_hostkey_algorithms[0]);

/* --- Ciphers --- */
/* AEAD + CTR (CTR requires MAC). */

const ssh_algorithm ssh_cipher_algorithms[] = {
    { SSH_ALG_CIPHER, "chacha20-poly1305@openssh.com", "CHACHA20-POLY1305", 1 },

    { SSH_ALG_CIPHER, "aes128-gcm@openssh.com",        "AES-128-GCM",        1 },
    { SSH_ALG_CIPHER, "aes256-gcm@openssh.com",        "AES-256-GCM",        1 },

    { SSH_ALG_CIPHER, "aes128-ctr",                    "AES-128-CTR",        1 },
    { SSH_ALG_CIPHER, "aes256-ctr",                    "AES-256-CTR",        1 },
};
const size_t ssh_cipher_algorithms_count =
    sizeof(ssh_cipher_algorithms) / sizeof(ssh_cipher_algorithms[0]);

/* --- MACs --- */
/* SHA-2 only; SHA-256 default, 512 optional. */

const ssh_algorithm ssh_mac_algorithms[] = {
    { SSH_ALG_MAC, "hmac-sha2-256", "SHA256", 1 },
    { SSH_ALG_MAC, "hmac-sha2-512", "SHA512", 0 }, /* optional */
};
const size_t ssh_mac_algorithms_count =
    sizeof(ssh_mac_algorithms) / sizeof(ssh_mac_algorithms[0]);

/* --- Compression --- */
/* Only "none" to keep it simple and modern. */

const ssh_algorithm ssh_compression_algorithms[] = {
    { SSH_ALG_COMPRESSION, "none", NULL, 1 },
};
const size_t ssh_compression_algorithms_count =
    sizeof(ssh_compression_algorithms) / sizeof(ssh_compression_algorithms[0]);


/* ================================
 * Backend checks (minimal)
 * ================================ */

static int cipher_supported(const char *backend_name)
{
    if (!backend_name)
        return 1;

    int supported = 0;
    EVP_CIPHER *c = IAmiSSL->EVP_CIPHER_fetch(NULL, backend_name, NULL);
    if (c) {
        supported = 1;
        IAmiSSL->EVP_CIPHER_free(c);
    }
    return supported;
}

static int md_supported(const char *backend_name)
{
    if (!backend_name)
        return 1;

    int supported = 0;
    EVP_MD *md = IAmiSSL->EVP_MD_fetch(NULL, backend_name, NULL);
    if (md) {
        supported = 1;
        IAmiSSL->EVP_MD_free(md);
    }
    return supported;
}

/* For minimal profile: assume AmiSSL/OpenSSL has X25519, ED25519, RSA.
 * If you want to be strict, you can add EVP_PKEY_CTX checks here.
 */
static int kex_supported(const ssh_algorithm *alg)
{
    (void)alg;
    return 1;
}

static int hostkey_supported(const ssh_algorithm *alg)
{
    (void)alg;
    return 1;
}


/* ================================
 * Name-list builder
 * ================================ */

static void append_name(char *out, size_t outlen,
                        const char *text, int *first_flag)
{
    size_t used = strlen(out);

    if (!*first_flag) {
        if (used + 1 >= outlen)
            return;
        out[used++] = ',';
        out[used] = '\0';
    }

    if (used < outlen) {
        strncat(out, text, outlen - used - 1);
    }
    *first_flag = 0;
}

int ssh_build_namelist(const ssh_algorithm *algs, size_t count,
                       char *out, size_t outlen)
{
    if (!out || outlen == 0)
        return -1;

    out[0] = '\0';
    int first = 1;

    for (size_t i = 0; i < count; ++i) {
        const ssh_algorithm *alg = &algs[i];
        if (!alg->enabled_by_default)
            continue;

        int supported = 1;

        switch (alg->type) {
            case SSH_ALG_CIPHER:
                supported = cipher_supported(alg->backend_name);
                break;
            case SSH_ALG_MAC:
                supported = md_supported(alg->backend_name);
                break;
            case SSH_ALG_KEX:
                supported = kex_supported(alg);
                break;
            case SSH_ALG_HOSTKEY:
                supported = hostkey_supported(alg);
                break;
            case SSH_ALG_COMPRESSION:
                supported = 1;
                break;
        }

        if (!supported)
            continue;

        append_name(out, outlen, alg->ssh_name, &first);
    }

    return 0;
}

/* Convenience wrappers */

int ssh_build_namelist_kex(char *out, size_t outlen)
{
    return ssh_build_namelist(ssh_kex_algorithms, ssh_kex_algorithms_count,
                              out, outlen);
}

int ssh_build_namelist_hostkey(char *out, size_t outlen)
{
    return ssh_build_namelist(ssh_hostkey_algorithms, ssh_hostkey_algorithms_count,
                              out, outlen);
}

int ssh_build_namelist_cipher(char *out, size_t outlen)
{
    return ssh_build_namelist(ssh_cipher_algorithms, ssh_cipher_algorithms_count,
                              out, outlen);
}

int ssh_build_namelist_mac(char *out, size_t outlen)
{
    return ssh_build_namelist(ssh_mac_algorithms, ssh_mac_algorithms_count,
                              out, outlen);
}

int ssh_build_namelist_compression(char *out, size_t outlen)
{
    return ssh_build_namelist(ssh_compression_algorithms,
                              ssh_compression_algorithms_count,
                              out, outlen);
}
