#ifndef __SHA_H__
#define __SHA_H__

#include <string>
#include <cstring>
#include <fstream>
#include <cstdint>

namespace alex { namespace crypt
{
    class SHA256
    {
    protected:
        const static uint32_t sha256_k[];
        static const unsigned int SHA_256_BLOCK_SIZE = (512 / 8);
    public:
        void init();
        void update(const unsigned char* message, unsigned int len);
        void final(unsigned char* digest);

        static const unsigned int DIGEST_SIZE = (256 / 8);
    protected:
        void transform(const unsigned char* message, unsigned int block_nb);
        unsigned int m_tot_len;
        unsigned int m_len;
        unsigned char m_block[2 * SHA_256_BLOCK_SIZE];
        uint32_t m_h[8];
    };


    #define SHA2_SHFR(x, n)    (x >> n)
    #define SHA2_ROTR(x, n)   ((x >> n) | (x << ((sizeof(x) << 3) - n)))
    #define SHA2_ROTL(x, n)   ((x << n) | (x >> ((sizeof(x) << 3) - n)))
    #define SHA2_CH(x, y, z)  ((x & y) ^ (~x & z))
    #define SHA2_MAJ(x, y, z) ((x & y) ^ (x & z) ^ (y & z))
    #define SHA256_F1(x) (SHA2_ROTR(x,  2) ^ SHA2_ROTR(x, 13) ^ SHA2_ROTR(x, 22))
    #define SHA256_F2(x) (SHA2_ROTR(x,  6) ^ SHA2_ROTR(x, 11) ^ SHA2_ROTR(x, 25))
    #define SHA256_F3(x) (SHA2_ROTR(x,  7) ^ SHA2_ROTR(x, 18) ^ SHA2_SHFR(x,  3))
    #define SHA256_F4(x) (SHA2_ROTR(x, 17) ^ SHA2_ROTR(x, 19) ^ SHA2_SHFR(x, 10))
    #define SHA2_UNPACK32(x, str)                 \
    {                                             \
        *((str) + 3) = (uint8_t) ((x)      );       \
        *((str) + 2) = (uint8_t) ((x) >>  8);       \
        *((str) + 1) = (uint8_t) ((x) >> 16);       \
        *((str) + 0) = (uint8_t) ((x) >> 24);       \
    }
    #define SHA2_PACK32(str, x)                   \
    {                                             \
        *(x) =   ((uint32_t) *((str) + 3)      )    \
               | ((uint32_t) *((str) + 2) <<  8)    \
               | ((uint32_t) *((str) + 1) << 16)    \
               | ((uint32_t) *((str) + 0) << 24);   \
    }

    #define SHA2_UNPACK64(x, str)                 \
    {                                             \
        *((str) + 7) = (uint8_t) ((x)      );       \
        *((str) + 6) = (uint8_t) ((x) >>  8);       \
        *((str) + 5) = (uint8_t) ((x) >> 16);       \
        *((str) + 4) = (uint8_t) ((x) >> 24);       \
        *((str) + 3) = (uint8_t) ((x) >> 32);       \
        *((str) + 2) = (uint8_t) ((x) >> 40);       \
        *((str) + 1) = (uint8_t) ((x) >> 48);       \
        *((str) + 0) = (uint8_t) ((x) >> 56);       \
    }
    #define SHA2_PACK64(str, x)                   \
    {                                             \
        *(x) =   ((uint64_t) *((str) + 7)      )    \
               | ((uint64_t) *((str) + 6) <<  8)    \
               | ((uint64_t) *((str) + 5) << 16)    \
               | ((uint64_t) *((str) + 4) << 24)    \
               | ((uint64_t) *((str) + 3) << 32)    \
               | ((uint64_t) *((str) + 2) << 40)    \
               | ((uint64_t) *((str) + 1) << 48)    \
               | ((uint64_t) *((str) + 0) << 56);   \
    }

    const uint32_t SHA256::sha256_k[64] =
    {
        0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
        0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
        0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
        0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
        0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
        0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
        0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
        0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
        0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
        0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
        0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
        0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
        0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
        0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
        0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
        0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
    };

    void SHA256::transform(const unsigned char* message, unsigned int block_nb)
    {
        uint32_t w[64];
        uint32_t wv[8];
        uint32_t t1, t2;
        const unsigned char* sub_block;
        int i;
        int j;
        for (i = 0; i < (int) block_nb; i++)
        {
            sub_block = message + (i << 6);
            for (j = 0; j < 16; j++)
            {
                SHA2_PACK32(&sub_block[j << 2], &w[j]);
            }
            for (j = 16; j < 64; j++)
            {
                w[j] =  SHA256_F4(w[j -  2]) + w[j -  7] + SHA256_F3(w[j - 15]) + w[j - 16];
            }
            for (j = 0; j < 8; j++)
            {
                wv[j] = m_h[j];
            }
            for (j = 0; j < 64; j++)
            {
                t1 = wv[7] + SHA256_F2(wv[4]) + SHA2_CH(wv[4], wv[5], wv[6])
                     + sha256_k[j] + w[j];
                t2 = SHA256_F1(wv[0]) + SHA2_MAJ(wv[0], wv[1], wv[2]);
                wv[7] = wv[6];
                wv[6] = wv[5];
                wv[5] = wv[4];
                wv[4] = wv[3] + t1;
                wv[3] = wv[2];
                wv[2] = wv[1];
                wv[1] = wv[0];
                wv[0] = t1 + t2;
            }
            for (j = 0; j < 8; j++)
            {
                m_h[j] += wv[j];
            }
        }
    }

    void SHA256::init()
    {
        m_h[0] = 0x6a09e667;
        m_h[1] = 0xbb67ae85;
        m_h[2] = 0x3c6ef372;
        m_h[3] = 0xa54ff53a;
        m_h[4] = 0x510e527f;
        m_h[5] = 0x9b05688c;
        m_h[6] = 0x1f83d9ab;
        m_h[7] = 0x5be0cd19;
        m_len = 0;
        m_tot_len = 0;
    }

    void SHA256::update(const unsigned char* message, unsigned int len)
    {
        unsigned int block_nb;
        unsigned int new_len, rem_len, tmp_len;
        const unsigned char* shifted_message;
        tmp_len = SHA_256_BLOCK_SIZE - m_len;
        rem_len = len < tmp_len ? len : tmp_len;
        memcpy(&m_block[m_len], message, rem_len);
        if (m_len + len < SHA_256_BLOCK_SIZE)
        {
            m_len += len;
            return;
        }
        new_len = len - rem_len;
        block_nb = new_len / SHA_256_BLOCK_SIZE;
        shifted_message = message + rem_len;
        transform(m_block, 1);
        transform(shifted_message, block_nb);
        rem_len = new_len % SHA_256_BLOCK_SIZE;
        memcpy(m_block, &shifted_message[block_nb << 6], rem_len);
        m_len = rem_len;
        m_tot_len += (block_nb + 1) << 6;
    }

    void SHA256::final(unsigned char* digest)
    {
        unsigned int block_nb;
        unsigned int pm_len;
        unsigned int len_b;
        int i;
        block_nb = (1 + ((SHA_256_BLOCK_SIZE - 9)
                         < (m_len % SHA_256_BLOCK_SIZE)));
        len_b = (m_tot_len + m_len) << 3;
        pm_len = block_nb << 6;
        memset(m_block + m_len, 0, pm_len - m_len);
        m_block[m_len] = 0x80;
        SHA2_UNPACK32(len_b, m_block + pm_len - 4);
        transform(m_block, block_nb);
        for (i = 0 ; i < 8; i++)
        {
            SHA2_UNPACK32(m_h[i], &digest[i << 2]);
        }
    }

    std::string sha256(const std::string& x);
    template <unsigned int N> std::string hexdigest(const std::string& x);
    
    template <unsigned int N> std::string hexdigest(const std::string& x)
    {
        unsigned char digest[SHA256::DIGEST_SIZE];
        SHA256 ctx = SHA256();
        ctx.init();
        ctx.update((unsigned char*)x.c_str(), x.size());
        ctx.final(digest);

        char buf[N+1]; buf[N] = 0;
        for (unsigned int i = 0; i < N; i++)
        {
            sprintf(buf + i, "%x", digest[i] % 16);
        }
        return std::string(buf);
    }

    std::string sha256(const std::string& x)
    {
        unsigned char digest[SHA256::DIGEST_SIZE];
        SHA256 ctx = SHA256();
        ctx.init();
        ctx.update( (unsigned char*)x.c_str(), x.length());
        ctx.final(digest);

        char buf[2 * SHA256::DIGEST_SIZE + 1];
        buf[2 * SHA256::DIGEST_SIZE] = 0;
        for (int i = 0; i < SHA256::DIGEST_SIZE; i++)
        {
            sprintf(buf + i * 2, "%02x", digest[i]);
        }
        return std::string(buf);
    }
}}

#endif