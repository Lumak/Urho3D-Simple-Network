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

#pragma once

#include "Sample.h"

namespace Urho3D
{

class Button;
class Connection;
class Scene;
class Text;
class UIElement;

}

//=============================================================================
//=============================================================================
class SceneReplication : public Sample
{
    URHO3D_OBJECT(SceneReplication, Sample);

public:
    SceneReplication(Context* context);

    virtual void Setup();
    virtual void Start();

protected:
    /// Return XML patch instructions for screen joystick layout for a specific sample app, if any.
    virtual String GetScreenJoystickPatchString() const { return
        "<patch>"
        "    <add sel=\"/element/element[./attribute[@name='Name' and @value='Hat0']]\">"
        "        <attribute name=\"Is Visible\" value=\"false\" />"
        "    </add>"
        "</patch>";
    }

private:
    void CreateServerSubsystem();
    void CreateScene();
    void CreateUI();
    void CreateAdminPlayer();
    void SetupViewport();
    void ChangeDebugHudText();
    void SubscribeToEvents();
    Button* CreateButton(const String& text, int width);
    void UpdateButtons();
    void MoveCamera();
    void HandlePhysicsPreStep(StringHash eventType, VariantMap& eventData);
    void HandlePostUpdate(StringHash eventType, VariantMap& eventData);
    void HandleConnect(StringHash eventType, VariantMap& eventData);
    void HandleDisconnect(StringHash eventType, VariantMap& eventData);
    void HandleStartServer(StringHash eventType, VariantMap& eventData);
    void HandleConnectionStatus(StringHash eventType, VariantMap& eventData);
    void HandleClientObjectID(StringHash eventType, VariantMap& eventData);

    HashMap<Connection*, WeakPtr<Node> > serverObjects_;
    SharedPtr<UIElement> buttonContainer_;
    SharedPtr<LineEdit> textEdit_;
    SharedPtr<Button> connectButton_;
    SharedPtr<Button> disconnectButton_;
    SharedPtr<Button> startServerButton_;
    SharedPtr<Text> instructionsText_;
    unsigned clientObjectID_;
    bool isServer_;

    bool drawDebug_;
};
