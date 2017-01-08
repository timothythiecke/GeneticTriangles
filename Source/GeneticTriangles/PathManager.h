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

UENUM(BlueprintType, Meta = (Bitflags))
enum EMutationType
{
	TranslatePoint = 1	UMETA(DisplayName = "TranslatePointMutation"),
	Insertion = 2		UMETA(DisplayName = "InsertionMutation"),
	Deletion = 3		UMETA(DisplayName = "DeletionMutation")
};
ENUM_CLASS_FLAGS(EMutationType)


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

	UPROPERTY(BlueprintReadOnly)
	float AverageFitness;

	// Fitness weights
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fitness Weights", meta = (ToolTip = "The less nodes a path has, the fitter it is"))
	float AmountOfNodesWeight;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fitness Weights", meta = (ToolTip = "The closer the final node of a path is to the targeted node, the fitter the path is"))
	float ProximityToTargetedNodeWeight;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fitness Weights", meta = (ToolTip = "Shorter paths are fitter than longer paths"))
	float LengthWeight;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fitness Weights", meta = (ToolTip = "If a path crosses an obstacle, then it is marked unfit. During fitness calculation, it will use this multiplier"))
	float ObstacleHitMultiplier;

	// Crossover
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crossover", meta = (ToolTip = "Decides the crossover operator to use, which will hugely affect the way offspring are generated"))
	ECrossoverOperator CrossoverOperator;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crossover", meta = (ToolTip = "When dealing with two variable length paths, the larger path will have a few nodes that can be considered for crossover. Though this will make the child solution less fit. This value determines per chromosome how much chance there is that it will be passed on."))
	float JunkDNACrossoverProbability;

	// Mutation
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mutation", meta = (ToolTip = "Do roulette wheel selection for what kind of mutation will occur, otherwise consider every mutation a possibility"))
	bool AggregateSelectOne;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mutation", meta = (ToolTip = "Apply bias to the final node"))
	bool HeadBias;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mutation", meta = (ToolTip = "Will either offset one node or all nodes"))
	bool FullMutation;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mutation")
	float MutationProbability;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mutation", meta = (ToolTip = "Will offset a random node"))
	float TranslatePointProbability;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mutation", meta = (ToolTip = "Will insert a new point between the start and final point"))
	float InsertionProbability;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mutation", meta = (ToolTip = "Will delete a point between the start and final point"))
	float DeletionProbability;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mutation", meta = (ToolTip = "Max offset during translation mutation"))
	float MaxTranslationOffset;

private:
	void EvaluateFitness();
	void SelectionStep();
	void CrossoverStep();
	void MutationStep();
	void Purge();
	void ColorCodePathsByFitness();

private:
	TArray<APath*> mPaths;
	TArray<APath*> mMatingPaths;
	float mTimer;
	float mTotalFitness;
};
