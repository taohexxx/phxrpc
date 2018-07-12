include /phx/phxrpc/phxrpc.mk

LDFLAGS := -L$(PHXRPC_ROOT)/lib -lphxrpc $(LDFLAGS)

LIB_MQTT_OBJS = ../mqtt/mqtt_client.o ../mqtt/mqtt_msg.o ../mqtt/mqtt_msg_handler.o ../mqtt/mqtt_utils.o \
		../mqtt/mqtt_msg_handler_factory.o ../mqtt/mqtt_caller.o

CLI_OBJS = mqttbroker.pb.o \
		search_client.o \
		search_client_uthread.o \
		phxrpc_search_stub.o

all: libsearch_client.a search_tool_main

libsearch_client.a: $(CLI_OBJS)
	$(AR) $@ $^

search_tool_main: phxrpc_search_tool.o search_tool_impl.o search_tool_main.o $(LIB_MQTT_OBJS)
	$(LINKER) $^ -L. -lsearch_client $(LDFLAGS) -o $@

########## message ##########

mqttbroker.pb.cc: mqttbroker.pb.h

mqttbroker.pb.h: mqttbroker.proto
	$(PROTOBUF_ROOT)/bin/protoc -I$(PROTOBUF_ROOT)/include --cpp_out=. -I$(PHXRPC_ROOT) -I. $^

########## client ##########

phxrpc_search_stub.cpp: phxrpc_search_stub.h
phxrpc_search_stub.o: phxrpc_search_stub.h
search_client.cpp: phxrpc_search_stub.h
search_client.o: phxrpc_search_stub.h
search_client_uthread.cpp: phxrpc_search_stub.h
search_client_uthread.o: phxrpc_search_stub.h

########## tool ##########

phxrpc_search_tool.cpp: phxrpc_search_tool.h
phxrpc_search_tool.o: phxrpc_search_tool.h
search_tool_impl.cpp: phxrpc_search_tool.h
search_tool_impl.o: phxrpc_search_tool.h
search_tool_main.cpp: phxrpc_search_tool.h
search_tool_main.o: phxrpc_search_tool.h

clean:
	@($(RM) $(TARGETS))
	@($(RM) *.o)

