#include <etcd_comm.h>
#include <etcd_client.h>
#include <unistd.h>

void test_range(etcd::EtcdClient& client,const std::string& key){
        std::vector<etcd::ConfigItem> result;
        client.range(key,false,result);
        for(etcd::ConfigItem & item : result){
            std::cout << "[test_range- " << item.key  << "-" << item.value << "-" << item.modify_index << " ]" << std::endl;
        }
}

void test_watch(etcd::EtcdClient& client,const std::string& key){
    if(!client.init_watch())
    {
        std::cout << "init_watch error!" << std::endl;
        return;
    }
    
    if(!client.set_watch(key,false))
    {
        std::cout << "set_watch error!" << std::endl;
        return;
    }

    while(true){
        std::vector<etcd::ConfigItem> result;
        if(!client.watch(result))
        {
            client.end_watch();
            std::cout << "watch error!" << std::endl;
            return;
        }
        for(etcd::ConfigItem & item : result){
            std::cout << "[test_watch " << item.key  << "-" << item.value << "-" << item.modify_index << "-" << static_cast<unsigned int>(item.operation) << " ]" << std::endl;
        }
    }

}

int main(int argc, const char ** argv)
{
    if (argc < 2){
        std::cout << "param error!" << std::endl;
        return -1;
    }
    etcd::EtcdClient client;
    if(!client.connect("127.0.0.1:2379")){
        std::cout << "connect error!" << std::endl;
        return false;
    }
    test_range(client,argv[1]);
    test_watch(client,argv[1]);
    return 0;
}

