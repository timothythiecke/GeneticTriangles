// Fill out your copyright notice in the Description page of Project Settings.

#include "GeneticTriangles.h"
#include "GeneticTrianglesGameModeBase.h"

#include "GeneticTrianglesController.h"

AGeneticTrianglesGameModeBase::AGeneticTrianglesGameModeBase()
{
	PlayerControllerClass = AGeneticTrianglesController::StaticClass();
}


void AGeneticTrianglesGameModeBase::BeginPlay()
{
	Super::BeginPlay();


}
