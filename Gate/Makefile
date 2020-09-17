CONTIKI_PROJECT = coap-gate-node
all: $(CONTIKI_PROJECT)
CONTIKI = ../../..


CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"

# Include CoAP resources
MODULES_REL += ./resources


include $(CONTIKI)/Makefile.dir-variables
# Include the CoAP implementation
MODULES += $(CONTIKI_NG_APP_LAYER_DIR)/coap

include $(CONTIKI)/Makefile.include
