#include <glad/glad.h>

#include "../entities.h"
#include "../../models/models.h"
#include "../../toolbox/vector.h"
#include "shcraneplatform.h"
#include "../../renderEngine/renderEngine.h"
#include "../../objLoader/objLoader.h"
#include "../../engineTester/main.h"
#include "../../entities/player.h"
#include "../../toolbox/maths.h"
#include "../camera.h"
#include "../../collision/collisionmodel.h"
#include "../../collision/collisionchecker.h"
#include "../point.h"
#include "../../audio/audioplayer.h"
#include "../../audio/source.h"

#include <list>
#include <iostream>
#include <algorithm>

std::list<TexturedModel*> SH_CranePlatform::models;
CollisionModel* SH_CranePlatform::cmOriginal;

SH_CranePlatform::SH_CranePlatform()
{

}

SH_CranePlatform::SH_CranePlatform(float x, float y, float z, float rotY, float speed, int point1ID, int point2ID)
{
	position.x = x;
	position.y = y;
	position.z = z;
	rotX = 0;
	this->rotY = rotY;
	rotZ = 0;
	scale = 1;
	visible = true;
	updateTransformationMatrix();
	this->speed = speed;

	isMoving = false;
	canMove = true;

	timeOffset = 0;

	collideModelOriginal = SH_CranePlatform::cmOriginal;
	collideModelTransformed = loadCollisionModel("Models/SpeedHighway/", "CranePlatform");
	collideModelTransformed2 = loadCollisionModel("Models/SpeedHighway/", "CranePlatform");
	collideModelTransformedBackWall = loadCollisionModel("Models/SpeedHighway/", "CranePlatformBackWall2");

	CollisionChecker::addCollideModel(collideModelTransformed);
	CollisionChecker::addCollideModel(collideModelTransformed2);
	CollisionChecker::addCollideModel(collideModelTransformedBackWall);

	for (Triangle3D* tri : collideModelTransformedBackWall->triangles)
	{
		fprintf(stdout, "%f %f %f\n", tri->p1X, tri->p1Y, tri->p1Z);
	}

	updateCollisionModel();

	extern std::list<Entity*> gameEntitiesToAdd;

	for (auto e : gameEntitiesToAdd)
	{
		if (e->isPoint()) 
		{
			Point* thisPoint = (Point*)e;
			if (thisPoint->getID() == point1ID)
			{
				pointPos1 = thisPoint->getPosition();
			}
			else if(thisPoint->getID() == point2ID)
			{
				pointPos2 = thisPoint->getPosition();
			}
		}
	}

	pointDifference = pointPos2 - pointPos1;
	pointLength = pointDifference.length();
}

void SH_CranePlatform::step()
{
	//second collision model so that if the player touches the backmost wall they don't get stuck until the platform reaches it's end
	updateCMJustPosition(collideModelTransformed2);

	if (collideModelTransformed2->playerIsOn)
	{
		Global::gamePlayer->setPosition(Global::gamePlayer->getX() + (position.x - oldPos.x), Global::gamePlayer->getY() + (position.y - oldPos.y), Global::gamePlayer->getZ() + (position.z - oldPos.z) );
		Global::gamePlayer->setDisplacement(position.x - oldPos.x, position.y - oldPos.y, position.z - oldPos.z);
	}

	if (abs(getX() - Global::gameCamera->getPosition()->x) > ENTITY_RENDER_DIST)
	{
		setVisible(false);
	}
	else
	{
		if (abs(getZ() - Global::gameCamera->getPosition()->z) > ENTITY_RENDER_DIST)
		{
			setVisible(false);
		}
		else
		{
			setVisible(true);
		}
	}

	if (collideModelTransformed->playerIsOn && canMove && !isMoving && Global::gamePlayer->getY() < position.y + 7.03499) //start moving
	{
		isMoving = true;
		canMove = false;
	}

	if (timeOffset > 1 && isMoving) //stop moving
	{
		isMoving = false;
		if (cranePlatSource != nullptr)
		{
			cranePlatSource->stop();
			cranePlatSource = nullptr;
		}
	}

	if (isMoving)
	{
		oldPos = position;
		position.x = pointPos1.x + (pointDifference.x * fmod(timeOffset, 1));
		position.y = pointPos1.y + (pointDifference.y * fmod(timeOffset, 1));
		position.z = pointPos1.z + (pointDifference.z * fmod(timeOffset, 1));
		//move the player only if the player is standing on the platform
		if (collideModelTransformed->playerIsOn)
		{
			Global::gamePlayer->setCanMoveTimer(0);
			Global::gamePlayer->setDisplacement(position.x - oldPos.x, position.y - oldPos.y, position.z - oldPos.z);
		}
			if (collideModelTransformedBackWall->playerIsOn)
		{
			Global::gamePlayer->setPosition(Global::gamePlayer->getX() + (position.x - oldPos.x) * 10, Global::gamePlayer->getY() + (position.y - oldPos.y) * 10, Global::gamePlayer->getZ() + (position.z - oldPos.z) * 10);
			Global::gamePlayer->setDisplacement((position.x - oldPos.x) * 2, (position.y - oldPos.y) * 2, (position.z - oldPos.z) * 2);
		}
		//if (collideModelTransformedBackWall->playerIsOn)
		//{
		//	Global::gamePlayer->setDisplacement(position.x - oldPos.x, position.y - oldPos.y, position.z - oldPos.z);
		//}


		timeOffset += speed / pointLength;
		if (cranePlatSource == nullptr)
		{
			cranePlatSource = AudioPlayer::play(22, getPosition(), 1, true);
		}

		if (cranePlatSource != nullptr)
		{
			cranePlatSource->setPosition(getX(), getY(), getZ());
		}
	}
	
	updateCMJustPosition();
	updateCMJustPosition(collideModelTransformedBackWall);
	updateTransformationMatrix();
}

std::list<TexturedModel*>* SH_CranePlatform::getModels()
{
	return &SH_CranePlatform::models;
}

void SH_CranePlatform::loadStaticModels()
{
	if (SH_CranePlatform::models.size() > 0)
	{
		return;
	}

	std::fprintf(stdout, "Loading SH_CranePlatform static models...\n");

	std::list<TexturedModel*>* newModels = loadObjModel("res/Models/SpeedHighway/", "CranePlatform.obj");
	for (auto newModel : (*newModels))
	{
		SH_CranePlatform::models.push_back(newModel);
	}
	delete newModels;
	Global::countDelete++;


	if (SH_CranePlatform::cmOriginal == nullptr)
	{
		SH_CranePlatform::cmOriginal = loadCollisionModel("Models/SpeedHighway/", "CranePlatform");
	}
}

void SH_CranePlatform::deleteStaticModels()
{
	std::fprintf(stdout, "Deleting SH_CranePlatform static models...\n");
	for (auto model : SH_CranePlatform::models)
	{
		model->deleteMe();
		delete model;
		Global::countDelete++;
	}

	SH_CranePlatform::models.clear();

	if (SH_CranePlatform::cmOriginal != nullptr)
	{
		SH_CranePlatform::cmOriginal->deleteMe();
		delete SH_CranePlatform::cmOriginal;
		Global::countDelete++;
		SH_CranePlatform::cmOriginal = nullptr;
	}
}