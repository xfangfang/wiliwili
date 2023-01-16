.section .text.nroEntrypointTrampoline, "ax", %progbits
.align 2
.global nroEntrypointTrampoline
.type   nroEntrypointTrampoline, %function
.cfi_startproc
nroEntrypointTrampoline:

    // Reset stack pointer.
    adrp x8, __stack_top //Defined in libnx.
    ldr  x8, [x8, #:lo12:__stack_top]
    mov  sp, x8

    // Call NRO.
    blr  x2

    // Save retval
    adrp x1, g_lastRet
    str  w0, [x1, #:lo12:g_lastRet]

    // Reset stack pointer and load next NRO.
    adrp x8, __stack_top
    ldr  x8, [x8, #:lo12:__stack_top]
    mov  sp, x8

    b    loadNro

.cfi_endproc

.section .text.__libnx_exception_entry, "ax", %progbits
.align 2
.global __libnx_exception_entry
.type   __libnx_exception_entry, %function
.cfi_startproc
__libnx_exception_entry:

    // Divert execution to the NRO entrypoint (if a NRO is actually loaded).
    adrp x7, g_nroAddr
    ldr  x7, [x7, #:lo12:g_nroAddr]
    cbz  x7, .Lfail
    br   x7

.Lfail:
    // Otherwise, pass this unhandled exception right back to the kernel.
    mov w0, #0xf801 // KERNELRESULT(UnhandledUserInterrupt)
    svc 0x28        // svcReturnFromException

.cfi_endproc
