/*+===================================================================
  File:      CUBE1.H

  Summary:   Cube header file contains declarations of Cube class
             used for the lab samples of Game Graphics Programming
             course.

  Classes: Cube

  © 2022 Kyung Hee University
===================================================================+*/
#pragma once

#include "Common.h"

#include "Cube/BaseCube.h"

/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
  Class:    Cube

  Summary:  A renderable 3d cube object

  Methods:  Update
              Overriden function that updates the cube every frame
            Cube
              Constructor.
            ~Cube
              Destructor.
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/
class KyaruCube : public BaseCube
{
public:
    KyaruCube(const std::filesystem::path& textureFilePath);
    KyaruCube(const KyaruCube& other) = delete;
    KyaruCube(KyaruCube&& other) = delete;
    KyaruCube& operator=(const KyaruCube& other) = delete;
    KyaruCube& operator=(KyaruCube&& other) = delete;
    ~KyaruCube() = default;

    virtual void Update(_In_ FLOAT deltaTime) override;
};