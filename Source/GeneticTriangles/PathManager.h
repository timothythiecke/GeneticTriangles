// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

// Engine includes
#include "GameFramework/Actor.h"

// API includes
#include "Disposable.h"

#include "PathManager.generated.h"

// Forward decl
class APath;

UENUM(BlueprintType)
enum class ECrossoverOperator : uint8
{
	SinglePoint UMETA(DisplayName="Single Point"),
	DoublePoint UMETA(DisplayName="Double Point"),
	Uniform UMETA(DisplayName="Uniform")
};

UCLASS()
class GENETICTRIANGLES_API APathManager : public AActor, public IDisposable
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APathManager();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	virtual void Dispose();

	void RunGeneration();

public:
	UPROPERTY(BlueprintReadWrite, meta = (Tooltip = "The transform component of the path manager, to be exposed to the editor."))
	USceneComponent* SceneComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "The amount of creatures to spawn"))
	int32 PopulationCount;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "Nodes A and B in the world"))
	TArray<AActor*> Nodes;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float MaxInitialVariation;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 MinAmountOfPointsPerPathAtStartup;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 MaxAmountOfPointsPerPathAtStartup;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float TimeBetweenGenerations;

	UPROPERTY(BlueprintReadOnly)
	int32 GenerationCount;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float CrossoverProbability;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float MutationProbability;

	UPROPERTY(BlueprintReadOnly)
	float AverageFitness;

	// Fitness weights
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fitness Weights", meta = (ToolTip = "The less nodes a path has, the fitter it is"))
	float AmountOfNodesWeight;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fitness Weights", meta = (ToolTip = "The closer the final node of a path is to the targeted node, the fitter the path is"))
	float ProximityToTargetedNodeWeight;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fitness Weights", meta = (ToolTip = "Shorter paths are fitter than longer paths"))
	float LengthWeight;

	// Crossover
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crossover", meta = (ToolTip = "Decides the crossover operator to use, which will hugely affect the way offspring are generated"))
	ECrossoverOperator CrossoverOperator;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crossover", meta = (ToolTip = "When dealing with two variable length paths, the larger path will have a few nodes that can be considered for crossover. Though this will make the child solution less fit. This value determines per chromosome how much chance there is that it will be passed on."))
	float JunkDNACrossoverProbability;

private:
	void EvaluateFitness();
	void SelectionStep();
	void CrossoverStep();
	void MutationStep();
	void Purge();

private:
	TArray<APath*> mPaths;
	TArray<APath*> mMatingPaths;
	float mTimer;
	float mTotalFitness;
};
