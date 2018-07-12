/* mqttbroker_service_impl.cpp

 Generated by phxrpc_pb2service from mqttbroker.proto

*/

#include "mqttbroker_service_impl.h"

#include "phxrpc/file.h"

#include "event_loop_server.h"
#include "mqtt/mqtt_msg.h"
#include "mqtt/mqtt_packet_id.h"
#include "mqtt/mqtt_session.h"
#include "mqttbroker.pb.h"
#include "mqttbroker_server_config.h"


using namespace std;


MqttBrokerServiceImpl::MqttBrokerServiceImpl(ServiceArgs_t &app_args,
        phxrpc::UThreadEpollScheduler *const worker_uthread_scheduler,
        phxrpc::DataFlowArgs *data_flow_args)
        : args_(app_args),
          worker_uthread_scheduler_(worker_uthread_scheduler),
          data_flow_args_(data_flow_args) {
}

MqttBrokerServiceImpl::~MqttBrokerServiceImpl() {
}

int MqttBrokerServiceImpl::PHXEcho(const google::protobuf::StringValue &req,
                                   google::protobuf::StringValue *resp) {
    resp->set_value(req.value());

    return 0;
}

int MqttBrokerServiceImpl::PhxHttpPublish(const phxqueue_phxrpc::mqttbroker::HttpPublishPb &req,
                                          phxqueue_phxrpc::mqttbroker::HttpPubackPb *resp) {
    // 1. check local session
    const auto sub_mqtt_session(args_.mqtt_session_mgr->GetByClientId(req.sub_client_id()));
    if (!sub_mqtt_session) {
        phxrpc::log(LOG_ERR, "%s GetByClientId err sub_client_id \"%s\"",
                    __func__, req.sub_client_id().c_str());

        return -1;
    }

    // 2. publish to event_loop_server
    phxqueue_phxrpc::mqttbroker::MqttPublishPb mqtt_publish_pb;
    mqtt_publish_pb.CopyFrom(req.mqtt_publish());
    // mqtt-3.3.1-3: reset dup
    mqtt_publish_pb.set_dup(false);

    if (1 == mqtt_publish_pb.qos()) {
        uint16_t sub_packet_id{0};
        if (!args_.mqtt_packet_id_mgr->AllocPacketId(req.pub_client_id(),
                req.mqtt_publish().packet_identifier(), req.sub_client_id(), sub_packet_id)) {
            phxrpc::log(LOG_ERR, "%s sub_session_id %" PRIx64 " AllocPacketId err sub_client_id \"%s\"",
                        __func__, sub_mqtt_session->session_id, req.sub_client_id().c_str());

            return -1;
        }
        mqtt_publish_pb.set_packet_identifier(sub_packet_id);

        // ack_key = sub_client_id + sub_packet_id
        const string ack_key(req.sub_client_id() + ':' + to_string(sub_packet_id));
        void *data{nullptr};
        auto *mqtt_publish(new phxqueue_phxrpc::mqttbroker::MqttPublish);
        mqtt_publish->FromPb(mqtt_publish_pb);
        args_.server_mgr->SendAndWaitAck(sub_mqtt_session->session_id, mqtt_publish,
                                         worker_uthread_scheduler_, ack_key, data);

        phxqueue_phxrpc::mqttbroker::MqttPuback *puback{
                (phxqueue_phxrpc::mqttbroker::MqttPuback *)data};
        if (!puback) {
            phxrpc::log(LOG_ERR, "%s sub_session_id %" PRIx64 " server_mgr.SendAndWaitAck nullptr "
                        "qos %u ack_key \"%s\"", __func__, sub_mqtt_session->session_id,
                        req.mqtt_publish().qos(), ack_key.c_str());

            return -1;
        }

        phxrpc::log(LOG_NOTICE, "%s sub_session_id %" PRIx64
                    " server_mgr.SendAndWaitAck ack_key \"%s\" qos %u",
                    __func__, sub_mqtt_session->session_id,
                    ack_key.c_str(), req.mqtt_publish().qos());

        int ret{puback->ToPb(resp->mutable_mqtt_puback())};

        delete puback;

        args_.mqtt_packet_id_mgr->ReleasePacketId(req.pub_client_id(),
                req.mqtt_publish().packet_identifier(), req.sub_client_id());

        if (0 != ret) {
            phxrpc::log(LOG_ERR, "%s ToPb err %d", __func__, ret);

            return ret;
        }
    } else {
        auto *mqtt_publish(new phxqueue_phxrpc::mqttbroker::MqttPublish);
        mqtt_publish->FromPb(mqtt_publish_pb);
        args_.server_mgr->Send(sub_mqtt_session->session_id, mqtt_publish);
        phxrpc::log(LOG_NOTICE, "%s sub_session_id %" PRIx64
                    " server_mgr.Send sub_client_id \"%s\" qos %u",
                    __func__, sub_mqtt_session->session_id, req.sub_client_id().c_str(),
                    req.mqtt_publish().qos());
    }

    return 0;
}

int MqttBrokerServiceImpl::PhxMqttConnect(const phxqueue_phxrpc::mqttbroker::MqttConnectPb &req,
                                          phxqueue_phxrpc::mqttbroker::MqttConnackPb *resp) {
    const auto old_mqtt_session(args_.mqtt_session_mgr->GetByClientId(
            req.client_identifier()));
    SessionMgr *session_mgr{(SessionMgr *)(data_flow_args_->session_mgr)};
    if (old_mqtt_session) {
        if (old_mqtt_session->session_id == data_flow_args_->session_id) {
            // mqtt-3.1.0-2: disconnect current connection

            args_.mqtt_session_mgr->DestroyBySessionId(data_flow_args_->session_id);
            session_mgr->DestroySession(data_flow_args_->session_id);

            phxrpc::log(LOG_ERR, "%s err session_id %" PRIx64 " client_id \"%s\"", __func__,
                        data_flow_args_->session_id, req.client_identifier().c_str());

            return -1;
        } else {
            // mqtt-3.1.4-2: disconnect other connection with same client_id
            args_.mqtt_session_mgr->DestroyBySessionId(old_mqtt_session->session_id);
            session_mgr->DestroySession(old_mqtt_session->session_id);

            phxrpc::log(LOG_NOTICE, "%s disconnect session_id old %" PRIx64 " new %" PRIx64
                        " client_id \"%s\"", __func__, old_mqtt_session->session_id,
                        data_flow_args_->session_id, req.client_identifier().c_str());
        }
    }

    // mqtt connect: set client_id and init
    const auto mqtt_session(args_.mqtt_session_mgr->Create(
            req.client_identifier(), data_flow_args_->session_id));
    mqtt_session->keep_alive = req.keep_alive();
    mqtt_session->Heartbeat();

    resp->set_session_present(!req.clean_session());
    resp->set_connect_return_code(0u);

    phxrpc::log(LOG_NOTICE, "%s session_id %" PRIx64 " client_id \"%s\"", __func__,
                data_flow_args_->session_id, req.client_identifier().c_str());

    return 0;
}

int MqttBrokerServiceImpl::PhxMqttPublish(const phxqueue_phxrpc::mqttbroker::MqttPublishPb &req,
                                          google::protobuf::Empty *resp) {
    // 1. check local session
    MqttSession *mqtt_session{nullptr};
    int ret{CheckSession(mqtt_session)};
    if (0 != ret) {
        phxrpc::log(LOG_ERR, "%s session_id %" PRIx64 " CheckSession err %d qos %u packet_id %d",
                    __func__, data_flow_args_->session_id, ret, req.qos(), req.packet_identifier());

        return ret;
    }

    // 2. ack
    if (1 == req.qos()) {
        phxqueue_phxrpc::mqttbroker::MqttPubackPb puback_pb;
        puback_pb.set_packet_identifier(req.packet_identifier());
        auto *puback(new phxqueue_phxrpc::mqttbroker::MqttPuback);
        puback->FromPb(puback_pb);
        args_.server_mgr->Send(data_flow_args_->session_id, (phxrpc::BaseResponse *)puback);
    }

    // TODO: remove
    if (isprint(req.data().at(req.data().size() - 1)) && isprint(req.data().at(0))) {
        phxrpc::log(LOG_NOTICE, "%s session_id %" PRIx64 " client_id \"%s\" qos %u packet_id %d topic \"%s\" data \"%s\"",
                    __func__, data_flow_args_->session_id, mqtt_session->client_id.c_str(), req.qos(), req.packet_identifier(),
                    req.topic_name().c_str(), req.data().c_str());
    } else {
        phxrpc::log(LOG_NOTICE, "%s session_id %" PRIx64 " client_id \"%s\" qos %u packet_id %d topic \"%s\" data.size %zu",
                    __func__, data_flow_args_->session_id, mqtt_session->client_id.c_str(), req.qos(), req.packet_identifier(),
                    req.topic_name().c_str(), req.data().size());
    }

    return 0;
}

int MqttBrokerServiceImpl::PhxMqttPuback(const phxqueue_phxrpc::mqttbroker::MqttPubackPb &req,
                                         google::protobuf::Empty *resp) {
    // 1. check local session
    MqttSession *mqtt_session{nullptr};
    int ret{CheckSession(mqtt_session)};
    if (0 != ret) {
        phxrpc::log(LOG_ERR, "%s session_id %" PRIx64 " CheckSession err %d packet_id %d",
                    __func__, data_flow_args_->session_id, ret, req.packet_identifier());

        return ret;
    }

    // 2. puback to hsha_server
    auto *puback(new phxqueue_phxrpc::mqttbroker::MqttPuback);
    ret = puback->FromPb(req);
    if (0 != ret) {
        delete puback;
        phxrpc::log(LOG_ERR, "%s session_id %" PRIx64 " FromPb err %d packet_id %d",
                    __func__, data_flow_args_->session_id, ret, req.packet_identifier());

        return ret;
    }

    // ack_key = sub_client_id + sub_packet_id
    const string ack_key(mqtt_session->client_id + ':' + to_string(req.packet_identifier()));
    // forward puback and do not delete here
    int ret2{args_.server_mgr->Ack(ack_key, (void *)puback)};
    if (0 != ret2) {
        phxrpc::log(LOG_ERR, "%s session_id %" PRIx64 " server_mgr.Ack err %d "
                    "ack_key \"%s\"", __func__, data_flow_args_->session_id,
                    ret2, ack_key.c_str());

        return ret2;
    }

    phxrpc::log(LOG_NOTICE, "%s session_id %" PRIx64 " packet_id %d",
                __func__, data_flow_args_->session_id, req.packet_identifier());

    return ret2;
}

int MqttBrokerServiceImpl::PhxMqttPubrec(const phxqueue_phxrpc::mqttbroker::MqttPubrecPb &req,
                                         google::protobuf::Empty *resp) {
    return -1;
}

int MqttBrokerServiceImpl::PhxMqttPubrel(const phxqueue_phxrpc::mqttbroker::MqttPubrelPb &req,
                                         google::protobuf::Empty *resp) {
    return -1;
}

int MqttBrokerServiceImpl::PhxMqttPubcomp(const phxqueue_phxrpc::mqttbroker::MqttPubcompPb &req,
                                          google::protobuf::Empty *resp) {
    return -1;
}

int MqttBrokerServiceImpl::PhxMqttSubscribe(const phxqueue_phxrpc::mqttbroker::MqttSubscribePb &req,
                                            phxqueue_phxrpc::mqttbroker::MqttSubackPb *resp) {
    resp->set_packet_identifier(req.packet_identifier());
    resp->clear_return_codes();
    for (int i{0}; req.topic_filters().size() > i; ++i) {
        resp->add_return_codes(0x00);
        phxrpc::log(LOG_DEBUG, "topic \"%s\"", req.topic_filters(i).c_str());
    }

    phxrpc::log(LOG_NOTICE, "%s session_id %" PRIx64 " packet_id %d", __func__,
                data_flow_args_->session_id, req.packet_identifier());

    return 0;
}

int MqttBrokerServiceImpl::PhxMqttUnsubscribe(const phxqueue_phxrpc::mqttbroker::MqttUnsubscribePb &req,
                                              phxqueue_phxrpc::mqttbroker::MqttUnsubackPb *resp) {
    resp->set_packet_identifier(req.packet_identifier());
    for (int i{0}; req.topic_filters().size() > i; ++i) {
        phxrpc::log(LOG_DEBUG, "topic \"%s\"", req.topic_filters(i).c_str());
    }

    phxrpc::log(LOG_NOTICE, "%s session_id %" PRIx64 " packet_id %d",
                __func__, data_flow_args_->session_id, req.packet_identifier());

    return 0;
}

int MqttBrokerServiceImpl::PhxMqttPing(const phxqueue_phxrpc::mqttbroker::MqttPingreqPb &req,
                                       phxqueue_phxrpc::mqttbroker::MqttPingrespPb *resp) {
    const auto mqtt_session(args_.mqtt_session_mgr->GetBySessionId(data_flow_args_->session_id));
    mqtt_session->Heartbeat();

    phxrpc::log(LOG_NOTICE, "%s session_id %" PRIx64, __func__, data_flow_args_->session_id);

    return 0;
}

int MqttBrokerServiceImpl::PhxMqttDisconnect(const phxqueue_phxrpc::mqttbroker::MqttDisconnectPb &req,
                                             google::protobuf::Empty *resp) {
    args_.mqtt_session_mgr->DestroyBySessionId(data_flow_args_->session_id);
    SessionMgr *session_mgr{(SessionMgr *)(data_flow_args_->session_mgr)};
    session_mgr->DestroySession(data_flow_args_->session_id);

    phxrpc::log(LOG_NOTICE, "%s session_id %" PRIx64, __func__, data_flow_args_->session_id);

    return 0;
}

int MqttBrokerServiceImpl::CheckSession(MqttSession *&mqtt_session) {
    SessionMgr *session_mgr{(SessionMgr *)(data_flow_args_->session_mgr)};
    const auto tmp_session(session_mgr->GetSession(data_flow_args_->session_id));
    const auto tmp_mqtt_session(args_.mqtt_session_mgr->GetBySessionId(data_flow_args_->session_id));
    if (!tmp_session || !tmp_mqtt_session || tmp_mqtt_session->IsExpired()) {
        // ignore return

        // destroy local session
        args_.mqtt_session_mgr->DestroyBySessionId(data_flow_args_->session_id);
        SessionMgr *session_mgr{(SessionMgr *)(data_flow_args_->session_mgr)};
        session_mgr->DestroySession(data_flow_args_->session_id);

        return -1;
    }
    mqtt_session = tmp_mqtt_session;

    return 0;
}

