#pragma once
#include <main.h>
#include "core.h"
#include "scene.h"
#include "spaceship.h"
#include "DummyEnemy.h"
#include "Projectile.h"

class CoreTP1 : public Core
{
public:
	CoreTP1();
	virtual ~CoreTP1() override;

protected:
	virtual void Render(double dt) override;
	virtual void OnKeyW(bool down) override;
	virtual void OnKeyA(bool down) override;
	virtual void OnKeyS(bool down) override;
	virtual void OnKeyD(bool down) override;

protected:
	Spaceship spaceship;
	DummyEnemy dummyEnemy;
	Projectile projectile;
};