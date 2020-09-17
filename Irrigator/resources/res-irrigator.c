#include <stdlib.h>
#include <string.h>
#include "coap-engine.h"
#include "contiki.h"
#include "os/dev/leds.h"

/*log conf*/
#include "sys/log.h"
#define LOG_MODULE "Watering Actuator"
#define LOG_LEVEL LOG_LEVEL_DBG

extern struct process irrigator_node;
process_event_t POST_EVENT;
bool status = false;
bool is_auto = true;

static void res_get_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void res_post_put_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void res_event_handler(void);

EVENT_RESOURCE(res_irrigator, "title=\"Watering actuator: ?POST/PUT auto=AUTO|MAN&status=ON|OFF\";rt=\"Irrigator\";obs",
	       res_get_handler,
               res_post_put_handler,
               res_post_put_handler,
               NULL,
               res_event_handler);

static void res_event_handler(void) {
	LOG_DBG("sending notification");
  	coap_notify_observers(&res_irrigator);
}

static void res_get_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset){

	  if(request!=NULL){
		LOG_DBG("Received GET\n");
	  }

	  LOG_DBG("Status: %d , Auto: %d \n", status, is_auto);

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
	    snprintf((char *)buffer, COAP_MAX_CHUNK_SIZE, "status=%s,auto=%d", res_mode, is_auto);
	    coap_set_payload(response, (uint8_t *)buffer, strlen((char *)buffer));
	    
	  } else if(accept == APPLICATION_XML) {
	    coap_set_header_content_format(response, APPLICATION_XML);
	    snprintf((char *)buffer, COAP_MAX_CHUNK_SIZE, "<status=\"%s\"/><auto=\"%d\"/>", res_mode, is_auto);
	    coap_set_payload(response, buffer, strlen((char *)buffer));
	    
	  } else if(accept == -1 || accept == APPLICATION_JSON) {
	    coap_set_header_content_format(response, APPLICATION_JSON);
	    snprintf((char *)buffer, COAP_MAX_CHUNK_SIZE, "{\"status\":\"%s\",\"auto\":\"%d\"}", res_mode, is_auto);
	    coap_set_payload(response, buffer, strlen((char *)buffer));
	    
	  } else {
	    coap_set_status_code(response, NOT_ACCEPTABLE_4_06);
	    const char *msg = "Supporting content-types text/plain, application/xml, and application/json";
	    coap_set_payload(response, msg, strlen(msg));
	  }
}

static void res_post_put_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset){  
	
	if(request!=NULL){
		LOG_DBG("Received POST/PUT\n");
	}

	size_t len = 0; 
	const char *status_s = NULL;
	const char *is_auto_s = NULL;
	int success = 0;
	int success2 = 0;

	if((len = coap_get_post_variable(request, "auto", &is_auto_s))){

		if(strncmp(is_auto_s, "AUTO", len)== 0){
			is_auto = true;
			LOG_DBG("in auto mode\n");
			
			success = 1;

		}else if(strncmp(is_auto_s, "MAN", len)== 0){
			is_auto = false;
			LOG_DBG("in manual mode\n");
			
			success = 1;
		}			
	}

	if(success&&is_auto){
		success2 = 1;
	}
	else if((len = coap_get_post_variable(request, "status", &status_s)) && !is_auto && success){
		if(strncmp(status_s, "ON", len)== 0){
			status=true;
			LOG_DBG("Starting watering\n");
			leds_on(LEDS_NUM_TO_MASK(LEDS_GREEN));
			success2 = 1;

		}else if(strncmp(status_s, "OFF", len)== 0){
			status=false;
			LOG_DBG("Stop watering\n");
			leds_off(LEDS_NUM_TO_MASK(LEDS_GREEN));
			success2 = 1;
		}			
	}

	if (success==0||((success2 == 0) && (success==1))){
		coap_set_status_code(response, BAD_REQUEST_4_00);
	}else{
		coap_set_status_code(response, CHANGED_2_04);
		process_post(&irrigator_node,POST_EVENT,NULL); 
	}
	
}



