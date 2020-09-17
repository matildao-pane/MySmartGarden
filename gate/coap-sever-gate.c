#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "contiki.h"
#include "coap-engine.h"
#include "coap-blocking-api.h"
#include "os/dev/leds.h"

/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_DBG

extern process_event_t OPENED_GATE_EVENT;
extern process_event_t CLOSED_GATE_EVENT;
process_event_t PHOTOCELL_EVENT;
extern bool photocell_value;
extern bool status;
extern coap_resource_t  res_gate; //RESOURCE DEFINITION 
extern coap_resource_t  res_photocell;

#define SERVER_EP ("coap://[fd00::1]:5683")
char *service_registration = "/registration";

bool registered = false; 

PROCESS(photocell_process,"close the gate after 10 seconds");

PROCESS(gate_node, "Gate Node");
AUTOSTART_PROCESSES(&gate_node);

/* This function will be passed to COAP_BLOCKING_REQUEST() to handle responses. */
void
client_chunk_handler(coap_message_t *response)
{
  const uint8_t *chunk;

  if(response == NULL) {
    LOG_INFO("Request timed out");
    return;
  }

  if(!registered)
	registered = true;

  int len = coap_get_payload(response, &chunk);
 
  LOG_INFO("|%.*s \n", len, (char *)chunk);
}


PROCESS_THREAD(gate_node, ev, data)
{

  static coap_endpoint_t server_ep;
  static coap_message_t request[1];  

  static struct etimer timer;

  PROCESS_BEGIN();

	LOG_INFO("Starting gate node\n");

	//activate the resource
	coap_activate_resource(&res_gate, "gate-actuator");
	coap_activate_resource(&res_photocell, "photocell-sensor");
	//populate endpoint datastructure
	coap_endpoint_parse(SERVER_EP, strlen(SERVER_EP), &server_ep);


	/* prepare request, TID is set by COAP_BLOCKING_REQUEST() */
	LOG_INFO("Registering\n");
	coap_init_message(request, COAP_TYPE_CON, COAP_GET, 0);
	coap_set_header_uri_path(request, service_registration);

	while(!registered){	
		COAP_BLOCKING_REQUEST(&server_ep, request, client_chunk_handler);
        }                 
	LOG_DBG("Registered\n");

	
	while(true){
		LOG_DBG("status%d\n",status );
		PROCESS_WAIT_EVENT_UNTIL(ev==OPENED_GATE_EVENT||ev==PROCESS_EVENT_TIMER  ||ev==CLOSED_GATE_EVENT || ev==PHOTOCELL_EVENT);  //if the gate has been opened
		if(ev == OPENED_GATE_EVENT  && status){		
			LOG_DBG("Gate opened manually%d\n",status );
			etimer_set(&timer, 10*CLOCK_SECOND); //start 10 seconds timer
			process_start(&photocell_process,&timer); //start photocell process  

		}else if(ev == PHOTOCELL_EVENT && status){
			LOG_DBG("Activated photocell\n");
			etimer_reset(&timer); //restart 10 seconds timer
		}
		
		else if( (ev == PROCESS_EVENT_TIMER || ev == CLOSED_GATE_EVENT) && status ){ //after 10 seconds, if the gate is still open,it will be closed
			
			status=false;
			leds_single_off(LEDS_GREEN);
			res_gate.trigger(); //notify the clients that the gate is now closed
			LOG_DBG("Closing gate automatically\n");
		}
	}


  PROCESS_END();
}


PROCESS_THREAD(photocell_process, ev, data){
	  static struct etimer etimer;

	  PROCESS_BEGIN();

		//LOG_INFO("process\n");
		
		etimer_set(&etimer, 5*CLOCK_SECOND);
		PROCESS_WAIT_EVENT_UNTIL(ev==PROCESS_EVENT_TIMER);  
		if(status){
			photocell_value = rand()%2;
			LOG_DBG("Photocell value: %d\n", photocell_value);
			if(photocell_value){
				res_photocell.trigger(); ////////////////
				process_post(&gate_node, OPENED_GATE_EVENT, data); 
			}
		}

	  PROCESS_END();
}

