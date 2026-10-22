.text
.globl _probe_action_word
.p2align 2
_probe_action_word:
 stp x21, x22, [sp, #-64]!
 stp x29, x30, [sp, #16]
 stp x19, x20, [sp, #32]
 mov x19, x2
 mov w21, w1
 mov w22, w1
 mov x16, #0x1234
 mov x17, #0x5678
 cmp x1, x1
 mrs x20, nzcv
 blr x0
 stp x21, x22, [x19]
 stp x16, x17, [x19, #16]
 mrs x8, nzcv
 str x8, [x19, #32]
 ldp x19, x20, [sp, #32]
 ldp x29, x30, [sp, #16]
 ldp x21, x22, [sp], #64
 ret
