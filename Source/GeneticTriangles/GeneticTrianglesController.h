// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/PlayerController.h"
#include "GeneticTrianglesController.generated.h"

// Forward declaration
class ATriangleManager;

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
	
	UFUNCTION(BlueprintCallable, Category = "GeneticAlgorithm")
	void GeneratePopulation();

	UFUNCTION(BlueprintCallable, Category = "GeneticAlgorithm")
	void EvaluateFitnessOfPopulation();

	UFUNCTION(BlueprintCallable, Category = "GeneticAlgorithm")
	void SelectPairsForReproduction();

	UFUNCTION(BlueprintCallable, Category = "GeneticAlgorithm")
	void CrossoverStep();

	UFUNCTION(BlueprintCallable, Category = "GeneticAlgorithm")
	void Mutation();

	UFUNCTION(BlueprintCallable, Category = "GeneticAlgorithm")
	void DoEverything();

	UFUNCTION(BlueprintCallable, Category = "GeneticAlgorithm")
	ATriangleManager* GetTriangleManager() const { return mTriangleManager; }

	UFUNCTION(BlueprintCallable, Category = "GeneticAlgorithm")
	void SetMutationRateBalancing(const bool UsesMutationRateBalancing);

protected:
	virtual void GASpaceBar();
	virtual void GAFitness();

	virtual void FindTriangleManager();

private:
	ATriangleManager* mTriangleManager = nullptr;
};
