#pragma once
#include <grpc++/grpc++.h>
#include <grpc/support/log.h>
#include <rpc.grpc.pb.h>

namespace etcd{

class EtcdClientImpl
{
public:
    explicit EtcdClientImpl(std::shared_ptr<grpc::Channel> channel);
    ~EtcdClientImpl();
    
    ::grpc::Status ConnectStatus();

     // 查询key.
     ::grpc::Status Range(grpc::ClientContext *context,
		 const etcdserverpb::RangeRequest &request,
		 ::etcdserverpb::RangeResponse &response);

     //创建 更新 key
     ::grpc::Status Put(grpc::ClientContext *context,
		 const etcdserverpb::PutRequest &request,
		 ::etcdserverpb::PutResponse &response);

     //删除key
     ::grpc::Status DeleteRange(grpc::ClientContext *context,
		 const etcdserverpb::DeleteRangeRequest &request,
		 ::etcdserverpb::DeleteRangeResponse &response);

    //事务
    ::grpc::Status Txn(grpc::ClientContext *context,
		 const ::etcdserverpb::TxnRequest& request,
		 ::etcdserverpb::TxnResponse &response);

    //压缩
    ::grpc::Status Compact(grpc::ClientContext *context,
		 const ::etcdserverpb::CompactionRequest& request,
		 ::etcdserverpb::CompactionResponse &response);

    ::grpc::Status InitWatch(grpc::ClientContext *context);

    //watch ---因为watch是同步的,需要阻塞进程等事件
    //监视resp->Write(wrequest,wot)后的事件。
    ::grpc::Status Watch(::etcdserverpb::WatchResponse &response);
    
    ::grpc::Status FinishWatch();

    //只能调一次。否则tocken会重复增加。
    ::grpc::Status SetWatch(const grpc::WriteOptions &xwot,
		 const ::etcdserverpb::WatchRequest& xrequest);

    // Lease 
    //创建租约
    ::grpc::Status LeaseGrant(grpc::ClientContext *context,
		 const ::etcdserverpb::LeaseGrantRequest& request,
		 ::etcdserverpb::LeaseGrantResponse &response);

    //删除租约
    ::grpc::Status LeaseRevoke(grpc::ClientContext *context,
		 const ::etcdserverpb::LeaseRevokeRequest& request,
		 ::etcdserverpb::LeaseRevokeResponse &response);

    //保活
    ::grpc::Status LeaseKeepAlive(grpc::ClientContext *context,
		 const grpc::WriteOptions &wot,
		 const ::etcdserverpb::LeaseKeepAliveRequest &request,
		 ::etcdserverpb::LeaseKeepAliveResponse &response);

    //租约剩余时间
    ::grpc::Status LeaseTimeToLive(grpc::ClientContext *context,
		 const ::etcdserverpb::LeaseTimeToLiveRequest& request,
		 ::etcdserverpb::LeaseTimeToLiveResponse &response);

    // auth 
    //使能认证,即打开认证
    ::grpc::Status AuthEnable(grpc::ClientContext *context,
		 const ::etcdserverpb::AuthEnableRequest& request,
		 ::etcdserverpb::AuthEnableResponse &response);

    //关闭认证
    ::grpc::Status AuthDisable(grpc::ClientContext *context,
		 const ::etcdserverpb::AuthDisableRequest& request,
		 ::etcdserverpb::AuthDisableResponse &response);

    //认证用户
    ::grpc::Status Authenticate(grpc::ClientContext *context,
		 const ::etcdserverpb::AuthenticateRequest& request,
		 ::etcdserverpb::AuthenticateResponse &response);

    //添加用户
    ::grpc::Status UserAdd(grpc::ClientContext *context,
		 const ::etcdserverpb::AuthUserAddRequest& request,
		 ::etcdserverpb::AuthUserAddResponse &response);

    //获取用户信息
    ::grpc::Status UserGet(grpc::ClientContext *context,
		 const ::etcdserverpb::AuthUserGetRequest& request,
		 ::etcdserverpb::AuthUserGetResponse &response);

    //返回所有用户列表
    ::grpc::Status UserList(grpc::ClientContext *context,
		 const ::etcdserverpb::AuthUserListRequest& request,
		 ::etcdserverpb::AuthUserListResponse &response);

    //删除用户
    ::grpc::Status UserDelete(grpc::ClientContext *context,
		 const ::etcdserverpb::AuthUserDeleteRequest& request,
		 ::etcdserverpb::AuthUserDeleteResponse &response);

    //修改用户密码
    ::grpc::Status UserChangePassword(grpc::ClientContext *context,
		 const ::etcdserverpb::AuthUserChangePasswordRequest& request,
		 ::etcdserverpb::AuthUserChangePasswordResponse &response);

    //添加用户的角色
    ::grpc::Status UserGrantRole(grpc::ClientContext *context,
		 const ::etcdserverpb::AuthUserGrantRoleRequest& request,
		 ::etcdserverpb::AuthUserGrantRoleResponse &response);

    //取消用户的角色
    ::grpc::Status UserRevokeRole(grpc::ClientContext *context,
		 const ::etcdserverpb::AuthUserRevokeRoleRequest& request,
		 ::etcdserverpb::AuthUserRevokeRoleResponse &response);

    //创建角色
    ::grpc::Status RoleAdd(grpc::ClientContext *context,
		 const ::etcdserverpb::AuthRoleAddRequest& request,
		 ::etcdserverpb::AuthRoleAddResponse &response);

    //返回某角色的信息
    ::grpc::Status RoleGet(grpc::ClientContext *context,
		 const ::etcdserverpb::AuthRoleGetRequest& request,
		 ::etcdserverpb::AuthRoleGetResponse &response);

    //列出所有角色
    ::grpc::Status RoleList(grpc::ClientContext *context,
		 const ::etcdserverpb::AuthRoleListRequest& request,
		 ::etcdserverpb::AuthRoleListResponse &response);

    //删除角色,之前关联了此角色的用户,用户的角色也被删除
    ::grpc::Status RoleDelete(grpc::ClientContext *context,
		 const ::etcdserverpb::AuthRoleDeleteRequest& request,
		 ::etcdserverpb::AuthRoleDeleteResponse &response);

    //授权权限,作用对象为 角色
    ::grpc::Status RoleGrantPermission(grpc::ClientContext *context,
		 const ::etcdserverpb::AuthRoleGrantPermissionRequest& request,
		 ::etcdserverpb::AuthRoleGrantPermissionResponse &response);

    //取消权限
    ::grpc::Status RoleRevokePermission(grpc::ClientContext *context,
		 const ::etcdserverpb::AuthRoleRevokePermissionRequest& request,
		 ::etcdserverpb::AuthRoleRevokePermissionResponse &response);

    // cluster 
    //增加一个etcd实例,并需要手动启动相关实例。
    ::grpc::Status MemberAdd(grpc::ClientContext *context,
		 const ::etcdserverpb::MemberAddRequest& request,
		 ::etcdserverpb::MemberAddResponse &response);

    //删除一个etcd实例
    ::grpc::Status MemberRemove(grpc::ClientContext *context,
		 const ::etcdserverpb::MemberRemoveRequest& request,
		 ::etcdserverpb::MemberRemoveResponse &response);

    //返回所有etcd实例
    ::grpc::Status MemberList(grpc::ClientContext *context,
		 const ::etcdserverpb::MemberListRequest& request,
		 ::etcdserverpb::MemberListResponse &response);

    //etcd实例更新
    ::grpc::Status MemberUpdate(grpc::ClientContext *context,
		 const ::etcdserverpb::MemberUpdateRequest& request,
		 ::etcdserverpb::MemberUpdateResponse &response);

    //Maintenance
    //对etcd的状态（NOSPACE的告警）执行相关处理（action）,（NOSPACE的告警）,对群集影响较big.
     ::grpc::Status Alarm(grpc::ClientContext *context,
		 const ::etcdserverpb::AlarmRequest& request,
		 ::etcdserverpb::AlarmResponse &response);

    //etcd状态。如版本,raftterm,数据size,leader id.
    ::grpc::Status Status(grpc::ClientContext *context,
		 const ::etcdserverpb::StatusRequest& request,
		 ::etcdserverpb::StatusResponse &response);
    
    //碎片整理
    ::grpc::Status Defragment(grpc::ClientContext *context,
		 const ::etcdserverpb::DefragmentRequest& request,
		 ::etcdserverpb::DefragmentResponse &response);

    // Hash returns the hash of the local KV state for consistency checking purpose.
    // This is designed for testing; do not use this in production when there
    ::grpc::Status Hash(grpc::ClientContext *context,
		 const ::etcdserverpb::HashRequest& request,
		 ::etcdserverpb::HashResponse &response);

    //快照
    ::grpc::Status Snapshot(grpc::ClientContext *context,
		 const ::etcdserverpb::SnapshotRequest& request,
		 ::etcdserverpb::SnapshotResponse &response);

private:
    std::shared_ptr<grpc::Channel> _grpc_channel;       //grpc通道连接
    std::unique_ptr<etcdserverpb::KV::Stub> _kv_stub;
    std::unique_ptr<etcdserverpb::Watch::Stub> _watch_stub;  
    std::unique_ptr<etcdserverpb::Lease::Stub> _lease_stub;
    std::unique_ptr<etcdserverpb::Auth::Stub> _auth_stub;
    std::unique_ptr<etcdserverpb::Cluster::Stub> _cluster_stub;  //集群操作
    std::unique_ptr<etcdserverpb::Maintenance::Stub> _maintenance_stub;  //维护操作
    std::unique_ptr<grpc::ClientReaderWriter<etcdserverpb::WatchRequest,
		 etcdserverpb::WatchResponse>> _watch_handler;
};

}
