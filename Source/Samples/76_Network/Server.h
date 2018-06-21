//
// Copyright (c) 2008-2016 the Urho3D project.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#pragma once

#include <Urho3D/Core/Object.h>
#include <Urho3D/Input/Controls.h>

namespace Urho3D
{
class Scene;
class Connection;
}

using namespace Urho3D;
//=============================================================================
//=============================================================================
class ClientObj;

//=============================================================================
//=============================================================================
// UDP port we will use
const unsigned short SERVER_PORT = 2345;

//=============================================================================
//=============================================================================
URHO3D_EVENT(E_SERVERSTATUS, ServerStatus)
{
	URHO3D_PARAM(P_STATUS, Status); // unsigned
}

URHO3D_EVENT(E_CLIENTOBJECTID, ClientObjectID)
{
	URHO3D_PARAM(P_ID, ID);         // unsigned
}

//=============================================================================
//=============================================================================
class Server : public Object
{
    URHO3D_OBJECT(Server, Object);
public:

    Server(Context* context);
    virtual ~Server();
    
    void RegisterClientHashAndScene(StringHash clientHash, Scene *scene);
    bool StartServer(unsigned short port);
    bool Connect(const String &address, unsigned short port, const VariantMap& identity = Variant::emptyVariantMap);
    void Disconnect();

    Node* CreateClientObject(Connection *connection);
    void UpdatePhysicsPreStep(const Controls &controls);

protected:
    void SubscribeToEvents();
    void SendStatusMsg(StringHash msg);

    /// Handle the physics world pre-step event.
    void HandlePhysicsPreStep(StringHash eventType, VariantMap& eventData);
    /// Handle connection status change (just update the buttons that should be shown.)
    void HandleConnectionStatus(StringHash eventType, VariantMap& eventData);
    /// Handle a client connecting to the server.
    void HandleClientConnected(StringHash eventType, VariantMap& eventData);
    /// Handle a client disconnecting from the server.
    void HandleClientDisconnected(StringHash eventType, VariantMap& eventData);
    /// Handle remote event from server which tells our controlled object node ID.
    void HandleNetworkUpdateSent(StringHash eventType, VariantMap& eventData);
    void HandleClientObjectID(StringHash eventType, VariantMap& eventData);
    void HandleClientIdentity(StringHash eventType, VariantMap& eventData);
    void HandleClientSceneLoaded(StringHash eventType, VariantMap& eventData);

protected:
    /// Mapping from client connections to controllable objects.
    HashMap<Connection*, WeakPtr<Node> > serverObjects_;
    StringHash clientHash_;
    unsigned clientObjectID_;
    SharedPtr<Scene> scene_;
};
