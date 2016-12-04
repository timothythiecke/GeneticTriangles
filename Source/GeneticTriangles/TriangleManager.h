// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "TriangleManager.generated.h"

class ATriangle;

UCLASS()
class GENETICTRIANGLES_API ATriangleManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATriangleManager();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	UFUNCTION(BlueprintCallable, Category="GeneticAlgorithm")
	void InitializePopulation();

	void EvaluateFitness();
	void SelectionStep();
	void CrossoverStep();
	void MutationStep();

	bool HasTriangles() const { return mTriangles.Num() > 0; }

	void SetBalanceMutationRate(const bool inValue) { mUsesMutationRateBalancing = inValue; }

	UFUNCTION(BlueprintCallable, Category = "GeneticAlgorithm")
	float GetActualMutationRate() const { return mActualMutationRate; }

	UFUNCTION(BlueprintCallable, Category = "GeneticAlgorithm")
	int32 GetGenerationCount() const { return GenerationCount; }

	UFUNCTION(BlueprintCallable, Category = "GeneticAlgorithm")
	float GetAverageFitness() const { return AverageFitness; }

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "Determines the amount of fenotypes and clamps it to this number"))
	int32 PopulationSize;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "Determines mutation rate"))
	float MutationRate;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "If the average mutation rate crosses this value, balancing may occur."))
	float MutationRateThreshold;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "The max amount a point may mutate in any given axis"))
	float MaxMutationAxisOffset;

	UPROPERTY(BlueprintReadOnly, meta = (ToolTip = "The current generation the triangles belong to (RO)"))
	int32 GenerationCount;

	UPROPERTY(BlueprintReadOnly, meta = (ToolTip = "Calculated average fitness (RO)"))
	float AverageFitness;

private:
	void PurgeOld();
	void GenerateNew();

private:
	struct MappedTriangle
	{
		MappedTriangle(ATriangle* inPtr, const float inFitness) 
			:
			ptr(inPtr),
			fitness(inFitness)
		{ }

		bool operator==(const MappedTriangle& ref)
		{
			return ptr == ref.ptr;
		}

		ATriangle* ptr;
		float fitness;
		bool marked_for_reproduction = false;
	};

	TArray<ATriangle*> mTriangles;
	TArray<MappedTriangle> mMappedTrianglesContiguous;
	TArray<ATriangle*> mTrianglesSortedByMatingOrder;

	float mActualMutationRate;
	bool mUsesMutationRateBalancing;
};
