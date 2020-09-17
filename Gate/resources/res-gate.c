#include <stdlib.h>
#include <string.h>
#include "coap-engine.h"
#include "contiki.h"
#include "os/dev/leds.h"

/*log conf*/
#include "sys/log.h"
#define LOG_MODULE "Gate"
#define LOG_LEVEL LOG_LEVEL_DBG

process_event_t OPENED_GATE_EVENT;
process_event_t CLOSED_GATE_EVENT;

extern struct process gate_node;

bool status = false;

static void res_get_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void res_post_put_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void res_event_handler(void);

EVENT_RESOURCE(res_gate, "title=\"Gate: ?POST/PUT status=ON|OFF\";rt=\"Gate\";obs",
         res_get_handler,
         res_post_put_handler,
         res_post_put_handler,
         NULL,
	 res_event_handler); 


static void res_event_handler(void){   
	LOG_DBG("res event handler called");
	coap_notify_observers(&res_gate);
}

static void res_get_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset){

	if(request!=NULL){
		LOG_DBG("Received GET\n");
	}
	
	char *res_mode = NULL;

	if(status){
		res_mode="ON";
	}
	else {
		res_mode="OFF";
	}

	unsigned int accept = -1;
  coap_get_header_accept(request, &accept);

  if(accept == TEXT_PLAIN) {
    coap_set_header_content_format(response, TEXT_PLAIN);
    snprintf((char *)buffer, COAP_MAX_CHUNK_SIZE, "status=%s", res_mode);
    coap_set_payload(response, (uint8_t *)buffer, strlen((char *)buffer));
    
  } else if(accept == APPLICATION_XML) {
    coap_set_header_content_format(response, APPLICATION_XML);
    snprintf((char *)buffer, COAP_MAX_CHUNK_SIZE, "\"<status=\"%s\"/>", res_mode);
    coap_set_payload(response, buffer, strlen((char *)buffer));
    
  } else if(accept == -1 || accept == APPLICATION_JSON) {
    coap_set_header_content_format(response, APPLICATION_JSON);
    snprintf((char *)buffer, COAP_MAX_CHUNK_SIZE, "{\"status\":\"%s\"}", res_mode);
    coap_set_payload(response, buffer, strlen((char *)buffer));
    
  } else {
    coap_set_status_code(response, NOT_ACCEPTABLE_4_06);
    const char *msg = "Supporting content-types text/plain, application/xml, and application/json";
    coap_set_payload(response, msg, strlen(msg));
  }
}

static void res_post_put_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset){  
	
	size_t len = 0; 
	const char *mode = NULL;
	int success = 0;
	
	if((len = coap_get_post_variable(request, "status", &mode))){
		LOG_DBG("%d,%s\n",status,mode);
		if(strncmp(mode, "ON", len)== 0){
			status=true;
			leds_single_on(LEDS_GREEN);
			success = 1;
		}else if(strncmp(mode, "OFF", len)== 0){
			status=false;
			leds_single_off(LEDS_GREEN);
			success = 1;	
		}	
		LOG_DBG("status:%d, mode:%s\n",status,mode);		
	}

	if (success == 0){
		coap_set_status_code(response, BAD_REQUEST_4_00);
		process_post(&gate_node,CLOSED_GATE_EVENT,NULL);	
	}else{
		coap_set_status_code(response, CHANGED_2_04);
		if(strncmp(mode, "ON", len)== 0){
			LOG_DBG("PROCESS POST\n");
			process_post(&gate_node,OPENED_GATE_EVENT,NULL); 
		}	
	}

	
}



