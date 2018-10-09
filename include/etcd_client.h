#ifndef _ETCD_CLIENT_EX_H_
#define _ETCD_CLIENT_EX_H_

#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <etcd_client_impl.h>
#include <etcd_comm.h>

namespace etcd
{

class EtcdClient
{
public:
    explicit EtcdClient()
        :_watch_context(NULL)
    {
    }
    ~EtcdClient(){}

public:
    bool connect(const std::string &url = "");

    bool connect_status();

    bool connect_realtime_status_timeout();

    bool login_status();

    bool login(const std::string &user, const std::string &passwd);

    bool range(const std::string &key, bool prefix, std::vector<ConfigItem> &config_item_vector);

    bool range_one(const std::string &key, uint64_t version, ConfigItem &km);

    bool init_watch();

    bool set_watch(const std::string &key, bool prefix);

    bool watch(std::vector<ConfigItem> &config_item_vector);
    
    void end_watch();

private:

    void set_client_context_token(grpc::ClientContext &context);

    ::grpc::Status probe_connect_status(uint64_t msecond);
    
private:
    std::string _token;   //登陆token，当链接断开，或未登陆，token被清空。
    std::shared_ptr<EtcdClientImpl> _etcd;
    grpc::ClientContext *_watch_context;
};

} // namespace etcd

#endif // _ETCD_CLIENT_EX_H_



