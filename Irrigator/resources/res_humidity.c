#include <stdio.h>
#include <string.h>
#include "coap-engine.h"
#include "coap.h"
#include "os/dev/leds.h"

#include"time.h"

#include "sys/log.h"
#define LOG_MODULE "Humidity Sensor"
#define LOG_LEVEL LOG_LEVEL_DBG

int humidity_value = 20;
bool is_humid = false;

static void res_get_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void res_event_handler(void);

EVENT_RESOURCE(res_humidity,
               "title=\"Humidity Sensor\";rt=\"humidity Sensor\";obs",
               res_get_handler,
               NULL,
               NULL,
               NULL,
               res_event_handler);


static void res_get_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset){

	if(request !=NULL){
		LOG_DBG("Received GET\n");
	}

	unsigned int accept = -1;
	  coap_get_header_accept(request, &accept);

	  if(accept == TEXT_PLAIN) {
	    coap_set_header_content_format(response, TEXT_PLAIN);
	    snprintf((char *)buffer, COAP_MAX_CHUNK_SIZE, "value=%d", humidity_value);
	    coap_set_payload(response, (uint8_t *)buffer, strlen((char *)buffer));
	    
	  } else if(accept == APPLICATION_XML) {
	    coap_set_header_content_format(response, APPLICATION_XML);
	    snprintf((char *)buffer, COAP_MAX_CHUNK_SIZE, "<value=\"%d\"/>", humidity_value);
	    coap_set_payload(response, buffer, strlen((char *)buffer));
	    
	  } else if(accept == -1 || accept == APPLICATION_JSON) {
	    coap_set_header_content_format(response, APPLICATION_JSON);
	    snprintf((char *)buffer, COAP_MAX_CHUNK_SIZE, "{\"value\":%d}", humidity_value);
	    coap_set_payload(response, buffer, strlen((char *)buffer));
	    
	  } else {
	    coap_set_status_code(response, NOT_ACCEPTABLE_4_06);
	    const char *msg = "Supporting content-types text/plain, application/xml, and application/json";
	    coap_set_payload(response, msg, strlen(msg));
	  }
}

static void res_event_handler(void) {
	LOG_DBG("Sending notification");
  	coap_notify_observers(&res_humidity);
}


