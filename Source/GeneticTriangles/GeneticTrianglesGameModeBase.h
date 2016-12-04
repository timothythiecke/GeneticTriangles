// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/GameModeBase.h"
#include "GeneticTrianglesGameModeBase.generated.h"

/**
 * 
 */
UCLASS()
class GENETICTRIANGLES_API AGeneticTrianglesGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	AGeneticTrianglesGameModeBase();

	virtual void BeginPlay() override;
	
};
