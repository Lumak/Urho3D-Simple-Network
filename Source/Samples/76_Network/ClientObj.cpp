//
// Copyright (c) 2008-2018 the Urho3D project.
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
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/SceneEvents.h>
#include <Urho3D/Network/Network.h>
#include <Urho3D/Network/NetworkEvents.h>
#include <Urho3D/Input/Controls.h>

#include "ClientObj.h"

#include <Urho3D/DebugNew.h>
//=============================================================================
//=============================================================================
ClientObj::ClientObj(Context *context)
    : LogicComponent(context)
    , userName_("Client1")
    , colorIdx_(0)
{
}

ClientObj::~ClientObj()
{
}

void ClientObj::RegisterObject(Context* context)
{
    context->RegisterFactory<ClientObj>();

    URHO3D_ATTRIBUTE("Name", String, userName_, String::EMPTY, AM_DEFAULT);
    URHO3D_ATTRIBUTE("Color Index", int, colorIdx_, 0, AM_DEFAULT | AM_NET);
}

void ClientObj::SetClientInfo(const String &usrName, int colorIdx)
{
    userName_ = usrName;
    colorIdx_ = colorIdx;
}

void ClientObj::ClearControls()
{
    controls_.buttons_ = 0;
}

void ClientObj::SetControls(const Controls &controls)
{
    controls_ = controls;
}



