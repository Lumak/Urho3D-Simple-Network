//
// Copyright (c) 2008-2017 the Urho3D project.
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

#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Graphics/Zone.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Input/Controls.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/Network/Network.h>
#include <Urho3D/Network/Connection.h>
#include <Urho3D/Network/NetworkEvents.h>
#include <Urho3D/Physics/PhysicsEvents.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Graphics/DebugRenderer.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/UI/Button.h>
#include <Urho3D/UI/Font.h>
#include <Urho3D/UI/LineEdit.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/Text3D.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/UIEvents.h>

#include "SceneReplication.h"
#include "Server.h"
#include "ClientObj.h"
#include "Baller.h"

#include <Urho3D/DebugNew.h>
//=============================================================================
//=============================================================================
URHO3D_DEFINE_APPLICATION_MAIN(SceneReplication)

SceneReplication::SceneReplication(Context* context) :
    Sample(context),
    clientObjectID_(0),
    isServer_(false),
    drawDebug_(false)
{
}

void SceneReplication::Setup()
{
    engineParameters_["WindowTitle"]   = GetTypeName();
    engineParameters_["LogName"]       = GetSubsystem<FileSystem>()->GetProgramDir() + "netdemo.log";
    engineParameters_["FullScreen"]    = false;
    engineParameters_["Headless"]      = false;
    engineParameters_["WindowWidth"]   = 1280; 
    engineParameters_["WindowHeight"]  = 720;
    engineParameters_["ResourcePaths"] = "Data;CoreData;Data/NetDemo;";
}

void SceneReplication::Start()
{
    Sample::Start();

    // rand seed
    SetRandomSeed(Time::GetSystemTime());

    CreateServerSubsystem();

    CreateScene();

    CreateUI();

    SetupViewport();

    SubscribeToEvents();

    ChangeDebugHudText();

    Sample::InitMouseMode(MM_RELATIVE);
}

void SceneReplication::CreateServerSubsystem()
{
    context_->RegisterSubsystem(new Server(context_));

    // register client objs
    ClientObj::RegisterObject(context_);
    Baller::RegisterObject(context_);
}

void SceneReplication::CreateScene()
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();

    scene_ = new Scene(context_);

    // server requires client hash and scene info
    Server *server = GetSubsystem<Server>();
    server->RegisterClientHashAndScene(Baller::GetTypeStatic(), scene_);

    // Create octree and physics world with default settings. Create them as local so that they are not needlessly replicated
    // when a client connects
    scene_->CreateComponent<Octree>(LOCAL);
    PhysicsWorld *physicsWorld = scene_->CreateComponent<PhysicsWorld>(LOCAL);
    DebugRenderer *dbgRenderer = scene_->CreateComponent<DebugRenderer>(LOCAL);
    physicsWorld->SetDebugRenderer(dbgRenderer);

    // All static scene content and the camera are also created as local, so that they are unaffected by scene replication and are
    // not removed from the client upon connection. Create a Zone component first for ambient lighting & fog control.
    Node* zoneNode = scene_->CreateChild("Zone", LOCAL);
    Zone* zone = zoneNode->CreateComponent<Zone>();
    zone->SetBoundingBox(BoundingBox(-1000.0f, 1000.0f));
    zone->SetAmbientColor(Color(0.5f, 0.5f, 0.5f));
    zone->SetFogColor(Color(0.3f, 0.3f, 0.5f));
    zone->SetFogStart(100.0f);
    zone->SetFogEnd(300.0f);

    // Create a directional light with shadows
    Node* lightNode = scene_->CreateChild("DirectionalLight", LOCAL);
    lightNode->SetDirection(Vector3(0.3f, -0.5f, 0.425f));
    Light* light = lightNode->CreateComponent<Light>();
    light->SetLightType(LIGHT_DIRECTIONAL);
    light->SetCastShadows(true);
    light->SetShadowBias(BiasParameters(0.00025f, 0.5f));
    light->SetShadowCascade(CascadeParameters(10.0f, 50.0f, 200.0f, 0.0f, 0.8f));
    light->SetSpecularIntensity(0.5f);

    // Create a "floor" consisting of several tiles. Make the tiles physical but leave small cracks between them
    Node* floorNode = scene_->CreateChild("floor", LOCAL);
    StaticModel* floor = floorNode->CreateComponent<StaticModel>();
    Model *model = cache->GetResource<Model>("NetDemo/level1.mdl");
    floor->SetModel(model);
    floor->SetMaterial(cache->GetResource<Material>("NetDemo/groundMat.xml"));
    floor->SetCastShadows(true);
    floorNode->CreateComponent<RigidBody>();
    CollisionShape* shape = floorNode->CreateComponent<CollisionShape>();
    shape->SetTriangleMesh(model);

    // cam
    cameraNode_ = scene_->CreateChild("Camera", LOCAL);
    Camera* camera = cameraNode_->CreateComponent<Camera>();
    camera->SetFarClip(300.0f);
    cameraNode_->SetPosition(Vector3(0.0f, 5.0f, 0.0f));
}

void SceneReplication::CreateAdminPlayer()
{
    Node* clientNode = scene_->CreateChild("Admin");
    clientNode->SetPosition(Vector3(Random(40.0f) - 20.0f, 5.0f, Random(40.0f) - 20.0f));

    ClientObj *clientObj = (ClientObj*)clientNode->CreateComponent(Baller::GetTypeStatic());

    // set identity
    clientObj->SetClientInfo("ADMIN", 99);
    clientObjectID_ = clientNode->GetID();
    isServer_ = true;
}

void SceneReplication::CreateUI()
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    UI* ui = GetSubsystem<UI>();
    UIElement* root = ui->GetRoot();
    XMLFile* uiStyle = cache->GetResource<XMLFile>("UI/DefaultStyle.xml");
    root->SetDefaultStyle(uiStyle);

    SharedPtr<Cursor> cursor(new Cursor(context_));
    cursor->SetStyleAuto(uiStyle);
    ui->SetCursor(cursor);
    Graphics* graphics = GetSubsystem<Graphics>();
    cursor->SetPosition(graphics->GetWidth() / 2, graphics->GetHeight() / 2);

    // Construct the instructions text element
    instructionsText_ = ui->GetRoot()->CreateChild<Text>();
    instructionsText_->SetText(
        "WASD to move, RMB to rotate view, T to swap mat"
    );
    instructionsText_->SetFont(cache->GetResource<Font>("Fonts/Anonymous Pro.ttf"), 12);
    instructionsText_->SetColor(Color::CYAN);
    // Position the text relative to the screen center
    instructionsText_->SetHorizontalAlignment(HA_CENTER);
    instructionsText_->SetPosition(0, 20);
    // Hide until connected
    instructionsText_->SetVisible(false);

    buttonContainer_ = root->CreateChild<UIElement>();
    buttonContainer_->SetFixedSize(500, 20);
    buttonContainer_->SetPosition(20, 20);
    buttonContainer_->SetLayoutMode(LM_HORIZONTAL);

    textEdit_ = buttonContainer_->CreateChild<LineEdit>();
    textEdit_->SetStyleAuto();

    connectButton_ = CreateButton("Connect", 90);
    disconnectButton_ = CreateButton("Disconnect", 130);
    startServerButton_ = CreateButton("Start Server", 110);

    UpdateButtons();
}

void SceneReplication::SetupViewport()
{
    Renderer* renderer = GetSubsystem<Renderer>();

    // Set up a viewport to the Renderer subsystem so that the 3D scene can be seen
    SharedPtr<Viewport> viewport(new Viewport(context_, scene_, cameraNode_->GetComponent<Camera>()));
    renderer->SetViewport(0, viewport);
}

void SceneReplication::ChangeDebugHudText()
{
    // change profiler text
    if (GetSubsystem<DebugHud>())
    {
        Text *dbgText = GetSubsystem<DebugHud>()->GetProfilerText();
        dbgText->SetColor(Color::CYAN);
        dbgText->SetTextEffect(TE_NONE);

        dbgText = GetSubsystem<DebugHud>()->GetStatsText();
        dbgText->SetColor(Color::CYAN);
        dbgText->SetTextEffect(TE_NONE);

        dbgText = GetSubsystem<DebugHud>()->GetMemoryText();
        dbgText->SetColor(Color::CYAN);
        dbgText->SetTextEffect(TE_NONE);

        dbgText = GetSubsystem<DebugHud>()->GetModeText();
        dbgText->SetColor(Color::CYAN);
        dbgText->SetTextEffect(TE_NONE);
    }
}

void SceneReplication::SubscribeToEvents()
{
    SubscribeToEvent(E_PHYSICSPRESTEP, URHO3D_HANDLER(SceneReplication, HandlePhysicsPreStep));
    SubscribeToEvent(E_POSTUPDATE, URHO3D_HANDLER(SceneReplication, HandlePostUpdate));

    // Subscribe to button actions
    SubscribeToEvent(connectButton_, E_RELEASED, URHO3D_HANDLER(SceneReplication, HandleConnect));
    SubscribeToEvent(disconnectButton_, E_RELEASED, URHO3D_HANDLER(SceneReplication, HandleDisconnect));
    SubscribeToEvent(startServerButton_, E_RELEASED, URHO3D_HANDLER(SceneReplication, HandleStartServer));

    // Subscribe to server events
    SubscribeToEvent(E_SERVERSTATUS, URHO3D_HANDLER(SceneReplication, HandleConnectionStatus));
    SubscribeToEvent(E_CLIENTOBJECTID, URHO3D_HANDLER(SceneReplication, HandleClientObjectID));
}

Button* SceneReplication::CreateButton(const String& text, int width)
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    Font* font = cache->GetResource<Font>("Fonts/Anonymous Pro.ttf");

    Button* button = buttonContainer_->CreateChild<Button>();
    button->SetStyleAuto();
    button->SetFixedWidth(width);

    Text* buttonText = button->CreateChild<Text>();
    buttonText->SetName("text");
    buttonText->SetFont(font, 12);
    buttonText->SetAlignment(HA_CENTER, VA_CENTER);
    buttonText->SetText(text);

    return button;
}

void SceneReplication::UpdateButtons()
{
    Network* network = GetSubsystem<Network>();
    Connection* serverConnection = network->GetServerConnection();
    bool serverRunning = network->IsServerRunning();

    // Show and hide buttons so that eg. Connect and Disconnect are never shown at the same time
    connectButton_->SetVisible(!serverConnection && !serverRunning);
    disconnectButton_->SetVisible(serverConnection || serverRunning);
    Text* discText = disconnectButton_->GetChildStaticCast<Text>(String("text"));
    if (serverConnection)
    {
        discText->SetText("cli Disconnect");
    }
    else if (serverRunning)
    {
        discText->SetText("svr Disconnect");
    }
    startServerButton_->SetVisible(!serverConnection && !serverRunning);
    textEdit_->SetVisible(!serverConnection && !serverRunning);
}

void SceneReplication::MoveCamera()
{
    // Right mouse button controls mouse cursor visibility: hide when pressed
    UI* ui = GetSubsystem<UI>();
    Input* input = GetSubsystem<Input>();
    ui->GetCursor()->SetVisible(!input->GetMouseButtonDown(MOUSEB_RIGHT));

    // Mouse sensitivity as degrees per pixel
    const float MOUSE_SENSITIVITY = 0.1f;

    // Use this frame's mouse motion to adjust camera node yaw and pitch. Clamp the pitch and only move the camera
    // when the cursor is hidden
    if (!ui->GetCursor()->IsVisible())
    {
        IntVector2 mouseMove = input->GetMouseMove();
        yaw_ += MOUSE_SENSITIVITY * mouseMove.x_;
        pitch_ += MOUSE_SENSITIVITY * mouseMove.y_;
        pitch_ = Clamp(pitch_, 1.0f, 90.0f);
    }

    // Construct new orientation for the camera scene node from yaw and pitch. Roll is fixed to zero
    cameraNode_->SetRotation(Quaternion(pitch_, yaw_, 0.0f));

    // Only move the camera / show instructions if we have a controllable object
    bool showInstructions = false;
    if (clientObjectID_)
    {
        Node* ballNode = scene_->GetNode(clientObjectID_);
        if (ballNode)
        {
            const float CAMERA_DISTANCE = 5.0f;

            Vector3 startPos = ballNode->GetPosition();
            Vector3 destPos = ballNode->GetPosition() + cameraNode_->GetRotation() * Vector3::BACK * CAMERA_DISTANCE;
            Vector3 seg = destPos - startPos;
            Ray cameraRay(startPos, seg.Normalized());
            float cameraRayLength = seg.Length();
            PhysicsRaycastResult result;
            scene_->GetComponent<PhysicsWorld>()->SphereCast(result, cameraRay, 0.2f, cameraRayLength, ~BALLER_COL_LAYER);
            if (result.body_)
                destPos = startPos + cameraRay.direction_ * result.distance_;

            // Move camera some distance away from the ball
            cameraNode_->SetPosition(destPos);
            showInstructions = true;
        }
    }

    instructionsText_->SetVisible(showInstructions);
}

void SceneReplication::HandlePostUpdate(StringHash eventType, VariantMap& eventData)
{
    // We only rotate the camera according to mouse movement since last frame, so do not need the time step
    MoveCamera();

    if (drawDebug_)
    {
        scene_->GetComponent<PhysicsWorld>()->DrawDebugGeometry(true);
    }
}

void SceneReplication::HandlePhysicsPreStep(StringHash eventType, VariantMap& eventData)
{
    Server *server = GetSubsystem<Server>();
    Input *input = GetSubsystem<Input>();

    // set controls and pos
    Controls controls;
    controls.yaw_ = yaw_;

    controls.Set(CTRL_FORWARD, input->GetKeyDown(KEY_W));
    controls.Set(CTRL_BACK, input->GetKeyDown(KEY_S));
    controls.Set(CTRL_LEFT, input->GetKeyDown(KEY_A));
    controls.Set(CTRL_RIGHT, input->GetKeyDown(KEY_D));
    controls.Set(SWAP_MAT, input->GetKeyDown(KEY_T));

    server->UpdatePhysicsPreStep(controls);

    // server is not a connection but direct controller
    if (isServer_)
    {
        Node* clientNode = scene_->GetNode(clientObjectID_);

        if (clientNode)
        {
            ClientObj *clientObj = clientNode->GetDerivedComponent<ClientObj>();

            if (clientObj)
            {
                clientObj->SetControls(controls);
            }
        }
    }
}

void SceneReplication::HandleConnect(StringHash eventType, VariantMap& eventData)
{
    static const int MAX_ARRAY_SIZE = 10;
    static String colorArray[MAX_ARRAY_SIZE] =
    {
        "WHITE",
        "GRAY",
        "BLACK",
        "RED",
        "GREEN",
        "BLUE",
        "CYAN",
        "MAGENTA",
        "YELLOW",
        "VEGAS GOLD"
    };

    Server *server = GetSubsystem<Server>();
    String address = textEdit_->GetText().Trimmed();

    // randomize (or customize) client info/data
    int idx = Random(MAX_ARRAY_SIZE - 1);
    String name = colorArray[idx];
    URHO3D_LOGINFOF("client idx=%i, username=%s", idx, name.CString());

    VariantMap& identity = GetEventDataMap();
    identity["UserName"] = name;
    identity["ColorIdx"] = idx;

    server->Connect(address, SERVER_PORT, identity);

    UpdateButtons();

    isServer_ = false;
}

void SceneReplication::HandleDisconnect(StringHash eventType, VariantMap& eventData)
{
    Server *server = GetSubsystem<Server>();
    server->Disconnect();

    UpdateButtons();
}

void SceneReplication::HandleStartServer(StringHash eventType, VariantMap& eventData)
{
    Server *server = GetSubsystem<Server>();
    server->StartServer(SERVER_PORT);

    // create Admin
    CreateAdminPlayer();

    UpdateButtons();
}

void SceneReplication::HandleConnectionStatus(StringHash eventType, VariantMap& eventData)
{
    using namespace ServerStatus;
    StringHash msg = eventData[P_STATUS].GetStringHash();

    if (msg == E_SERVERDISCONNECTED)
    {
        scene_->RemoveAllChildren();
        CreateScene();
        SetupViewport();

        URHO3D_LOGINFO("server connection lost");
    }

    UpdateButtons();
}

void SceneReplication::HandleClientObjectID(StringHash eventType, VariantMap& eventData)
{
    clientObjectID_ = eventData[ClientObjectID::P_ID].GetUInt();
}
