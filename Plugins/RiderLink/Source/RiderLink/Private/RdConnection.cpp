#include "RdConnection.hpp"

#include "ProtocolFactory.h"
#include "RdEditorProtocol/UE4Library/UE4Library.h"

RdConnection::RdConnection():
    SocketLifetimeDef{rd::Lifetime::Eternal()}
    , SocketLifetime{SocketLifetimeDef.lifetime}
    , Scheduler{SocketLifetime, "UnrealEditorScheduler"}
{
}

RdConnection::~RdConnection()
{
    SocketLifetimeDef.terminate();
}

void RdConnection::Init()
{
    Protocol = ProtocolFactory::Create(&Scheduler, SocketLifetime);
    UnrealToBackendModel.connect(SocketLifetime, Protocol.Get());
    Jetbrains::EditorPlugin::UE4Library::serializersOwner.registerSerializersCore(
        UnrealToBackendModel.get_serialization_context().get_serializers()
    );
}

void RdConnection::Shutdown()
{
    SocketLifetimeDef.terminate();
}
