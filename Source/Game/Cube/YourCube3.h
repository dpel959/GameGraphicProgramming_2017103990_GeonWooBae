#pragma once
#include "BaseCube.h"

class YourCube3 : public BaseCube
{
public:
    YourCube3();
    ~YourCube3();
    virtual void Update(_In_ FLOAT deltaTime) override;
};