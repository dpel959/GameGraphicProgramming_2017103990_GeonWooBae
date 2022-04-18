#include "Cube/KyaruCube.h"

/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Method:   Cube::Cube

  Summary:  Constructor

  Args:     const std::filesystem::path& textureFilePath
              Path to the texture to use
M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
KyaruCube::KyaruCube(const std::filesystem::path& textureFilePath)
    : BaseCube(textureFilePath)
{
}
/*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
  Method:   Cube::Update

  Summary:  Updates the cube every frame

  Args:     FLOAT deltaTime
              Elapsed time

  Modifies: [m_world].
M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
void KyaruCube::Update(_In_ FLOAT deltaTime)
{
    static FLOAT s_totalTime = 0.0f, move_speed = 10.0f;
    s_totalTime += deltaTime;

    m_world = XMMatrixTranslation(2.0f, XMScalarSin(s_totalTime * move_speed), 0.0f) * XMMatrixRotationY(s_totalTime)
        * XMMatrixTranslation(0.0f, 2.0f, 0.0f);
}