#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum OpCodeE
{
  OP_NONE,
  MOV,
  MOVIMM,
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

struct DecodingOutput
{
  String32 decoded;
  int bytesConsumed;
};

struct MOVFields
{
  int dBit;
  int wBit;
  int mod;
  int reg;
  int rm;
};

struct MOVIMMFields
{
  int wBit;
  int reg;
};

struct EACalculation
{
  RegisterE operand1;
  RegisterE operand2;
  int displacement;
};

static OpCodeE GetOpCode(U8 byte);
static MOVFields MOVDecode(U8* buf_p);
static RegisterE GetRegister(int reg, int wBit);
static String32 GetRegisterString(RegisterE reg);
static DecodingOutput MOV_InstructionDecoding(const U8* buf_p);
static DecodingOutput MOVIMM_InstructionDecoding(const U8* buf_p);

//-----------------------------------------------------------------------------
// Global Functions
//-----------------------------------------------------------------------------
void Sim8086(Buffer buffer)
{
  const U8* buf_p = buffer.buf_p;

  printf("bits 16\n");

  while(BYTE_OFFSET(buf_p, buffer.buf_p) < buffer.bufSize)
  {
    DecodingOutput decodingOutput;

    OpCodeE opCode = GetOpCode(*buf_p);
    switch (opCode)
    {
    case MOV:
      decodingOutput = MOV_InstructionDecoding(buf_p);
      break;
    case MOVIMM:
      decodingOutput = MOVIMM_InstructionDecoding(buf_p);
      break;
    case OP_NONE:
    default:
      assert(false);
    break;
    }

    printf("%s\n", decodingOutput.decoded.buf);
    
    buf_p += decodingOutput.bytesConsumed;
  }
}

//-----------------------------------------------------------------------------
// Local Functions
//-----------------------------------------------------------------------------
static OpCodeE GetOpCode(U8 byte)
{
  if (((byte >> 4) & 0xf) == 0xb)
  {
    return MOVIMM;
  }
  if (((byte >> 2) & 0x3f) == 0x22)
  {
    return MOV;
  }
  assert(false);
  return OP_NONE;
}

static MOVFields MOV_GetFields(const U8* buf_p)
{
  U8 byte1 = buf_p[0];
  U8 byte2 = buf_p[1];
  
  MOVFields decoded;
  decoded.dBit = (byte1 >> 1) & 0x1;
  decoded.wBit = (byte1     ) & 0x1;
  decoded.mod =  (byte2 >> 6) & 0x3;
  decoded.reg =  (byte2 >> 3) & 0x7;
  decoded.rm =   (byte2     ) & 0x7;
  return decoded;
}

static MOVIMMFields MOVIMM_GetFields(const U8* buf_p)
{
  U8 byte1 = buf_p[0];
  MOVIMMFields decoded;

  decoded.wBit = (byte1 >> 3) & 0x1;
  decoded.reg =  (byte1     ) & 0x7;

  return decoded;
}

static void GetEACalculationRegisters(int rm, RegisterE* reg1_p, RegisterE* reg2_p)
{
  RegisterE RmEAEncoding[8][2] = {{BX, SI}, {BX, DI}, {BP, SI}, {BP, DI}, {SI, REG_NONE}, {DI, REG_NONE}, {BP, REG_NONE}, {BX, REG_NONE}};
  *reg1_p = RmEAEncoding[rm][0];
  *reg2_p = RmEAEncoding[rm][1];
}


static RegisterE GetRegister(int reg, int wBit)
{
    RegisterE registers[8][2] = { {AL, AX}, {CL, CX}, {DL, DX}, {BL, BX}, {AH, SP}, {CH, BP}, {DH, SI}, {BH, DI} };
    return registers[reg][wBit];
}

static DecodingOutput MOV_InstructionDecoding(const U8* buf_p)
{
  MOVFields movFields = MOV_GetFields(buf_p);
  
  String32 srcStr = {0};
  String32 dstStr = {0};
  int consumedBytes = 2; // At least 2 bytes are consumed by MOV.
  if (movFields.mod == 0x3) // Register Mode.
  {
    RegisterE srcReg = GetRegister(movFields.reg, movFields.wBit);
    RegisterE dstReg = GetRegister(movFields.rm, movFields.wBit);
    srcStr = GetRegisterString(srcReg);
    dstStr = GetRegisterString(dstReg);
  }
  else // Memory Mode.
  {
    RegisterE srcReg = GetRegister(movFields.reg, movFields.wBit);
    srcStr = GetRegisterString(srcReg);

    RegisterE operand1; RegisterE operand2;
    GetEACalculationRegisters(movFields.rm, &operand1, &operand2);
    bool directAddress = (movFields.mod == 0x0 && movFields.rm == 0x6);

    U32 displacement = 0;
    if (movFields.mod == 0x1)
    {
      displacement = buf_p[2];
      consumedBytes += 1;
    }
    else if (movFields.mod == 0x2 || directAddress)
    {
      displacement = buf_p[2] | buf_p[3] << 8;
      consumedBytes += 2;
    }

    String32 operand1Str = GetRegisterString(operand1);
    String32 operand2Str = GetRegisterString(operand2);
    if (directAddress)
    {
      sprintf(operand1Str.buf, "%d", displacement);
    }
    
    sprintf(dstStr.buf, "[%s", operand1Str.buf);
    if (operand2 != REG_NONE)
    {
      char buf[32] = {0};
      sprintf(buf, "+ %s", operand2Str.buf);
      strcat(dstStr.buf, buf);
    }
    if (displacement)
    {
      char buf[32] = {0};
      sprintf(buf, "+ %d", displacement);
      strcat(dstStr.buf, buf);
    }
    strcat(dstStr.buf, "]");
  }
    
  if (movFields.dBit == 1) // Swap Src<->Dst.
  {
    String32 tmp = srcStr;
    srcStr = dstStr;
    dstStr = tmp;
  }
  
  DecodingOutput decodingOutput = {0};
  decodingOutput.bytesConsumed = consumedBytes;
  sprintf(decodingOutput.decoded.buf, "mov %s, %s", dstStr.buf, srcStr.buf);
  decodingOutput.decoded.len = strlen(decodingOutput.decoded.buf);
  return decodingOutput;
}

static DecodingOutput MOVIMM_InstructionDecoding(const U8* buf_p)
{
  MOVIMMFields movImmFields = MOVIMM_GetFields(buf_p);

  String32 srcStr;
  String32 dstStr;
  int consumedBytes = 2; // At least 2 bytes are consumed by MOVIMM.
  int data = buf_p[1];
  if (movImmFields.wBit == 1)
  {
    data = buf_p[1] | buf_p[2] << 8;
    consumedBytes += 1;
  }
  sprintf(srcStr.buf, "%d", data);

  RegisterE dstReg = GetRegister(movImmFields.reg, movImmFields.wBit);
  dstStr = GetRegisterString(dstReg);
  
  DecodingOutput decodingOutput = {0};
  decodingOutput.bytesConsumed = consumedBytes;
  sprintf(decodingOutput.decoded.buf, "mov %s, %s", dstStr.buf, srcStr.buf);
  decodingOutput.decoded.len = strlen(decodingOutput.decoded.buf);
  return decodingOutput;
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
    sprintf(result.buf, "");
    break;
  default:
    assert(false);
    break;    
  }

  result.len = strlen(result.buf);
  return result;
}
