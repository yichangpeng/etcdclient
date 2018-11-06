#pragma once
#include <string>

namespace etcd{

static const char * KEY_TOKEN = "token";

enum class KEY_OPRATION : unsigned int
{
    KEY_OPRATION_NONE   = 0,         // 无操作
    KEY_OPRATION_UPDATE = 1,         // 更新
    KEY_OPRATION_DELETE = 2,         // 删除
    KEY_OPRATION_ADD = 3,         // 新增
};

struct ConfigItem
{
    ConfigItem():
        modify_index(0),
        operation(KEY_OPRATION::KEY_OPRATION_NONE) 
    {
    }

    std::string key;
    std::string value;
    int64_t modify_index;
    KEY_OPRATION operation;

};

}
