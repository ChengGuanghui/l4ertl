.text
/******************/
/* Context switch */
/******************/
/*
extern void context_switch (pthread_t new_thread, pthread_t *current_thread);
                                         r0                     r1
*/
.global context_switch
context_switch:
        stmdb sp!, {r0 - r10, fp, lr}
        str ip, [sp, #-4]!
        ldr r2, [r1]
        str sp, [r2]
        ldr sp, [r0]
        str r0, [r1]                @ set current task to new task
        ldr ip, [sp], #4
        ldmia sp!, {r0 - r10, fp, pc}^
