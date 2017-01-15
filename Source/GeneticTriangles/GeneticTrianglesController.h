// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/PlayerController.h"

#include "Enums.h"

#include "GeneticTrianglesController.generated.h"

// Forward declaration
class ATriangleManager;
class AUpdatedTriangleManager;
class APathManager;

/**
 * 
 */
UCLASS()
class GENETICTRIANGLES_API AGeneticTrianglesController : public APlayerController
{
	GENERATED_BODY()

public:
	AGeneticTrianglesController();
	virtual void BeginPlay() override;
	
	UFUNCTION(BlueprintCallable, Category = "GeneticTriangles")
	void GeneratePopulation();

	UFUNCTION(BlueprintCallable, Category = "GeneticTriangles")
	void EvaluateFitnessOfPopulation();

	UFUNCTION(BlueprintCallable, Category = "GeneticTriangles")
	void SelectPairsForReproduction();

	UFUNCTION(BlueprintCallable, Category = "GeneticTriangles")
	void CrossoverStep();

	UFUNCTION(BlueprintCallable, Category = "GeneticTriangles")
	void Mutation();

	UFUNCTION(BlueprintCallable, Category = "GeneticTriangles")
	void DoEverything();

	UFUNCTION(BlueprintCallable, Category = "GeneticTriangles")
	ATriangleManager* GetTriangleManager() const { return mTriangleManager; }

	UFUNCTION(BlueprintCallable, Category = "GeneticTriangles")
	void SetMutationRateBalancing(const bool UsesMutationRateBalancing);

	UFUNCTION(BlueprintCallable, Category = "GeneticPaths")
	void RequestAnimationControlStateUpdate(const EAnimationControlState inAnimationControlState);

	UFUNCTION(BlueprintCallable, Category = "GeneticPaths")
	void RequestDeserialization();

	UFUNCTION(BlueprintCallable, Category = "GeneticPaths")
	int32 RequestKnowledgeOfGenerationCount();
	
protected:
	virtual void GASpaceBar();
	virtual void GAFitness();

	virtual void FindTriangleManager();
	virtual void FindPathManager();

private:
	ATriangleManager* mTriangleManager = nullptr;
	AUpdatedTriangleManager* mUpdatedTriangleManager = nullptr;
	APathManager* mPathManager = nullptr;
};
