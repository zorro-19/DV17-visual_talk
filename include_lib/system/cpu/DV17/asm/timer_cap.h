#ifndef  __TIMER_CA__H__
#define  __TIMER_CA__H__

#include "typedef.h"
#include "asm/pwm.h"


struct timer_reg {
    volatile u32 con;
    volatile u32 cnt;
    volatile u32 prd;
    volatile u32 cmp;
};

struct irflt {
    u8 en;
};

struct cap_parm {
    u8 timerx;
    u8 timerx_en;
    u8 irq_priority;
    int gpio;
    PORT_EDGE_E edge;

    u8 irq_int;
    struct irflt irflt_parm;
    struct timer_reg *reg;
    void (*handler)(void *priv);
    void *priv;
};

struct timer_cap_work {
    struct cap_parm *cparm;
    void (*handler)(void *priv, void *prd);
    void *priv;
} ;


void *timerx_cap_reg(struct cap_parm *cparm, void (*handler)(void *priv, void *), void *priv);
void timerx_cam_unreg(void *hdl);
#endif  /*PORT_WAKED_UP_H*/
