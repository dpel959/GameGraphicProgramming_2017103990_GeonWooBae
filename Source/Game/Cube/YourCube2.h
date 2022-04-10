#pragma once
#include "BaseCube.h"

class YourCube2 : public BaseCube
{
public:
    YourCube2();
    ~YourCube2();
    virtual void Update(_In_ FLOAT deltaTime) override;
};