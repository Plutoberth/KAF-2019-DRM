import struct
from binascii import hexlify, unhexlify

little_endian = lambda x: struct.pack("<I", x)
reg_bytes = lambda x: int.to_bytes(x, 1, "little")

DIV_BY_ZERO_LOAD_REG_IMM32 = b'\xab'
INVALID_HANDLE_HASH_REG2 = b'\x10'
PRIV_INSTRUCTION_CMP_REG_REG = b'\xff'
NONE_GETCHAR_INTO_REGISTER = b'\xcd'
BREAKPOINT_ROL_REGISTER_IMM8 = b'\x73'

def LOAD_REG_IMM32(reg, imm32):
    return DIV_BY_ZERO_LOAD_REG_IMM32 + reg_bytes(reg) + little_endian(imm32)

def HASH_REG2():
    return INVALID_HANDLE_HASH_REG2

def CMP_REG_REG(reg1, reg2):
    return PRIV_INSTRUCTION_CMP_REG_REG + reg_bytes(reg1) + reg_bytes(reg2)

import random

def XOR_REG_REG(reg1, reg2):
    ops = list(range(0x100))
    ops.remove(0xab)
    ops.remove(0x10)
    ops.remove(0xff)
    ops.remove(0xcd)
    ops.remove(0x73)
    print(ops)
    return reg_bytes(random.choice(ops)) + reg_bytes(reg1) + reg_bytes(reg2)

def GETCHAR_INTO_REGISTER(reg):
    return NONE_GETCHAR_INTO_REGISTER + reg_bytes(reg)

def ROL_REGISTER(reg, num):
    return BREAKPOINT_ROL_REGISTER_IMM8 + reg_bytes(reg) + reg_bytes(num)

def turnToC(buf):
    carr = "unsigned char opcodes[] = {"
    for b in buf:
        carr += "0x" + hexlify(bytes([b])).decode() + ","

    carr = carr[:-1]
    carr += "};"
    return carr

def LOAD_4_CHARS(tmp, final):
    buf = LOAD_REG_IMM32(final, 0)

    buf += GETCHAR_INTO_REGISTER(tmp)
    buf += XOR_REG_REG(final, tmp)

    buf += GETCHAR_INTO_REGISTER(tmp)
    buf += ROL_REGISTER(tmp, 8)
    buf += XOR_REG_REG(final, tmp)

    buf += GETCHAR_INTO_REGISTER(tmp)
    buf += ROL_REGISTER(tmp, 16)
    buf += XOR_REG_REG(final, tmp)

    buf += GETCHAR_INTO_REGISTER(tmp)
    buf += ROL_REGISTER(tmp, 24)
    buf += XOR_REG_REG(final, tmp)
    return buf

def main_flag():
    
    buf = LOAD_4_CHARS(0, 1)
    # b'KAF{'
    buf += LOAD_REG_IMM32(2, 0x7b46414b)
    buf += CMP_REG_REG(1,2)
    buf += LOAD_REG_IMM32(0, 0xdeadbeef)
    # NoW_
    buf += LOAD_4_CHARS(1,2) 
    buf += XOR_REG_REG(0, 2)
    # Correct result
    buf += LOAD_REG_IMM32(1, 0x81fad1a1)
    buf += CMP_REG_REG(0,1)

    # T
    # Initial hash of T: 0x50007df0
    # Rol 16: 0x7df05000
    buf += GETCHAR_INTO_REGISTER(2)
    buf += HASH_REG2()
    buf += ROL_REGISTER(2, 16)
    buf += LOAD_REG_IMM32(1, 0x7df05000)
    buf += CMP_REG_REG(1,2)
    # rY_2
    # chars in reg 0 now
    buf += LOAD_4_CHARS(1, 0)
    buf += XOR_REG_REG(0, 2)
    # xor with prev result, now 4faf0972
    # xor with last char in register, should be 2, now 7daf0972
    
    buf += LOAD_REG_IMM32(2, 0x7daf0972)
    buf += XOR_REG_REG(0, 1)
    buf += CMP_REG_REG(2, 0)

    #_R3V
    buf += LOAD_4_CHARS(0,1)
    buf += LOAD_REG_IMM32(2, 0x92fab19a)
    #92fab19a now
    buf += ROL_REGISTER(1, 19)
    buf += CMP_REG_REG(1,2)
    
    #3RSe
    buf += LOAD_4_CHARS(1,0)
    buf += LOAD_REG_IMM32(2, 0x83)
    buf += HASH_REG2()
    buf += XOR_REG_REG(0, 2)

    buf += LOAD_REG_IMM32(1, 0xe3ac13b6)
    buf += CMP_REG_REG(0, 1)

    #_Scr

    buf += LOAD_4_CHARS(2, 1)
    buf += LOAD_REG_IMM32(2, 0x91cf40e9)
    buf += XOR_REG_REG(0, 1)
    buf += CMP_REG_REG(0, 2)

    #ATcH

    buf += LOAD_4_CHARS(1,0)
    buf += LOAD_REG_IMM32(1, 0x90c6a882)
    buf += ROL_REGISTER(0, 1)
    buf += CMP_REG_REG(0, 1)


    # }
    buf += GETCHAR_INTO_REGISTER(0)
    buf += LOAD_REG_IMM32(1, ord("}"))
    buf += CMP_REG_REG(1,0)
    print (turnToC(buf))
    return buf

def fake_flag():
    buf = LOAD_4_CHARS(0,2)
    buf += LOAD_REG_IMM32(1, 0x7b4b4146)
    buf += CMP_REG_REG(1,2)
    buf += LOAD_4_CHARS(0,2)
    buf += LOAD_REG_IMM32(1, 0x316e5f65)
    buf += CMP_REG_REG(1,2)
    buf += LOAD_4_CHARS(0,2)
    buf += LOAD_REG_IMM32(1, 0x595f3343)
    buf += CMP_REG_REG(1,2)
    buf += LOAD_4_CHARS(0,2)
    buf += LOAD_REG_IMM32(1, 0x415f556f)
    buf += CMP_REG_REG(1,2)
    buf += LOAD_4_CHARS(0,2)
    buf += LOAD_REG_IMM32(1, 0x545f3372)
    buf += CMP_REG_REG(1,2)
    buf += LOAD_4_CHARS(0,2)
    buf += LOAD_REG_IMM32(1, 0x5f337572)
    buf += CMP_REG_REG(1,2)
    buf += LOAD_4_CHARS(0,2)
    buf += LOAD_REG_IMM32(1, 0x30786148)
    buf += CMP_REG_REG(1,2)

    buf += LOAD_REG_IMM32(1, 0)

    buf += GETCHAR_INTO_REGISTER(2)
    buf += XOR_REG_REG(1, 2)

    buf += GETCHAR_INTO_REGISTER(2)
    buf += ROL_REGISTER(2, 8)
    buf += XOR_REG_REG(1,2)
    
    buf += LOAD_REG_IMM32(2, 0x7d72)
    buf += CMP_REG_REG(1,2)
    buf += b'\x10' * (len(main_flag()) - len(buf))
    print (turnToC(buf))

if __name__ == "__main__":
    main_flag()
    