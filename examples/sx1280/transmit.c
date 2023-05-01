#include <stdio.h>
#include <stdlib.h>
#include "contiki.h"
#include "dev/serial-line.h"
#include "dev/uart.h"
#include "dev/spi.h"
#include "netstack.h"
#include "process.h"
#include "rtimer-arch.h"
#include "sys/_stdint.h"
#include "sys/log.h"
#include "sx128x.h"
#include "antenna-sw.h"
#include "shell.h"
#include "shell-commands.h"
/*---------------------------------------------------------------------------*/
#define LOG_MODULE "MAIN"
#define LOG_LEVEL LOG_LEVEL_NONE
#define TIMEOUTVALUE (20) 
/*---------------------------------------------------------------------------*/
static struct etimer tx_timer;
char buf[255];
/*---------------------------------------------------------------------------*/
PROCESS(tx_process, "tx_process");
AUTOSTART_PROCESSES(&tx_process);

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(tx_process, ev, data)
{
  PROCESS_BEGIN();

  etimer_set(&tx_timer,TIMEOUTVALUE);

    while(1){
        PROCESS_WAIT_EVENT();

        if(ev == PROCESS_EVENT_TIMER){
            NETSTACK_RADIO.send("helloworld", 10);
            etimer_reset(&tx_timer);
        }
        
    }
    
  PROCESS_END();
  
}