#include <glad/glad.h>

#include "../entities.h"
#include "../../models/models.h"
#include "shspotlight.h"
#include "../../renderEngine/renderEngine.h"
#include "../../objLoader/objLoader.h"
#include "../../engineTester/main.h"
#include "../../animation/body.h"
#include "../camera.h"

#include <list>

std::list<TexturedModel*> SH_Spotlight::models;
std::list<TexturedModel*> SH_Spotlight::modelsLight;

SH_Spotlight::SH_Spotlight(float x, float y, float z,
	float xRot, float yRot, float zRot)
{
	position.x = x;
	position.y = y;
	position.z = z;
	rotX = xRot;
	rotY = yRot;
	rotZ = zRot;
	scale = 1;
	visible = true;
	updateTransformationMatrixSADX();

	light = new Body(&SH_Spotlight::modelsLight);
	light->setVisible(true);
	Global::countNew++;
	Main_addTransparentEntity(light);
	light->setPosition(&position);
	light->setRotX(xRot);
	light->setRotY(yRot);
	light->setRotZ(zRot);
	light->setScale(1);
	light->updateTransformationMatrixSADX();
}

void SH_Spotlight::step()
{
	if (abs(getX() - Global::gameCamera->getPosition()->x) > ENTITY_RENDER_DIST_HIGH)
	{
		setVisible(false);
		light->setVisible(false);
	}
	else
	{
		if (abs(getZ() - Global::gameCamera->getPosition()->z) > ENTITY_RENDER_DIST_HIGH)
		{
			setVisible(false);
			light->setVisible(false);
		}
		else
		{
			setVisible(true);
			light->setVisible(true);
		}
	}
}

std::list<TexturedModel*>* SH_Spotlight::getModels()
{
	return &SH_Spotlight::models;
}

void SH_Spotlight::loadStaticModels()
{
	if (SH_Spotlight::models.size() > 0)
	{
		return;
	}

	std::fprintf(stdout, "Loading SH_Spotlight static models...\n");

	loadObjModel(&SH_Spotlight::models, "res/Models/SpeedHighway/", "Spotlight.obj");
	loadObjModel(&SH_Spotlight::modelsLight, "res/Models/SpeedHighway/", "SpotlightLight.obj");
}

void SH_Spotlight::deleteStaticModels()
{
	std::fprintf(stdout, "Deleting SH_Spotlight static models...\n");

	Entity::deleteModels(&SH_Spotlight::models);
	Entity::deleteModels(&SH_Spotlight::modelsLight);
}
