#pragma once
#include "BaseCube.h"

class YourCube : public BaseCube
{
public:
    YourCube();
    ~YourCube();
    virtual void Update(_In_ FLOAT deltaTime) override;
};