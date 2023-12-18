#include "MRCrypt.h"
//-----------------------------------------------------------------------------
std::string MRCrypt::StringToSHA(MRCrypt::SHAType sha_type, const std::string& s)
{
    constexpr char HEX_CHARS[] = "0123456789ABCDEF";

    unsigned long sha_size = 0;
    switch (sha_type)
    {
    case MRCrypt::SHAType::SHA256: sha_size = 32; break;
    case MRCrypt::SHAType::SHA384: sha_size = 48; break;
    case MRCrypt::SHAType::SHA512: sha_size = 64; break;
    }

    std::vector<unsigned char> vector_hash;
    bool is_ok = false;
#ifdef WIN32
    HCRYPTPROV HCryptoProv = 0;
    if (CryptAcquireContext(&HCryptoProv, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT) == TRUE)
    {
        int method = 0;
        switch (sha_type)
        {
        case SHAType::SHA256: method = CALG_SHA_256; break;
        case SHAType::SHA384: method = CALG_SHA_384; break;
        case SHAType::SHA512: method = CALG_SHA_512; break;
        }

        HCRYPTHASH CryptoHash = 0;
        if (CryptCreateHash(HCryptoProv, method, 0, 0, &CryptoHash) == TRUE)
        {
            if (CryptHashData(CryptoHash, (unsigned char*)s.c_str(), (DWORD)s.size(), 0) == TRUE)
            {
                vector_hash.resize(sha_size);
                if (CryptGetHashParam(CryptoHash, HP_HASHVAL, &vector_hash[0], &sha_size, 0) == TRUE)
                {
                    is_ok = true;
                }
            }
            CryptDestroyHash(CryptoHash);
        }
        CryptReleaseContext(HCryptoProv, 0);
    }
#else
    if (sha_type == MRCrypt::SHAType::SHA256)
    {
        SHA256_CTX Context;
        if (SHA256_Init(&Context) == 1)
        {
            if (SHA256_Update(&Context, s.c_str(), s.size()) == 1)
            {
                vector_hash.resize(sha_size);
                is_ok = SHA256_Final(&vector_hash[0], &Context) == 1;
            }
        }
    }
    else if (sha_type == MRCrypt::SHAType::SHA384)
    {
        SHA512_CTX Context;
        if (SHA384_Init(&Context) == 1)
        {
            if (SHA384_Update(&Context, s.c_str(), s.size()) == 1)
            {
                vector_hash.resize(sha_size);
                is_ok = SHA384_Final(&vector_hash[0], &Context) == 1;
            }
        }
    }
    else if (sha_type == MRCrypt::SHAType::SHA512)
    {
        SHA512_CTX Context;
        if (SHA512_Init(&Context) == 1)
        {
            if (SHA512_Update(&Context, s.c_str(), s.size()) == 1)
            {
                vector_hash.resize(sha_size);
                is_ok = SHA512_Final(&vector_hash[0], &Context) == 1;
            }
        }
    }
#endif
    std::string res;
    if (is_ok)
    {
        res.resize(sha_size * 2);
        size_t idx = 0;
        for (size_t i = 0; i < sha_size; ++i, ++idx)
        {
            res[idx] = HEX_CHARS[vector_hash[i] >> 4];
            res[++idx] = HEX_CHARS[vector_hash[i] & 0xF];
        }
    }
    return res;
}
//-----------------------------------------------------------------------------
