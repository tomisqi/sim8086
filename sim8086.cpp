#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum OpCodeE
{
  OP_NONE,
  MOV,
};

enum RegisterE
{
    REG_NONE,
    AL,
    AX,
    CL,
    CX,
    DL,
    DX,
    BL,
    BX,
    AH,
    CH,
    DH,
    BH,
    SP,
    BP,
    SI,
    DI,
};

struct Decoded
{
  OpCodeE opCode;
  int dBit;
  int wBit;
  int mod;
  int reg;
  int rm;
};

static OpCodeE GetOpCode(U16 instr);
static Decoded Decode(U16 instr);
static RegisterE GetRegister(int reg, int wBit);
static String32 GetRegisterString(RegisterE reg);
static String32 MOVInstruction(Decoded decoded);

//-----------------------------------------------------------------------------
// Global Functions
//-----------------------------------------------------------------------------
void Dissasemble(Buffer buffer)
{
  const U16* buf_p = (U16*)buffer.buf_p;
  int bufSize16 = buffer.bufSize / 2;

  printf("bits 16\n");

  for(int i = 0; i < bufSize16; i++)
  {
    String32 decodedStr;
    U16 instr = ENDIAN_SHIFT16(*buf_p);

    Decoded decoded = Decode(instr);
    switch (decoded.opCode)
    {
    case MOV:
      decodedStr = MOVInstruction(decoded);
      break;
    case OP_NONE:
    default:
      assert(false);
    break;
    }

    printf("%s\n", decodedStr.buf);
    
    buf_p++;
  }
}

//-----------------------------------------------------------------------------
// Local Functions
//-----------------------------------------------------------------------------

static OpCodeE GetOpCode(U16 instr)
{
  int op = (instr >> 10) & 0x3f;
  OpCodeE opCode = OP_NONE;
  switch (op)
  {
  case 0x22:
    opCode = MOV;
    break;
  default:
    assert(false);
    break;
  }
  return opCode;
}

static Decoded Decode(U16 instr)
{
  Decoded decoded;
  decoded.opCode = GetOpCode(instr);
  decoded.dBit = (instr >> 9) & 0x1;
  decoded.wBit = (instr >> 8) & 0x1;
  decoded.mod = (instr >> 6) & 0x3;
  decoded.reg = (instr >> 3) & 0x7;
  decoded.rm = (instr) & 0x7;
  return decoded;
}

static RegisterE GetRegister(int reg, int wBit)
{
    RegisterE registers[8][2] = { {AL, AX}, {CL, CX}, {DL, DX}, {BL, BX}, {AH, SP}, {CH, BP}, {DH, SI}, {BH, DI} };
    return registers[reg][wBit];
}

static String32 MOVInstruction(Decoded decoded)
{
  if (decoded.mod != 0x3)
  {
    String32 notsupported = {0};
    sprintf(notsupported.buf, "NO_SUPPORT");
    notsupported.len = strlen(notsupported.buf);
    return notsupported;
  }
  
  RegisterE srcReg = GetRegister(decoded.reg, decoded.wBit);
  RegisterE dstReg = GetRegister(decoded.rm, decoded.wBit);
  if (decoded.dBit == 1)
  {
    RegisterE tmp = srcReg;
    srcReg = dstReg;
    dstReg = tmp;
  }

  String32 srcRegStr = GetRegisterString(srcReg);
  String32 dstRegStr = GetRegisterString(dstReg);

  String32 result = {0};
  sprintf(result.buf, "mov %s, %s", dstRegStr.buf, srcRegStr.buf);
  result.len = strlen(result.buf);
  return result;
}

static String32 GetRegisterString(RegisterE reg)
{
  String32 result = {0};
  switch (reg)
  {
  case AL:
    sprintf(result.buf, "al");
    break;
  case AX:
    sprintf(result.buf, "ax");
    break;
  case CL:
    sprintf(result.buf, "cl");
    break;
  case CX:
    sprintf(result.buf, "cx");
    break;
  case DL:
    sprintf(result.buf, "dl");
    break;
  case DX:
    sprintf(result.buf, "dx");
    break;
  case BL:
    sprintf(result.buf, "bl");
    break;
  case BX:
    sprintf(result.buf, "bx");
    break;
  case AH:
    sprintf(result.buf, "ah");
    break;
  case CH:
    sprintf(result.buf, "ch");
    break;
  case DH:
    sprintf(result.buf, "dh");
    break;
  case BH:
    sprintf(result.buf, "bh");
    break;
  case SP:
    sprintf(result.buf, "sp");
    break;
  case BP:
    sprintf(result.buf, "bp");
    break;
  case SI:
    sprintf(result.buf, "si");
    break;
  case DI:
    sprintf(result.buf, "di");
    break;    
  case REG_NONE:
  default:
    assert(false);
    break;    
  }

  result.len = strlen(result.buf);
  return result;
}
