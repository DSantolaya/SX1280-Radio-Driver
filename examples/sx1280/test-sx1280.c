/*
 * Copyright (c) 2020, Perale Thomas
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 */
/**
 * \addtogroup zoul-examples
 * @{
 *
 * \defgroup zoul-sx1280-test RN2483 Wireless Module sensor.
 *
 * @{
 *
 * \file
 *         A quick program for testing the SX1280 Wireless Module sensor.
 * \author
 *         Perale Thomas <tperale@vub.be>
 */
/*---------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include "contiki.h"
#include "dev/serial-line.h"
#include "dev/uart.h"
#include "dev/spi.h"
#include "netstack.h"
#include "process.h"
#include "rtimer-arch.h"
#include "shell.h"
#include "shell-commands.h"
#include "sys/_stdint.h"
#include "sys/log.h"
#include "sx128x.h"
#include "antenna-sw.h"
#include "sys/energest.h"
/*---------------------------------------------------------------------------*/
#define LOG_MODULE "MAIN"
#define LOG_LEVEL LOG_LEVEL_DBG
#define TIMEOUTVALUE (5 * 2000) 

/*---------------------------------------------------------------------------*/

/* ENERGEST from ticks to ms*/
static unsigned int to_ms(uint64_t time_energy)
{
  return (unsigned int)(time_energy*1000/(ENERGEST_SECOND));
}

/*SHELL_RECV Turn radio in Rx mode*/
static PT_THREAD(shell_recv(struct pt *pt, shell_output_func output, char *args)) {
  PT_BEGIN(pt);
  char buf[255];
  int timeout = 0;

  NETSTACK_RADIO.on();

  while (!NETSTACK_RADIO.receiving_packet() && !NETSTACK_RADIO.pending_packet() && timeout < TIMEOUTVALUE) {
    watchdog_periodic();      
    clock_delay_usec(500);    
    timeout++;
  }
  
  if (!NETSTACK_RADIO.pending_packet()) {
    SHELL_OUTPUT(output, "Waiting for pending \n");
    while (!NETSTACK_RADIO.pending_packet() && timeout < TIMEOUTVALUE) {
      watchdog_periodic();
      clock_delay_usec(500);
      timeout++;
    }
  }

   if (timeout >= TIMEOUTVALUE) {
    NETSTACK_RADIO.off();
    SHELL_OUTPUT(output, "Recv timed out after 10sec\n");
    PT_EXIT(pt);
   }

  //Packet lecture
  int len = NETSTACK_RADIO.read((void *)buf, 255);
  NETSTACK_RADIO.off();
  SHELL_OUTPUT(output, "Received (%d bytes): '%s'\n", len, buf);
  PT_END(pt);

}
/* SHELL_RECV */

static PT_THREAD(shell_send(struct pt *pt, shell_output_func output, char *args))
{
  char *next_args;

  PT_BEGIN(pt);

  SHELL_ARGS_INIT(args, next_args);

  SHELL_ARGS_NEXT(args, next_args);
  if (args == NULL) {
    SHELL_OUTPUT(output, "Sending 'helloworld'\n");
    SHELL_OUTPUT(output, "Working\n");
    
    NETSTACK_RADIO.send("helloworld", 10);
  } else {
    NETSTACK_RADIO.send(args, strlen(args));
  }

  PT_END(pt);
}
/* SHELL_SEND */

static PT_THREAD(shell_toa(struct pt *pt, shell_output_func output, char *args))
{
  char *next_args;

  PT_BEGIN(pt);

  SHELL_ARGS_INIT(args, next_args);

  SHELL_ARGS_NEXT(args, next_args);
  if (args == NULL) {
    SHELL_OUTPUT(output, "Should specify length\n");
    PT_EXIT(pt);
  } 

  SHELL_OUTPUT(output, "%ld\n", RTIMERTICKS_TO_US_64(TSCH_PACKET_DURATION(atoi(args))));

  PT_END(pt);
}
/* SHELL_TOA */

/* SHELL_ENERGY*/
static PT_THREAD(shell_energy(struct pt *pt, shell_output_func output, char *args))
{
  PT_BEGIN(pt);
  energest_flush();   /*updates times*/

  SHELL_OUTPUT(output,"\n***********ENERGY CONSUMPTION**********\n")
  SHELL_OUTPUT(output, " LPM %1llu ticks RX %1llu ticks TX %1llu ticks \n", 
              energest_type_time(ENERGEST_TYPE_LPM),
              energest_type_time(ENERGEST_TYPE_LISTEN),
              energest_type_time(ENERGEST_TYPE_TRANSMIT)
              );  
  SHELL_OUTPUT(output, " LPM %3ums RX %1ums TX %1ums \n\n",  
              to_ms(energest_type_time(ENERGEST_TYPE_LPM)),
              to_ms(energest_type_time(ENERGEST_TYPE_LISTEN)),
              to_ms(energest_type_time(ENERGEST_TYPE_TRANSMIT))             
               );              
 
  PT_END(pt);
}
/* SHELL_ENERGY*/

const struct shell_command_t custom_shell_commands[] = {
  { "send", shell_send, "'> send': Send a basic 'helloworld' message using LoRa Radio." },
  { "recv", shell_recv, "'> recv': Busywait the next message." },
  { "toa", shell_toa, "'> toa <len> : ." },
  { "energy", shell_energy, "'> energy' : Energy consumption measurement "},
  { NULL, NULL, NULL},
};

static struct shell_command_set_t custom_shell_command_set = {
  .next = NULL,
  .commands = custom_shell_commands,
};

/*---------------------------------------------------------------------------*/
PROCESS(node_process, "Shell");
AUTOSTART_PROCESSES(&node_process);

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(node_process, ev, data)
{
  PROCESS_BEGIN();

  shell_command_set_register(&custom_shell_command_set);

  while(1) {
    PROCESS_WAIT_EVENT();
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */