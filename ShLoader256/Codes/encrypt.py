import random
import hashlib

result=[]

def encryption(byte):
    randomm = random.randint(0, 255)
    xored = byte ^ randomm
    hashed = hashlib.sha256(bytes([xored]))
    hashedHex = hashed.hexdigest()

    result.append((randomm, hashedHex))




def read_shellcode():
    with open("Codes/shellcode.bin", "rb") as f:
        for byte in f.read():
    
            encryption(byte)
    
    append_File()
    


def append_File():

    with open("Codes/shellcode.h","w") as h:
        h.write("#pragma once\n\n")
        h.write("typedef struct {\n")
        h.write("\tunsigned char randomm;\n")
        h.write("\tconst char* hash;\n")
        h.write("} EncryptedByte;\n\n")
        h.write("static const EncryptedByte shellcode[] = {\n")

        for randomm, hashedHex in result:
            h.write(f'\t{{ {randomm},"{hashedHex}"}},\n')

        h.write("};\n")
    h.close()

read_shellcode()
















































    # for i in range(256):
    #     candidate = i ^ randomm
    #     x = hashlib.sha256(bytes([candidate]))
    #     if x.hexdigest() == hashedHex:
    #         print("real byte finded : ", i)  
    #         break
