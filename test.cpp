#include <stdio.h>
#include <string.h>
#include <openssl/evp.h>

// OpenSSL engine implementation
#define OPENSSL_ENGINE NULL

/**
 * Returns the SHA256 value of the input string
 *
 * @param string input string for which the hash to be calculated
 * @returns string (32 bytes) - SHA256 hash
 */
static const unsigned char *getShaSum(const unsigned char *string)
{
  EVP_MD_CTX *mdCtx = EVP_MD_CTX_new();
  unsigned char mdVal[EVP_MAX_MD_SIZE], *md;
  unsigned int mdLen, i;

  if (!EVP_DigestInit_ex(mdCtx, EVP_sha256(), OPENSSL_ENGINE))
  {
    printf("Message digest initialization failed.\n");
    EVP_MD_CTX_free(mdCtx);
    exit(EXIT_FAILURE);
  }

  // Hashes cnt bytes of data at d into the digest context mdCtx
  if (!EVP_DigestUpdate(mdCtx, string, strlen((const char *)string)))
  {
    printf("Message digest update failed.\n");
    EVP_MD_CTX_free(mdCtx);
    exit(EXIT_FAILURE);
  }

  if (!EVP_DigestFinal_ex(mdCtx, mdVal, &mdLen))
  {
    printf("Message digest finalization failed.\n");
    EVP_MD_CTX_free(mdCtx);
    exit(EXIT_FAILURE);
  }
  EVP_MD_CTX_free(mdCtx);

  printf("DEBUG: Digest is: ");
  for (i = 0; i < mdLen; i++)
    printf("%02x", mdVal[i]);
  printf("\n");

  md = mdVal;

  return md;
}

int main()
{
  // To calculate the hash of a file, read it and pass the pointer
  getShaSum("Hello world");

  exit(EXIT_SUCCESS);
}
