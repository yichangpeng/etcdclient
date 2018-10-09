#include <etcd_comm.h>
#include <etcd_client_impl.h>

namespace etcd{

EtcdClientImpl::EtcdClientImpl(std::shared_ptr<grpc::Channel> channel)
    : _grpc_channel(channel),
    _kv_stub(etcdserverpb::KV::NewStub(channel)),
    _lease_stub(etcdserverpb::Lease::NewStub(channel)),
    _auth_stub(etcdserverpb::Auth::NewStub(channel)),
    _cluster_stub(etcdserverpb::Cluster::NewStub(channel)),
    _maintenance_stub(etcdserverpb::Maintenance::NewStub(channel))
{
}

EtcdClientImpl::~EtcdClientImpl()
{
}

::grpc::Status EtcdClientImpl::ConnectStatus()
{
    grpc_connectivity_state cstatus = _grpc_channel->GetState(true);

    //连接刚建立，其状态为GRPC_CHANNEL_IDLE，这个状态不能表示 链接一定成功了，可能未建立成功，需要有数据操作后才知道其是否连接成功。
    //一定是连接的情况
    if (GRPC_CHANNEL_CONNECTING != cstatus && GRPC_CHANNEL_READY != cstatus)
    {
        return grpc::Status(grpc::StatusCode::INTERNAL,
		"channel not connect");
    }

    return ::grpc::Status();
}

// 查询key.
::grpc::Status EtcdClientImpl::Range(grpc::ClientContext *context,
		const etcdserverpb::RangeRequest &request,
		::etcdserverpb::RangeResponse &response) {
    assert(NULL != context);

    ::grpc::Status status = _kv_stub->Range(context,
		request,
		&response);
    return status;
}

//创建 更新 key
::grpc::Status EtcdClientImpl::Put(grpc::ClientContext *context,
		const etcdserverpb::PutRequest &request,
		::etcdserverpb::PutResponse &response) {
    assert(NULL != context);
    
    ::grpc::Status status = _kv_stub->Put(context,
		request,
		&response);
    return status;
}

//删除key
::grpc::Status EtcdClientImpl::DeleteRange(grpc::ClientContext *context,
		const etcdserverpb::DeleteRangeRequest &request,
		::etcdserverpb::DeleteRangeResponse &response) {
    assert(NULL != context);
    
    ::grpc::Status status = _kv_stub->DeleteRange(context,
		request,
		&response);
    return status;
}

//事务
::grpc::Status EtcdClientImpl::Txn(grpc::ClientContext *context,
		const ::etcdserverpb::TxnRequest& request,
		::etcdserverpb::TxnResponse &response) {
    assert(NULL != context); 
    
    ::grpc::Status status = _kv_stub->Txn(context,
		request,
		&response);
    return status;
}

//压缩
::grpc::Status EtcdClientImpl::Compact(grpc::ClientContext *context,
		const ::etcdserverpb::CompactionRequest& request,
		::etcdserverpb::CompactionResponse &response) {
    assert(NULL != context);

    ::grpc::Status status = _kv_stub->Compact(context,
		request,
		&response);
    return status;
}

::grpc::Status EtcdClientImpl::InitWatch(grpc::ClientContext *context){
    assert(NULL != context);

    _watch_handler.reset();
    _watch_stub.reset();

   _watch_stub = etcdserverpb::Watch::NewStub(_grpc_channel);
   _watch_handler = _watch_stub->Watch(context);

   return ::grpc::Status();
}

::grpc::Status EtcdClientImpl::SetWatch(const grpc::WriteOptions &xwot,
		const ::etcdserverpb::WatchRequest& xrequest){
    if (!_watch_handler->Write(xrequest,xwot))
    { 
        std::cerr << "set_watch error!" << std::endl;
        grpc::Status tstatus(grpc::StatusCode::UNKNOWN,
		            "Write request error!");
        return tstatus;
    }

    _watch_handler->WritesDone();
    return grpc::Status();
}

::grpc::Status EtcdClientImpl::Watch(::etcdserverpb::WatchResponse &response){
    if (!_watch_handler.get())
    {
        std::cerr << "null _watch_handler!" << std::endl;
        return ::grpc::Status(grpc::StatusCode::UNKNOWN,"null _watch_handler!");
    }

    if (!_watch_handler->Read(&response))
    {
        std::cerr << "Read response error!" << std::endl;
        return grpc::Status(grpc::StatusCode::UNKNOWN,"Read response error!");
    }

    return ::grpc::Status();
}

::grpc::Status EtcdClientImpl::FinishWatch(){

    if (!_watch_handler.get())
    {
        return ::grpc::Status(grpc::StatusCode::UNKNOWN,"null _watch_handler!");
    }

    return _watch_handler->Finish();
}

//创建租约
::grpc::Status EtcdClientImpl::LeaseGrant(grpc::ClientContext *context,
		const ::etcdserverpb::LeaseGrantRequest& request,
		::etcdserverpb::LeaseGrantResponse &response) {
    assert(NULL != context); 

    ::grpc::Status status = _lease_stub->LeaseGrant(context,
		request,
		&response);
    return status;
}

//删除租约
::grpc::Status EtcdClientImpl::LeaseRevoke(grpc::ClientContext *context,
		const ::etcdserverpb::LeaseRevokeRequest& request,
		::etcdserverpb::LeaseRevokeResponse &response) {
    assert(NULL != context); 

    ::grpc::Status status = _lease_stub->LeaseRevoke(context,
		request,
		&response);
    return status;
}

//保活
::grpc::Status EtcdClientImpl::LeaseKeepAlive(grpc::ClientContext *context,
		const grpc::WriteOptions &wot,
		const ::etcdserverpb::LeaseKeepAliveRequest &request,
		::etcdserverpb::LeaseKeepAliveResponse &response) {
    assert(NULL != context); 

    std::unique_ptr< ::grpc::ClientReaderWriterInterface< ::etcdserverpb::LeaseKeepAliveRequest,
		::etcdserverpb::LeaseKeepAliveResponse>> grep = _lease_stub->LeaseKeepAlive(context);

    bool ok = false;
    ok = grep->Write(request,
		wot);
    if (!ok)
    {
        return grpc::Status(grpc::StatusCode::UNKNOWN,"Write request error!");
    }

    ok = grep->Read(&response);
    if (!ok)
    {
        return grpc::Status(grpc::StatusCode::UNKNOWN,"Read response error!");
    }

    return ::grpc::Status();
}

//租约剩余时间
::grpc::Status EtcdClientImpl::LeaseTimeToLive(grpc::ClientContext *context,
		const ::etcdserverpb::LeaseTimeToLiveRequest& request,
		::etcdserverpb::LeaseTimeToLiveResponse &response) {
    assert(NULL != context); 

    ::grpc::Status status = _lease_stub->LeaseTimeToLive(context,
		request,
		&response);
    return status;
}

// auth 
//使能认证，即打开认证
::grpc::Status EtcdClientImpl::AuthEnable(grpc::ClientContext *context,
		const ::etcdserverpb::AuthEnableRequest& request,
		::etcdserverpb::AuthEnableResponse &response) {
    assert(NULL != context); 

    ::grpc::Status status = _auth_stub->AuthEnable(context,
		request,
		&response);
    return status;
}

//关闭认证
::grpc::Status EtcdClientImpl::AuthDisable(grpc::ClientContext *context,
		const ::etcdserverpb::AuthDisableRequest& request,
		::etcdserverpb::AuthDisableResponse &response) {
    assert(NULL != context); 

    ::grpc::Status status = _auth_stub->AuthDisable(context,
		request,
		&response);

    return status;
}

//认证用户
::grpc::Status EtcdClientImpl::Authenticate(grpc::ClientContext *context,
		const ::etcdserverpb::AuthenticateRequest& request,
		::etcdserverpb::AuthenticateResponse &response) {
    assert(NULL != context); 

    ::grpc::Status status = _auth_stub->Authenticate(context,
		request,
		&response);
    if(status.ok())
    {
        context->AddMetadata(KEY_TOKEN,
		response.token());
    }
    return status;
}

//添加用户
::grpc::Status EtcdClientImpl::UserAdd(grpc::ClientContext *context,
		const ::etcdserverpb::AuthUserAddRequest& request,
		::etcdserverpb::AuthUserAddResponse &response) {
    assert(NULL != context); 

    ::grpc::Status status = _auth_stub->UserAdd(context,
		request,
		&response);
    return status;
}

//获取用户信息
::grpc::Status EtcdClientImpl::UserGet(grpc::ClientContext *context,
		const ::etcdserverpb::AuthUserGetRequest& request,
		::etcdserverpb::AuthUserGetResponse &response) {
    assert(NULL != context); 

    ::grpc::Status status = _auth_stub->UserGet(context,
		request,
		&response);
    return status;
}

//返回所有用户列表
::grpc::Status EtcdClientImpl::UserList(grpc::ClientContext *context,
		const ::etcdserverpb::AuthUserListRequest& request,
		::etcdserverpb::AuthUserListResponse &response) {
    assert(NULL != context); 

    ::grpc::Status status = _auth_stub->UserList(context,
		request,
		&response);
    return status;
}

//删除用户
::grpc::Status EtcdClientImpl::UserDelete(grpc::ClientContext *context,
		const ::etcdserverpb::AuthUserDeleteRequest& request,
		::etcdserverpb::AuthUserDeleteResponse &response) {
    assert(NULL != context); 

    ::grpc::Status status = _auth_stub->UserDelete(context,
		request,
		&response);
    return status;
}

//修改用户密码
::grpc::Status EtcdClientImpl::UserChangePassword(grpc::ClientContext *context,
		const ::etcdserverpb::AuthUserChangePasswordRequest& request,
		::etcdserverpb::AuthUserChangePasswordResponse &response) {
    assert(NULL != context); 

    ::grpc::Status status = _auth_stub->UserChangePassword(context,
		request,
		&response);
    return status;
}

//添加用户的角色
::grpc::Status EtcdClientImpl::UserGrantRole(grpc::ClientContext *context,
		const ::etcdserverpb::AuthUserGrantRoleRequest& request,
		::etcdserverpb::AuthUserGrantRoleResponse &response) {
    assert(NULL != context); 

    ::grpc::Status status = _auth_stub->UserGrantRole(context,
		request,
		&response);
    return status;
}

//取消用户的角色
::grpc::Status EtcdClientImpl::UserRevokeRole(grpc::ClientContext *context,
		const ::etcdserverpb::AuthUserRevokeRoleRequest& request,
		::etcdserverpb::AuthUserRevokeRoleResponse &response) {
    assert(NULL != context); 

    ::grpc::Status status = _auth_stub->UserRevokeRole(context,
		request,
		&response);
    return status;
}

//创建角色
::grpc::Status EtcdClientImpl::RoleAdd(grpc::ClientContext *context,
		const ::etcdserverpb::AuthRoleAddRequest& request,
		::etcdserverpb::AuthRoleAddResponse &response) {
    assert(NULL != context); 

    ::grpc::Status status = _auth_stub->RoleAdd(context,
		request,
		&response);
    return status;
}

//返回某角色的信息
::grpc::Status EtcdClientImpl::RoleGet(grpc::ClientContext *context,
		const ::etcdserverpb::AuthRoleGetRequest& request,
		::etcdserverpb::AuthRoleGetResponse &response) {
    assert(NULL != context); 

    ::grpc::Status status = _auth_stub->RoleGet(context,
		request,
		&response);
    return status;
}

//列出所有角色
::grpc::Status EtcdClientImpl::RoleList(grpc::ClientContext *context,
		const ::etcdserverpb::AuthRoleListRequest& request,
		::etcdserverpb::AuthRoleListResponse &response) {
    assert(NULL != context); 

    ::grpc::Status status = _auth_stub->RoleList(context,
		request,
		&response);
    return status;
}

//删除角色，之前关联了此角色的用户，用户的角色也被删除
::grpc::Status EtcdClientImpl::RoleDelete(grpc::ClientContext *context,
		const ::etcdserverpb::AuthRoleDeleteRequest& request,
		::etcdserverpb::AuthRoleDeleteResponse &response) {
    assert(NULL != context); 

    ::grpc::Status status = _auth_stub->RoleDelete(context,
		request,
		&response);
    return status;
}

//授权权限，作用对象为 角色
::grpc::Status EtcdClientImpl::RoleGrantPermission(grpc::ClientContext *context,
		const ::etcdserverpb::AuthRoleGrantPermissionRequest& request,
		::etcdserverpb::AuthRoleGrantPermissionResponse &response) {
    assert(NULL != context); 

    ::grpc::Status status = _auth_stub->RoleGrantPermission(context,
		request,
		&response);
    return status;
}

//取消权限
::grpc::Status EtcdClientImpl::RoleRevokePermission(grpc::ClientContext *context,
		const ::etcdserverpb::AuthRoleRevokePermissionRequest& request,
		::etcdserverpb::AuthRoleRevokePermissionResponse &response) {
    assert(NULL != context); 

    ::grpc::Status status = _auth_stub->RoleRevokePermission(context,
		request,
		&response);
    return status;
}

// cluster 
//增加一个etcd实例，并需要手动启动相关实例。
::grpc::Status EtcdClientImpl::MemberAdd(grpc::ClientContext *context,
		const ::etcdserverpb::MemberAddRequest& request,
		::etcdserverpb::MemberAddResponse &response) {
    assert(NULL != context); 

    ::grpc::Status status = _cluster_stub->MemberAdd(context,
		request,
		&response);
    return status;
}

//删除一个etcd实例
::grpc::Status EtcdClientImpl::MemberRemove(grpc::ClientContext *context,
		const ::etcdserverpb::MemberRemoveRequest& request,
		::etcdserverpb::MemberRemoveResponse &response) {
    assert(NULL != context); 

    ::grpc::Status status = _cluster_stub->MemberRemove(context,
		request,
		&response);
    return status;
}

//返回所有etcd实例
::grpc::Status EtcdClientImpl::MemberList(grpc::ClientContext *context,
		const ::etcdserverpb::MemberListRequest& request,
		::etcdserverpb::MemberListResponse &response) {
    assert(NULL != context); 

    ::grpc::Status status = _cluster_stub->MemberList(context,
		request,
		&response);
    return status;
}

//etcd实例更新
::grpc::Status EtcdClientImpl::MemberUpdate(grpc::ClientContext *context,
		const ::etcdserverpb::MemberUpdateRequest& request,
		::etcdserverpb::MemberUpdateResponse &response) {
    assert(NULL != context); 

    ::grpc::Status status = _cluster_stub->MemberUpdate(context,
		request,
		&response);
    return status;
}

//Maintenance
//对etcd的状态（NOSPACE的告警）执行相关处理（action），（NOSPACE的告警），对群集影响较big.
::grpc::Status EtcdClientImpl::Alarm(grpc::ClientContext *context,
		const ::etcdserverpb::AlarmRequest& request,
		::etcdserverpb::AlarmResponse &response) {
    assert(NULL != context); 

    ::grpc::Status status = _maintenance_stub->Alarm(context,
		request,
		&response);
    return status;
}

//etcd状态。如版本，raftterm，数据size，leader id.
::grpc::Status EtcdClientImpl::Status(grpc::ClientContext *context,
		const ::etcdserverpb::StatusRequest& request,
		::etcdserverpb::StatusResponse &response) {
    assert(NULL != context); 

    ::grpc::Status status = _maintenance_stub->Status(context,
		request,
		&response);
    return status;
}

//碎片整理
::grpc::Status EtcdClientImpl::Defragment(grpc::ClientContext *context,
		const ::etcdserverpb::DefragmentRequest& request,
		::etcdserverpb::DefragmentResponse &response) {
    assert(NULL != context); 

    ::grpc::Status status = _maintenance_stub->Defragment(context,
		request,
		&response);
    return status;
}

// Hash returns the hash of the local KV state for consistency checking purpose.
// This is designed for testing; do not use this in production when there
::grpc::Status EtcdClientImpl::Hash(grpc::ClientContext *context,
		const ::etcdserverpb::HashRequest& request,
		::etcdserverpb::HashResponse &response) {
    assert(NULL != context); 

    ::grpc::Status status = _maintenance_stub->Hash(context,
		request,
		&response);
    return status;
}

//快照
::grpc::Status EtcdClientImpl::Snapshot(grpc::ClientContext *context,
		const ::etcdserverpb::SnapshotRequest& request,
		::etcdserverpb::SnapshotResponse &response) {
    assert(NULL != context); 
    ::grpc::Status tstatus;
    std::unique_ptr< ::grpc::ClientReader< ::etcdserverpb::SnapshotResponse>> grep = _maintenance_stub->Snapshot(context,
		request);

    bool ok = grep->Read(&response);
    if (!ok)
    {
        return grpc::Status(grpc::StatusCode::UNKNOWN,
		"Read response error!");
    }

    return grpc::Status();
}

}
