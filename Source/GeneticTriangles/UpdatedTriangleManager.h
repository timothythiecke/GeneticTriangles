// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "UpdatedTriangleManager.generated.h"

class ATriangle;

UCLASS()
class GENETICTRIANGLES_API AUpdatedTriangleManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AUpdatedTriangleManager();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	UFUNCTION(BlueprintCallable, Category = "GeneticAlgorithm")
	void Initialize();

	UFUNCTION(BlueprintCallable, Category = "GeneticAlgorithm")
	void RunGeneration();

	bool HasGeneratedTriangles() const { return mTriangles.Num() > 0; }

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "Determines the amount of fenotypes and clamps it to this number"))
	int32 RandomSeed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "Amount of creatures to generate and to maintain during each generation"))
	int32 PopulationCount;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "The GA will terminate by default if no suitable solution has been found by this generation"))
	int32 MaxGenerationCount;

	UPROPERTY(BlueprintReadOnly, meta = (ToolTip = "Amount of generations already occurred"))
	int32 GenerationCount;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "Determines mutation rate"))
	float CrossoverProbability;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "Pm"))
	float MutationProbability;

	UPROPERTY(BlueprintReadOnly, meta = (ToolTip = "Calculated average fitness (RO)"))
	float AverageFitness;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "Are elements allowed to mate with themselves or not"))
	bool AllowsSelfMating;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "The max amount a point may mutate in any given axis"))
	float MaxMutationAxisOffset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "The delay between generation steps"))
	float GenerationDelay = 1.0f;

private:
	void EvaluateFitness();
	void SelectionStep();
	void CrossoverStep();
	void MutationStep();
	void Purge();

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
	TArray<ATriangle*> mMatingTriangles;

	float mConstTimer = 1.0f;
};
