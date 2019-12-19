import struct
from binascii import hexlify, unhexlify

little_endian = lambda x: struct.pack("<I", x)
reg_bytes = lambda x: int.to_bytes(x, 1, "little")


DIV_BY_ZERO_LOAD_REG_IMM32 = b'\xab'
INVALID_HANDLE_HASH_REG2 = b'\x10'
PRIV_INSTRUCTION_CMP_REG_REG = b'\xff'
ILLEGAL_INSTRUCTION_XOR_REG_REG = b'\x00'
NONE_GETCHAR_INTO_REGISTER = b'\xcd'
BREAKPOINT_ROL_REGISTER_IMM8 = b'\x73'

def LOAD_REG_IMM32(reg, imm32):
    return DIV_BY_ZERO_LOAD_REG_IMM32 + reg_bytes(reg) + little_endian(imm32)

def HASH_REG2():
    return INVALID_HANDLE_HASH_REG2

def CMP_REG_REG(reg1, reg2):
    return PRIV_INSTRUCTION_CMP_REG_REG + reg_bytes(reg1) + reg_bytes(reg2)

def XOR_REG_REG(reg1, reg2):
    return ILLEGAL_INSTRUCTION_XOR_REG_REG + reg_bytes(reg1) + reg_bytes(reg2)

def GETCHAR_INTO_REGISTER(reg):
    return NONE_GETCHAR_INTO_REGISTER + reg_bytes(reg)

def ROL_REGISTER(reg, num):
    return BREAKPOINT_ROL_REGISTER_IMM8 + reg_bytes(reg) + reg_bytes(num)

def turnToC(buf):
    carr = "unsigned char opcodes[] = {"
    for b in buf:
        print(hexlify(bytes([b])))
        carr += "0x" + hexlify(bytes([b])).decode() + ","

    carr = carr[:-1]
    carr += "};"
    return carr


def main():
    buf = LOAD_REG_IMM32(1, 0)

    buf += GETCHAR_INTO_REGISTER(0)
    buf += XOR_REG_REG(1, 0)

    buf += GETCHAR_INTO_REGISTER(0)
    buf += ROL_REGISTER(0, 8)
    buf += XOR_REG_REG(1, 0)

    buf += GETCHAR_INTO_REGISTER(0)
    buf += ROL_REGISTER(0, 16)
    buf += XOR_REG_REG(1, 0)

    buf += GETCHAR_INTO_REGISTER(0)
    buf += ROL_REGISTER(0, 24)
    buf += XOR_REG_REG(1, 0)

    # b'KAF{'
    buf += LOAD_REG_IMM32(2, 0x7b46414b)
    buf += CMP_REG_REG(1,2)

    # }
    buf += GETCHAR_INTO_REGISTER(0)
    buf += LOAD_REG_IMM32(1, ord("}"))
    buf += CMP_REG_REG(1,0)
    print (turnToC(buf))


if __name__ == "__main__":
    main()
    