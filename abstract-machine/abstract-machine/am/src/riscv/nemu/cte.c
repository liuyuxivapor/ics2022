#include <am.h>
#include <riscv/riscv.h>
#include <klib.h>

static Context* (*user_handler)(Event, Context*) = NULL;

void __am_get_cur_as(Context* c);
void __am_switch(Context* c);
void get_mscratch();

Context* __am_irq_handle(Context *c) {
  __am_get_cur_as(c);
  if (user_handler) {
    Event ev = {0};
    switch (c->mcause) {
      case EVENT_YIELD: ev.event = EVENT_YIELD; break;
      case EVENT_SYSCALL: ev.event = EVENT_SYSCALL; break;
      case EVENT_PAGEFAULT: ev.event = EVENT_PAGEFAULT; break;
      default: ev.event = EVENT_ERROR; break;
    }
    c = user_handler(ev, c);
    assert(c != NULL);
  }
  __am_switch(c);
  return c;
}

extern void __am_asm_trap(void);

bool cte_init(Context*(*handler)(Event, Context*)) {
  // initialize exception entry
  // 用0号寄存器来存异常入口地址，后续会自动归0
  asm volatile("csrw mtvec, %0" : : "r"(__am_asm_trap));

  // register event handler
  user_handler = handler;

  return true;
}

Context *kcontext(Area kstack, void (*entry)(void *), void *arg) {
  Context* c = (Context*)kstack.end - sizeof(Context);
  c->mepc = (uintptr_t)entry;
  c->mstatus = 0x1880;
  c->mcause = 0xb;
  c->GPR2 = (uintptr_t)arg;
  c->pdir = NULL;
  c->np = 0;
  c->gpr[2] = (uintptr_t)c;
  printf("addr of hello: %p\n", c->mepc);
  return c;
}

void yield() {
  asm volatile("li a7, -1; ecall");
}

bool ienabled() {
  return false;
}

void iset(bool enable) {
}
