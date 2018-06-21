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
#include <Urho3D/Core/Context.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Graphics/Light.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/UI/Text3D.h>
#include <Urho3D/UI/Font.h>
#include <Urho3D/IO/Log.h>

#include "Baller.h"

#include <Urho3D/DebugNew.h>
//=============================================================================
//=============================================================================
Baller::Baller(Context* context)
    : ClientObj(context)
    , mass_(1.0f)
{
    SetUpdateEventMask(0);
}

Baller::~Baller()
{
    if (nodeInfo_)
    {
        nodeInfo_->Remove();
    }
}

void Baller::RegisterObject(Context* context)
{
    context->RegisterFactory<Baller>();

    URHO3D_COPY_BASE_ATTRIBUTES(ClientObj);
}

void Baller::ApplyAttributes()
{
}

void Baller::DelayedStart()
{
    Create();
}

void Baller::Create()
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();

    // model
    StaticModel* ballModel = node_->GetOrCreateComponent<StaticModel>();
    ballModel->SetModel(cache->GetResource<Model>("Models/Sphere.mdl"));
    String matName = ToString("NetDemo/ballmat%i.xml", colorIdx_);
    ballModel->SetMaterial(cache->GetResource<Material>(matName));
    ballModel->SetCastShadows(true);

    // physics components
    hullBody_ = node_->GetOrCreateComponent<RigidBody>();
    hullBody_->SetCollisionLayer(BALLER_COL_LAYER);
    hullBody_->SetMass(mass_);
    hullBody_->SetFriction(1.0f);
    hullBody_->SetLinearDamping(0.5f);
    hullBody_->SetAngularDamping(0.5f);
    CollisionShape* shape = node_->GetOrCreateComponent<CollisionShape>();
    shape->SetSphere(1.0f);

    // create text3d client info node LOCALLY
    nodeInfo_ = GetScene()->CreateChild("light", LOCAL);
    nodeInfo_->SetPosition(node_->GetPosition() + Vector3(0.0f, 1.1f, 0.0f));
    Text3D *text3D = nodeInfo_->CreateComponent<Text3D>();
    text3D->SetColor(Color::GREEN);
    text3D->SetFont(cache->GetResource<Font>("Fonts/Anonymous Pro.ttf"), 12);
    text3D->SetText(userName_);
    text3D->SetFaceCameraMode(FC_ROTATE_XYZ);

    // register
    SetUpdateEventMask(USE_FIXEDUPDATE);
}

void Baller::SwapMat()
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();

    int idx = Random(MAX_MAT_COUNT);
    while (idx == colorIdx_)
    {
        idx = Random(MAX_MAT_COUNT);
    }

    // update serializable of the change
    SetAttribute("Color Index", Variant(idx));

    String matName = ToString("NetDemo/ballmat%i.xml", colorIdx_);
    StaticModel* ballModel = node_->GetComponent<StaticModel>();
    ballModel->SetMaterial(cache->GetResource<Material>(matName));
}

void Baller::FixedUpdate(float timeStep)
{
    if (!hullBody_ || !nodeInfo_)
    {
        return;
    }

    const float MOVE_TORQUE = 3.0f;
    Quaternion rotation(0.0f, controls_.yaw_, 0.0f);

    if (controls_.buttons_ & CTRL_FORWARD)
    {
        hullBody_->ApplyTorque(rotation * Vector3::RIGHT * MOVE_TORQUE);
    }
    if (controls_.buttons_ & CTRL_BACK)
    {
        hullBody_->ApplyTorque(rotation * Vector3::LEFT * MOVE_TORQUE);
    }
    if (controls_.buttons_ & CTRL_LEFT)
    {
        hullBody_->ApplyTorque(rotation * Vector3::FORWARD * MOVE_TORQUE);
    }
    if (controls_.buttons_ & CTRL_RIGHT)
    {
        hullBody_->ApplyTorque(rotation * Vector3::BACK * MOVE_TORQUE);
    }

    if (controls_.IsPressed(SWAP_MAT, prevControls_))
    {
        SwapMat();
    }

    // update prev
    prevControls_ = controls_;

    // update text pos
    nodeInfo_->SetPosition(node_->GetPosition() + Vector3(0.0f, 0.7f, 0.0f));
}


