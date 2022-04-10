#include"YourCube3.h"

YourCube3::YourCube3()
{
}

YourCube3::~YourCube3()
{
}

void YourCube3::Update(_In_ FLOAT deltaTime)
{

	XMMATRIX mSpin = XMMatrixRotationZ(-deltaTime);
	XMMATRIX mOrbit = XMMatrixRotationX(-deltaTime * 2.0f);
	XMMATRIX mTranslate = XMMatrixTranslation(0.0f, 4.0f, 0.0f);
	XMMATRIX mScale = XMMatrixScaling(0.3f, 0.3f, 0.3f);

	m_world = mScale * mSpin * mTranslate * mOrbit;
}