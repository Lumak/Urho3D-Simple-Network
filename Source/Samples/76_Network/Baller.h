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

#include "ClientObj.h"

namespace Urho3D
{
class Controls;
class Node;
class RigidBody;
}

using namespace Urho3D;
//=============================================================================
//=============================================================================
static const unsigned CTRL_FORWARD = (1<<0);
static const unsigned CTRL_BACK    = (1<<1);
static const unsigned CTRL_LEFT    = (1<<2);
static const unsigned CTRL_RIGHT   = (1<<3);
static const unsigned SWAP_MAT     = (1<<4);

static const unsigned BALLER_COL_LAYER = 2;
static const int MAX_MAT_COUNT = 9;

//=============================================================================
//=============================================================================
class Baller : public ClientObj
{
    URHO3D_OBJECT(Baller, ClientObj)

public:
    Baller(Context* context);
    ~Baller();
   
    static void RegisterObject(Context* context);

    virtual void ApplyAttributes();
    virtual void DelayedStart();
    virtual void Create();

protected:
    void SwapMat();
    virtual void FixedUpdate(float timeStep);
   
protected:
    WeakPtr<RigidBody> hullBody_;
    WeakPtr<Node> nodeInfo_;
    Controls prevControls_;

    float mass_;
};

