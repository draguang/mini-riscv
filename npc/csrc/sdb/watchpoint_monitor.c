#include "common.h"
#ifdef CONFIG_WATCHPOINT
    void wp_monitor()
    {
        WP *wp = head;
        bool success = true;
        while(wp!=NULL)
        {
            uint32_t new_wp = expr(wp->expr,&success);
            if(success &&(new_wp!=wp->value))
            {
                
                printf("Watchpoint %d: %s changed from 0x%x to 0x%x\n", 
                wp->NO, wp->expr, wp->value, new_wp);
                wp->value = new_wp;
                npc_state.state = NPC_STOP;
                continue;
            }
            wp = wp->next;
        }
    }
#endif