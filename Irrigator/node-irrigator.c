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
extern process_event_t POST_EVENT;

extern coap_resource_t  res_light; //RESOURCE DEFINITION 
extern coap_resource_t  res_irrigator;
extern coap_resource_t  res_humidity;

#define SERVER_EP ("coap://[fd00::1]:5683")
char *service_registration = "/registration";

bool registered = false; 
static struct etimer timer;
#define THRESHOLD 40
extern bool is_sunset;
extern bool is_humid;
extern bool status;
extern bool is_auto;
extern int humidity_value;
extern int light_value;
PROCESS(irrigator_node, "Irrigator Node");
AUTOSTART_PROCESSES(&irrigator_node);

/* This function will be passed to COAP_BLOCKING_REQUEST() to handle responses. */
void client_chunk_handler(coap_message_t *response) {
  
	const uint8_t *chunk;

	if(response == NULL) {
		puts("Request timed out");
	return;
	}

	if(!registered)
	registered = true;

	int len = coap_get_payload(response, &chunk);
	printf("|%.*s", len, (char *)chunk);
}


PROCESS_THREAD(irrigator_node, ev, data)
{

  static coap_endpoint_t server_ep;
  static coap_message_t request[1];    

  PROCESS_BEGIN();

	LOG_INFO("Starting irrigation node\n");
  
	//activate the resource
	coap_activate_resource(&res_light, "light-sensor");
	coap_activate_resource(&res_irrigator, "irrigator-actuator"); 
	coap_activate_resource(&res_humidity, "humidity-sensor"); 

	//populate endpoint datastructure
	coap_endpoint_parse(SERVER_EP, strlen(SERVER_EP), &server_ep);

	/* prepare request, TID is set by COAP_BLOCKING_REQUEST() */
	LOG_INFO("registering\n");
	coap_init_message(request, COAP_TYPE_CON, COAP_GET, 0);
	coap_set_header_uri_path(request, service_registration);

	while(!registered){	
		COAP_BLOCKING_REQUEST(&server_ep, request, client_chunk_handler);	
        }                 
	LOG_INFO("registered\n");
	etimer_set(&timer, CLOCK_SECOND*10);

	while(true){
		
		PROCESS_WAIT_EVENT();
		if(ev == PROCESS_EVENT_TIMER){  ///  

			humidity_value = rand()% 100 + 1;
			LOG_DBG("humidity: %d\n", humidity_value);
			if(humidity_value <= THRESHOLD){
				is_humid = false;
				leds_on(LEDS_NUM_TO_MASK(LEDS_RED));
			}
			if(humidity_value > THRESHOLD){
				is_humid = true;
				leds_off(LEDS_NUM_TO_MASK(LEDS_RED));
			}

			light_value = rand()% 100 + 1;
			LOG_DBG("light: %d\n", light_value);
			if(light_value <= THRESHOLD){
				is_sunset=true;
				leds_on(LEDS_NUM_TO_MASK(LEDS_YELLOW));

			}
			if(light_value > THRESHOLD){
				is_sunset=false;
				leds_off(LEDS_NUM_TO_MASK(LEDS_YELLOW));
			}
		
			if(is_auto){//if in automatic mode
				if(is_sunset && !is_humid){	//if dark, not humid
					status=true;
					leds_on(LEDS_NUM_TO_MASK(LEDS_GREEN));
				}
				else {			
					status=false;
					leds_off(LEDS_NUM_TO_MASK(LEDS_GREEN));
				}
			}
			res_light.trigger();
			res_humidity.trigger();
			res_irrigator.trigger();
			etimer_reset(&timer);
			LOG_DBG("Triggered update\n");
		}

		if(ev == POST_EVENT){
			if(is_auto){//if in automatic mode
				if(is_sunset && !is_humid){	//if dark, not humid
					status=true;
					leds_on(LEDS_NUM_TO_MASK(LEDS_GREEN));
				}
				else {			
					status=false;
					leds_off(LEDS_NUM_TO_MASK(LEDS_GREEN));
				}
			}
		}
	}
	
  PROCESS_END();
}
