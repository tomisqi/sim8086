#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include "sim8086.h"

static void DumpHex(Buffer buffer);
static void Usage();

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int main(int argc, char** argv)
{
  char* filename = argv[1];

  if (argc != 2) 
  {
    fprintf(stderr, "Invalid number of arguments.\n");
    Usage();
    return 1;
  }

  FILE *file = fopen(filename, "rb");

  Buffer buffer = {0};
  buffer.buf_p = (U8*)malloc(BUF_SIZE);
  buffer.bufSize = fread(buffer.buf_p, 1, BUF_SIZE, file);
  assert(buffer.bufSize <= BUF_SIZE);

  Sim8086(buffer);

  return 0;
}

//-----------------------------------------------------------------------------
// Local Functions
//-----------------------------------------------------------------------------
static void DumpHex(Buffer buffer)
{
  const U8* buf_p = buffer.buf_p;
  for(int i = 0; i < buffer.bufSize; i++)
  {
    if (i % 16 == 0) printf("%08d: ", i);
    printf("%02x", *buf_p);
    buf_p++;
    if (i % 2 == 1) printf(" ");
    if (i % 16 == 15) printf("\n");
  }
  printf("\n");
}

static void Usage()
{
  printf("Usage: sim8086.exe <binaryfile>\n");
}