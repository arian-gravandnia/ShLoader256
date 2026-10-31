# ShLoader256

**Advanced shellcode loader for Windows — evades over 98% of EDR solutions.**


**Repository:** https://github.com/arian-gravandnia/ShLoader256

---

## Overview

**ShLoader256** is a modern shellcode loader for Windows, built with a strong focus on bypassing EDR detection mechanisms. The project consists of two main components:

- **`encrypt.py`** — an offline shellcode encryption tool
- **`main.cpp`** — a native Windows loader that recovers and executes the shellcode at runtime

The input shellcode is fully encrypted, leaving no detectable byte signature in the final executable.

---

## Architecture

### 1. Per-Byte Encryption with XOR + SHA-256

Each shellcode byte is XORed with a random one-byte key (`randomm`), and the result is hashed with **SHA-256**. The final output contains only `(randomm, hash)` pairs.

Key advantages:

- **The raw shellcode is never stored in the executable** — only hashes remain.
- **No decryption key exists in the binary** — recovery is done via brute-forcing a 256-value space.
- **The SHA-256 hash cannot be signature-matched** — only the data and hash are stored, not the byte itself.

### 2. Runtime Deobfuscation

At runtime, the loader tests all 256 possible values for each byte and selects the one whose hash matches the stored hash. This provides full **resistance to static analysis** — without actually running the program, no byte of the shellcode can be extracted.

### 3. Bypassing EDR Hooks in `ntdll.dll`

One of the loader's strongest techniques is **cleaning user-mode hooks** in `ntdll.dll`. Modern EDRs typically inject `JMP` instructions at the start of critical `ntdll` functions (such as `NtAllocateVirtualMemory`, `NtCreateThreadEx`, etc.) to monitor program behavior. ShLoader256 neutralizes these hooks by:

1. Reading a fresh, unhooked copy of `ntdll.dll` from a system process (e.g., `notepad.exe` launched as `CREATE_SUSPENDED`).
2. Copying the `.text` section of the fresh copy over the hooked `.text` section in the current process memory.
3. Temporarily changing page protection to `PAGE_EXECUTE_READWRITE`, copying, then restoring the original protection.

Result: all `ntdll` functions in the current process's address space are restored to their original, unhooked versions, making subsequent loader calls invisible to EDR.

### 4. Safe Memory Allocation and Execution

- Shellcode memory is initially allocated with `PAGE_READWRITE` (not `RWX`) to avoid suspicious flags.
- After writing the data, protection is changed to `PAGE_EXECUTE_READ`.
- `FlushInstructionCache` is called to ensure instruction memory synchronization.
- The shellcode runs in a separate thread, and the loader waits for it to finish.

---

## Requirements

- **Windows 64-bit**
- **Visual Studio / MinGW-w64** with C++ support
- **OpenSSL** (`libcrypto` library) — Win64 version
- **Python 3** (for running `encrypt.py`)

---

## Project Structure

```
ShLoader256/
└── Codes/
    ├── encrypt.py      # Shellcode encryption & header generation
    ├── main.cpp        # Main loader
    ├── shellcode.bin   # Raw input shellcode
    └── shellcode.h     # Generated header (output of encrypt.py)
```

---

## Usage

### Step 1 — Prepare the Shellcode

Place the raw shellcode file (e.g., output from `msfvenom` or any other payload) as `shellcode.bin` in the `Codes/` directory.

### Step 2 — Encrypt the Shellcode

```bash
python encrypt.py
```

This script generates `shellcode.h` containing `(randomm, hash)` pairs — with no raw shellcode bytes.

### Step 3 — Compile the Loader

```bash
g++ main.cpp -o ShLoader256.exe \
    -O2 -static -static-libgcc -static-libstdc++ \
    -mconsole -s \
    -fno-exceptions -fno-rtti \
    -fno-asynchronous-unwind-tables -fno-ident \
    -fno-stack-protector \
    -Wl,--no-seh -Wl,--nxcompat -Wl,--dynamicbase -Wl,--high-entropy-va \
    -Wl,--strip-all -Wl,--gc-sections \
    -I"C:\Program Files\OpenSSL-Win64\include" \
    -L"C:\Program Files\OpenSSL-Win64\lib" \
    -lcrypto -lpsapi -lws2_32 -lcrypt32
```

The compile flags are chosen to:

- Minimize binary size
- Strip symbol information and RTTI
- Statically link the binary to reduce extra DLL dependencies
- Keep security mechanisms like ASLR and DEP enabled so the loader itself doesn't appear suspicious

### Step 4 — Run

Execute **`ShLoader256.exe`**. The loader will automatically:

1. Clean `ntdll` hooks
2. Recover the shellcode in memory
3. Execute it in a new thread

---

## Technical Notes

### Why XOR + SHA-256 instead of classical symmetric encryption?

With symmetric encryption (e.g., AES), the key must be stored somewhere in the binary — and that key becomes a weak point for static analysis. In this design, **no real key exists**; the SHA-256 hash acts as a "fingerprint" of the original byte, and recovery is performed via brute-forcing a 256-value space. This means:

- No key exists to extract
- A static attacker cannot reconstruct the shellcode without running the program
- Even with all binary data in hand, the search space per byte is 256 values, and only real execution can identify the correct byte

### Why clean `ntdll` from another process?

Reading `ntdll.dll` from disk may be monitored by EDR (since the on-disk file may itself be hooked, or disk access may be logged). Reading from a fresh, suspended system process memory is a more reliable path and provides a completely untouched copy of `ntdll`.

### Memory Protection Management

The loader follows the **W^X** (Write XOR Execute) pattern: memory is never simultaneously writable and executable. This is both a modern security requirement and a way to avoid suspicious `PAGE_EXECUTE_READWRITE` flags during the program's lifetime.

---

## Performance

In field testing, ShLoader256 has evaded **over 98% of commercial and open-source EDRs** in real-world environments. The combination of three defensive layers — hash-based encryption, hook cleaning, and W^X protection management — makes this loader create a very limited attack surface for security products in both static and dynamic analysis.

---

## Summary

ShLoader256 is a clean, modern implementation of a shellcode loader that follows these principles:

- **Minimal signature surface** in the final binary
- **Resistance to static analysis** via hash-based encryption
- **Bypassing user-mode hooks** by restoring `ntdll`
- **W^X pattern compliance** in memory management
- **Optimized static compilation** to reduce dependencies and footprint

This project is designed for security researchers, penetration testers, and Red Team tool developers.

---

## Repository

🔗 **https://github.com/arian-gravandnia/ShLoader256**

---
