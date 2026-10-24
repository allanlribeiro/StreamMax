#ifndef SM_WATCHNEXT_H
#define SM_WATCHNEXT_H
#ifdef NV_ANDROID
void watchnext_sujar(void);
void watchnext_atualizar(unsigned agora, int pronto);
#else
static inline void watchnext_sujar(void) {}
static inline void watchnext_atualizar(unsigned agora, int pronto) { (void)agora; (void)pronto; }
#endif
#endif
