#pragma once
#include <openssl/md5.h>
#include <openssl/sha.h>
#include <stdint.h>
#include <string.h>

static inline uint32_t ELFhash_32(const char *str);
static inline uint64_t ELFhash_64(const char *str);
static inline uint32_t md5_ELFhash_32(const char *str);
static inline uint64_t sha256_ELFhash_64(const char *str);

inline uint32_t ELFhash_32(const char *str)
{
    uint32_t hash=0, x;
    while (*str)
    {
        hash=(hash<<4)+*(str++);
        if((x=hash & 0xf0000000)!=0)
        {
            hash^=(x>>24);
        }
    }
    //hash&=0xfffffff;
    return hash;
}

inline uint64_t ELFhash_64(const char *str)
{
    uint64_t hash=0, x;
    while (*str)
    {
        hash=(hash<<4)+*(str++);
        if((x=hash&0xf000000000000000)!=0)
        {
            hash^=(x>>56);
        }
    }
    //hash&=0xfffffffffffffff;
    return hash;
}

inline uint32_t md5_ELFhash_32(const char *str)
{
    unsigned char result[MD5_DIGEST_LENGTH];
    MD5((const unsigned char *)str, strlen(str), result);
    uint32_t hash=0,x;
    for (uint8_t i=0; i!=MD5_DIGEST_LENGTH; i++)
    {
        hash=(hash<<4)+result[i];
        if ((x=hash & 0xf0000000)!=0)
        {
            hash^=(x>>24);
        }
    }
    return hash;
}

inline uint64_t sha256_ELFhash_64(const char *str)
{
    unsigned char result[SHA256_DIGEST_LENGTH];
    SHA256((const unsigned char *)str, strlen(str), result);
    uint64_t hash=0,x;
    for (uint8_t i=0; i!=SHA256_DIGEST_LENGTH; i++)
    {
        hash=(hash<<4)+result[i];
        if ((x=hash & 0xf000000000000000)!=0)
        {
            hash^=(x>>56);
        }
    }
    return hash;
}
