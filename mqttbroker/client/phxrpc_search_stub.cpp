/* phxrpc_search_stub.cpp

 Generated by phxrpc_pb2client from search.proto

 Please DO NOT edit unless you know exactly what you are doing.

*/

#include "phxrpc_search_stub.h"

#include "phxrpc/rpc.h"
#include "phxrpc/network.h"


SearchStub::SearchStub(phxrpc::BaseTcpStream &socket, phxrpc::ClientMonitor &client_monitor,
                       phxrpc::BaseMessageHandlerFactory *const msg_handler_factory)
        : socket_(socket), client_monitor_(client_monitor), keep_alive_(false),
          msg_handler_factory_(msg_handler_factory) {
}

SearchStub::~SearchStub() {
}

void SearchStub::set_keep_alive(const bool keep_alive) {
    keep_alive_ = keep_alive;
}

// http protocol

int SearchStub::PHXEcho(const google::protobuf::StringValue &req,
                        google::protobuf::StringValue *resp) {
    phxrpc::Caller caller(socket_, client_monitor_, msg_handler_factory_);
    caller.set_uri("/phxqueue_phxrpc.mqttbroker/PHXEcho", -1);
    caller.set_keep_alive(keep_alive_);
    return caller.Call(req, resp);
}

int SearchStub::PhxHttpPublish(const phxqueue_phxrpc::mqttbroker::HttpPublishPb &req,
                               phxqueue_phxrpc::mqttbroker::HttpPubackPb *resp) {
    phxrpc::Caller caller(socket_, client_monitor_, msg_handler_factory_);
    caller.set_uri("/phxqueue_phxrpc.mqttbroker/PhxHttpPublish", 2000031);
    caller.set_keep_alive(keep_alive_);
    return caller.Call(req, resp);
}

// mqtt protocol

int SearchStub::PhxMqttConnect(const phxqueue_phxrpc::mqttbroker::MqttConnectPb &req,
                               phxqueue_phxrpc::mqttbroker::MqttConnackPb *resp) {
    phxrpc::Caller caller(socket_, client_monitor_, msg_handler_factory_);
    caller.set_uri("/phxqueue_phxrpc.mqttbroker/PhxMqttConnect", 2000011);
    caller.set_keep_alive(keep_alive_);
    return caller.Call(req, resp);
}

int SearchStub::PhxMqttPublish(const phxqueue_phxrpc::mqttbroker::MqttPublishPb &req,
                               google::protobuf::Empty *resp) {
    phxrpc::Caller caller(socket_, client_monitor_, msg_handler_factory_);
    caller.set_uri("/phxqueue_phxrpc.mqttbroker/PhxMqttPublish", 2000012);
    caller.set_keep_alive(keep_alive_);
    return caller.Call(req, resp);
}

int SearchStub::PhxMqttPuback(const phxqueue_phxrpc::mqttbroker::MqttPubackPb &req,
                              google::protobuf::Empty *resp) {
    phxrpc::Caller caller(socket_, client_monitor_, msg_handler_factory_);
    caller.set_uri("/phxqueue_phxrpc.mqttbroker/PhxMqttPuback", 2000013);
    caller.set_keep_alive(keep_alive_);
    return caller.Call(req, resp);
}

int SearchStub::PhxMqttPubrec(const phxqueue_phxrpc::mqttbroker::MqttPubrecPb &req,
                              google::protobuf::Empty *resp) {
    phxrpc::Caller caller(socket_, client_monitor_, msg_handler_factory_);
    caller.set_uri("/phxqueue_phxrpc.mqttbroker/PhxMqttPubrec", 2000014);
    caller.set_keep_alive(keep_alive_);
    return caller.Call(req, resp);
}

int SearchStub::PhxMqttPubrel(const phxqueue_phxrpc::mqttbroker::MqttPubrelPb &req,
                              google::protobuf::Empty *resp) {
    phxrpc::Caller caller(socket_, client_monitor_, msg_handler_factory_);
    caller.set_uri("/phxqueue_phxrpc.mqttbroker/PhxMqttPubrel", 2000015);
    caller.set_keep_alive(keep_alive_);
    return caller.Call(req, resp);
}

int SearchStub::PhxMqttPubcomp(const phxqueue_phxrpc::mqttbroker::MqttPubcompPb &req,
                               google::protobuf::Empty *resp) {
    phxrpc::Caller caller(socket_, client_monitor_, msg_handler_factory_);
    caller.set_uri("/phxqueue_phxrpc.mqttbroker/PhxMqttPubcomp", 2000016);
    caller.set_keep_alive(keep_alive_);
    return caller.Call(req, resp);
}

int SearchStub::PhxMqttSubscribe(const phxqueue_phxrpc::mqttbroker::MqttSubscribePb &req,
                                 phxqueue_phxrpc::mqttbroker::MqttSubackPb *resp) {
    phxrpc::Caller caller(socket_, client_monitor_, msg_handler_factory_);
    caller.set_uri("/phxqueue_phxrpc.mqttbroker/PhxMqttSubscribe", 2000017);
    caller.set_keep_alive(keep_alive_);
    return caller.Call(req, resp);
}

int SearchStub::PhxMqttUnsubscribe(const phxqueue_phxrpc::mqttbroker::MqttUnsubscribePb &req,
                                   phxqueue_phxrpc::mqttbroker::MqttUnsubackPb *resp) {
    phxrpc::Caller caller(socket_, client_monitor_, msg_handler_factory_);
    caller.set_uri("/phxqueue_phxrpc.mqttbroker/PhxMqttUnsubscribe", 2000018);
    caller.set_keep_alive(keep_alive_);
    return caller.Call(req, resp);
}

int SearchStub::PhxMqttPing(const phxqueue_phxrpc::mqttbroker::MqttPingreqPb &req,
                            phxqueue_phxrpc::mqttbroker::MqttPingrespPb *resp) {
    phxrpc::Caller caller(socket_, client_monitor_, msg_handler_factory_);
    caller.set_uri("/phxqueue_phxrpc.mqttbroker/PhxMqttPing", 2000019);
    caller.set_keep_alive(keep_alive_);
    return caller.Call(req, resp);
}

int SearchStub::PhxMqttDisconnect(const phxqueue_phxrpc::mqttbroker::MqttDisconnectPb &req,
                                  google::protobuf::Empty *resp) {
    phxrpc::Caller caller(socket_, client_monitor_, msg_handler_factory_);
    caller.set_uri("/phxqueue_phxrpc.mqttbroker/PhxMqttDisconnect", 2000020);
    caller.set_keep_alive(keep_alive_);
    return caller.Call(req, resp);
}

