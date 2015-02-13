#include "Projectile.h"

Projectile::Projectile(double x, double y, double z, double dx, double dy, double dz) :
    _outerSphere(glm::vec4(1.0f, 1.0f, 0, 0.7f), 8, 8, 0.5f),
    _innerSphere(glm::vec4(0, 1.0f, 0, 1), 8, 8, 0.25f),
	_positionX(x),
	_positionY(y),
	_positionZ(z),
	_dx(dx),
	_dy(dy),
	_dz(dz)
{
	_outerSphere.AddChild(&_innerSphere);
}

Projectile::~Projectile()
{
	_LOG_INFO() << "~Projectile";
}

void Projectile::render(double dt)
{
	_positionX += _dx * dt;
	_positionY += _dy * dt;
	_positionZ += _dz * dt;

	_outerSphere.SetTransform(glm::translate(glm::mat4(), glm::vec3(_positionX, _positionY, _positionZ)));

	_outerSphere.Render();
	_innerSphere.Render();
}

glm::vec3 Projectile::getPosition()
{
	return glm::vec3(_positionX, _positionY, _positionZ);
}

bool Projectile::isOutsideScreen()
{
	return (
		((_dz < 0) && (_positionZ <= OUTSIDE_SCREEN_MINIMUM_Z))
		||
		((_dz > 0) && (_positionZ >= OUTSIDE_SCREEN_MAXIMUM_Z))
		||
		(_positionX <= OUTSIDE_SCREEN_MINIMUM_X)
		||
		(_positionX >= OUTSIDE_SCREEN_MAXIMUM_X)
		);
}