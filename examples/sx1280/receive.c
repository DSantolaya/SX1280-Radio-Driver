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
#define LOG_LEVEL LOG_LEVEL_DBG
#define TIMEOUTVALUE (5*2000) 
#define TIMEOUTRX (400) 
/*---------------------------------------------------------------------------*/
static struct etimer rx_timer;
char buf[255];
int timeout=0;
/*---------------------------------------------------------------------------*/
PROCESS(rx_process, "rx_process");
AUTOSTART_PROCESSES(&rx_process);

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(rx_process, ev, data)
{
  PROCESS_BEGIN();
    timeout = 0;
    NETSTACK_RADIO.on();
    etimer_set(&rx_timer,TIMEOUTRX);
    while(1){
        PROCESS_WAIT_EVENT();
        if(ev==PROCESS_EVENT_TIMER){
            while (!NETSTACK_RADIO.receiving_packet() && !NETSTACK_RADIO.pending_packet() && timeout < TIMEOUTVALUE) {
             watchdog_periodic();      
                clock_delay_usec(500);    
                timeout++;
                }
  
            if (!NETSTACK_RADIO.pending_packet()) {
                while (!NETSTACK_RADIO.pending_packet() && timeout < TIMEOUTVALUE) {
                 watchdog_periodic();
                 clock_delay_usec(500);
                 timeout++;
                  }
                }

            if (timeout >= TIMEOUTVALUE) {
                NETSTACK_RADIO.off();
                }

            //Packet lecture
            int len = NETSTACK_RADIO.read((void *)buf, 255);
            NETSTACK_RADIO.off();
            LOG_DBG("Received (%d bytes): '%s'\n", len, buf);

            etimer_reset(&rx_timer); 
            
        }
    }
        PROCESS_END();
}
       