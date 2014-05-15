// example2.c - Simple demonstration of miniz.c's ZIP archive API's.
// Public domain, May 15 2011, Rich Geldreich, richgel99@gmail.com
#include <string.h>

#include "miniz.h"
#include "main.h"

// The string to compress.
static const char *s_pStr = "Good morning Dr. Chandra. This is Hal. I am ready for my first lesson." \
  "Good morning Dr. Chandra. This is Hal. I am ready for my first lesson." \
  "Good morning Dr. Chandra. This is Hal. I am ready for my first lesson." \
  "Good morning Dr. Chandra. This is Hal. I am ready for my first lesson." \
  "Good morning Dr. Chandra. This is Hal. I am ready for my first lesson." \
  "Good morning Dr. Chandra. This is Hal. I am ready for my first lesson." \
  "Good morning Dr. Chandra. This is Hal. I am ready for my first lesson.";

uint8_t miniz_test (int32_t argc, char *argv[])
{
  uint32_t step = 0;
  int32_t cmp_status;
  unsigned long src_len = (unsigned long)strlen(s_pStr);
  unsigned long cmp_len = mz_compressBound(src_len);
  unsigned long uncomp_len = src_len;
  uint8_t *pCmp, *pUncomp;
  uint32_t total_succeeded = 0;

  printf("miniz.c version: %s\n", MZ_VERSION);

  do
  {
    // Allocate buffers to hold compressed and uncompressed data.
    pCmp = (unsigned char *)malloc((uint32_t)cmp_len);
    pUncomp = (unsigned char *)malloc((uint32_t)src_len);
    if ((!pCmp) || (!pUncomp))
    {
      printf("Out of memory!\n");
      return EXIT_FAILURE;
    }

    // Compress the string.
    cmp_status = mz_compress(pCmp, &cmp_len, (const unsigned char *)s_pStr, src_len);
    if (cmp_status != MZ_OK)
    {
      printf("compress() failed!\n");
      myfree(pCmp);
      myfree(pUncomp);
      return EXIT_FAILURE;
    }

    printf("Compressed from %lu to %lu bytes\n", src_len, cmp_len);

    if (step)
    {
      // Purposely corrupt the compressed data if fuzzy testing (this is a very crude fuzzy test).
      uint32_t n = 1 + (rand() % 3);
      while (n--)
      {
        unsigned long i = rand() % cmp_len;
        pCmp[i] ^= (rand() & 0xFF);
      }
    }

    // Decompress.
    cmp_status = mz_uncompress(pUncomp, &uncomp_len, pCmp, cmp_len);
    total_succeeded += (cmp_status == MZ_OK);

    if (step)
    {
      printf("Simple fuzzy test: step %u total_succeeded: %u\n", step, total_succeeded);
    }
    else
    {
      if (cmp_status != MZ_OK)
      {
        printf("uncompress failed!\n");
        myfree(pCmp);
        myfree(pUncomp);
        return EXIT_FAILURE;
      }

      printf("Decompressed from %lu to %lu bytes\n", cmp_len, uncomp_len);

      // Ensure uncompress() returned the expected data.
      if ((uncomp_len != src_len) || (memcmp(pUncomp, s_pStr, (uint32_t)src_len)))
      {
        printf("Decompression failed!\n");
        myfree(pCmp);
        myfree(pUncomp);
        return EXIT_FAILURE;
      }
    }

    myfree(pCmp);
    myfree(pUncomp);

    step++;

    // Keep on fuzzy testing if there's a non-empty command line.
  } while (argc >= 2);

  printf("Success.\n");
  return EXIT_SUCCESS;
}

