#include <grpc++/grpc++.h>
#include <grpc/support/log.h>
#include <rpc.grpc.pb.h>
#include "etcd_client.h"

namespace etcd
{

static constexpr uint32_t PROBE_CONNECT_TIMEOUT = 5000;  //5s
static constexpr uint32_t MAX_CONNECT_TIMEOUT = 25000;  //25s

bool EtcdClient::connect(const std::string &url)
{
    _etcd.reset();
    _etcd = std::make_shared<EtcdClientImpl>(
                grpc::CreateChannel(url, 
                grpc::InsecureChannelCredentials()));;

    return connect_status();
}

::grpc::Status EtcdClient::probe_connect_status(uint64_t msecond)
{
    grpc::ClientContext context;
    set_client_context_token(context);
    etcdserverpb::RangeRequest request;
    request.set_key("agent_test_connect_range");
    ::etcdserverpb::RangeResponse response;

    if (0 != msecond)
    {
        std::chrono::system_clock::time_point deadline
            = std::chrono::system_clock::now() + 
            std::chrono::microseconds(msecond);

        context.set_deadline(deadline);
    }

    return _etcd->Range(&context, request, response);
}

bool EtcdClient::connect_status()
{
    ::grpc::Status status = _etcd->ConnectStatus();

    if (status.ok())  //连接成功
    {
        return true;
    }

    probe_connect_status(MAX_CONNECT_TIMEOUT);

    status = _etcd->ConnectStatus();
    if (!status.ok())
    {
        _token = ""; //异常，则清空token.
        return false;
    }

    return true;
}

bool EtcdClient::connect_realtime_status_timeout()
{
    // 处理认证失效的问题。
    ::grpc::Status status = probe_connect_status(PROBE_CONNECT_TIMEOUT);
    if (status.error_code() == grpc::StatusCode::UNAUTHENTICATED)
    {
        return false;
    }

    status = _etcd->ConnectStatus();

    if (!status.ok())
    {
        _token = ""; //异常，则清空token.
        return false;
    }

    return true;
}

bool EtcdClient::login_status()
{
    //若token非空，则说明处理连接状态。
    return !_token.empty();
}

bool EtcdClient::login(const std::string &user, 
                         const std::string &passwd)
{
    grpc::ClientContext context;
    ::etcdserverpb::AuthenticateRequest authrequest;
    ::etcdserverpb::AuthenticateResponse authresponse;
    authrequest.set_name(user);
    authrequest.set_password(passwd);
    
    ::grpc::Status status = _etcd->Authenticate(&context, authrequest, authresponse);
    if (!status.ok())
    {
        return false;
    }

    _token = authresponse.token();
    return true;
}

bool EtcdClient::range(const std::string &key,
                        bool prefix,
                        std::vector<ConfigItem>& config_item_vector)
{
    grpc::ClientContext context;
    set_client_context_token(context); //x:1,errcode:16,errmsg:etcdserver: invalid auth token
    etcdserverpb::RangeRequest request;

    request.set_key(key);
    if (prefix)
    {
        std::string end_key = key;
        ++end_key.back();
        request.set_range_end(end_key);
    }

    ::etcdserverpb::RangeResponse response;
    ::grpc::Status status = _etcd->Range(&context, request, response);

    if (!status.ok())
    {
        return false;
    }
    
    ::google::protobuf::RepeatedPtrField< ::mvccpb::KeyValue >* vlist = response.mutable_kvs();
    for (google::protobuf::RepeatedPtrField< ::mvccpb::KeyValue >::iterator it = vlist->begin(); 
         it != vlist->end(); it++)
    {
        ConfigItem km;
        km.key = it->key();
        km.value = it->value();
        km.modify_index = it->mod_revision();
        config_item_vector.push_back(km);
    }

    return true;
}

bool EtcdClient::range_one(const std::string &key, uint64_t version, ConfigItem &km)
{
    grpc::ClientContext context;
    etcdserverpb::RangeRequest request;
    set_client_context_token(context);

    request.set_key(key);

    if (version > 0)
    {
        request.set_revision(version);
    }

    ::etcdserverpb::RangeResponse response;
    ::grpc::Status status = _etcd->Range(&context, request, response);

    if (!status.ok() || !response.count())
    {
        return false;
    }

    ::mvccpb::KeyValue* kv = response.mutable_kvs(0);
    km.key = kv->key();
    km.value = kv->value();
    km.modify_index = kv->mod_revision();
        
    return true;
}

bool EtcdClient::init_watch()
{
    if (_watch_context != NULL)
    {
        delete _watch_context;
        _watch_context = NULL;
    }

    _watch_context = new grpc::ClientContext();
    set_client_context_token(*_watch_context);

    ::grpc::Status status = _etcd->InitWatch(_watch_context);

    return status.ok();
}

bool EtcdClient::set_watch(const std::string &key, bool prefix)
{
    ::etcdserverpb::WatchRequest request;
    ::etcdserverpb::WatchCreateRequest watch_req;
    watch_req.set_key(key);
    if (prefix) //前缀
    {
        std::string end_key = key;
        ++end_key.back();
        watch_req.set_range_end(end_key);
    }

    watch_req.set_prev_kv(true);
    request.mutable_create_request()->CopyFrom(watch_req);
    grpc::WriteOptions wot;
    wot.set_no_compression();

    return _etcd->SetWatch(wot, request).ok();
}

bool EtcdClient::watch(std::vector<ConfigItem> &config_item_vector)
{
    ::etcdserverpb::WatchResponse response;
    if(_etcd->Watch(response).ok()){
        ::google::protobuf::RepeatedPtrField< ::mvccpb::Event > *elist = response.mutable_events();
        for(::google::protobuf::RepeatedPtrField< ::mvccpb::Event >::const_iterator it =
            elist->begin(); it != elist->end(); it++)
        {
            if (!it->has_kv())
            {
                continue;
            }

            const ::mvccpb::KeyValue& kvev = it->kv();
            ConfigItem km;
            km.key = kvev.key();
            km.value = kvev.value();
            km.modify_index = kvev.mod_revision();

            if (::mvccpb::Event_EventType::Event_EventType_DELETE == it->type())
            {
                km.operation = KEY_OPRATION::KEY_OPRATION_DELETE;
            }
            else if (::mvccpb::Event_EventType::Event_EventType_PUT == it->type())
            {
                if (it->has_prev_kv()){
                    km.operation = KEY_OPRATION::KEY_OPRATION_ADD;
                }
                else{
                    km.operation = KEY_OPRATION::KEY_OPRATION_UPDATE;
                }
            }
            config_item_vector.push_back(km);
        }
        return true;
    }
    return false;
}

void EtcdClient::end_watch()
{
    _etcd->FinishWatch();
}

void EtcdClient::set_client_context_token(grpc::ClientContext &context)
{
    if (_token.empty()){
        return;
    }
    context.AddMetadata(KEY_TOKEN,_token);
}

}

