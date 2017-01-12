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

/**
* AnyButStart: A random chromosome is selected (except the starting one) and is mutated
* HeadOnly: The last chromosome in the genetic representation is mutated
* HeadFalloff: The last chromosome in the genetic representation is mutated, the same mutation is applied with a linear falloff for each subsequent chromosome
* AllAtOnce: All chromsomes (except the starting one) are mutated at the same time
*/
UENUM(BlueprintType)
enum class ETranslationMutationType : uint8
{
	AnyButStart UMETA(DisplayName = "AnyButStart"),
	HeadOnly UMETA(DisplayName = "HeadOnly"),
	HeadFalloff UMETA(DisplayName = "HeadFalloff"),
	AllAtOnce UMETA(DisplayName = "AllAtOnce")
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

	UPROPERTY(BlueprintReadOnly)
	int32 GenerationCount;

	UPROPERTY(BlueprintReadOnly)
	float AverageFitness;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "Nodes A and B in the world"))
	TArray<AActor*> Nodes;

	// Customization for user
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization", meta = (ToolTip = "The amount of seconds between each generation run"))
	float TimeBetweenGenerations;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization", meta = (ToolTip = "The color of paths that are marked invalid"))
	FColor InvalidPathColor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customization", meta = (ToolTip = "The amount of creatures to spawn"))
	int32 PopulationCount;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customization", meta = (ToolTip = "Maximum distance between each point of the paths in the first generation"))
	float MaxInitialVariation;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customization", meta = (ToolTip = "The least amount of points in path in the first generation"))
	int32 MinAmountOfPointsPerPathAtStartup;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customization", meta = (ToolTip = "The maximum amount of points in path in the first generation"))
	int32 MaxAmountOfPointsPerPathAtStartup;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customization", meta = (ToolTip = ""))
	float MaxSlopeToleranceAngle;

	// Fitness weights
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fitness Weights", meta = (ToolTip = "The less nodes a path has, the fitter it is"))
	float AmountOfNodesWeight;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fitness Weights", meta = (ToolTip = "The closer the final node of a path is to the targeted node, the fitter the path is"))
	float ProximityToTargetedNodeWeight;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fitness Weights", meta = (ToolTip = "Shorter paths are fitter than longer paths"))
	float LengthWeight;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fitness Weights", meta = (ToolTip = "If the path can see the target, then it is on the right track and thus fitter"))
	float CanSeeTargetWeight;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fitness Weights", meta = (ToolTip = "When the target node has been reached, apply this fitness weight to the fitness calculation for a path"))
	float TargetReachedWeight;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fitness Weights", meta = (ToolTip = "If a path crosses an obstacle, then it is marked unfit. During fitness calculation, it will use this multiplier"))
	float ObstacleHitMultiplier;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fitness Weights", meta = (ToolTip = "If the slope of a path between its chromosomes is in between"))
	float SlopeWeight;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fitness Weights", meta = (ToolTip = "If the slope of a path between two points is too large, the path will be marked unfit"))
	float SlopeTooIntenseMultiplier;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fitness Weights", meta = (ToolTip = "When the path travels through terrain, this multiplier kicks in"))
	float PiercesTerrainMultiplier;

	// Crossover
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crossover", meta = (ToolTip = "The probability of a pair mating with each other, the pairs will reproduce asexually otherwise"))
	float CrossoverProbability;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crossover", meta = (ToolTip = "Decides the crossover operator to use, which will hugely affect the way offspring are generated"))
	ECrossoverOperator CrossoverOperator;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crossover", meta = (ToolTip = "When dealing with two variable length paths, the larger path will have a few nodes that can be considered for crossover. Though this will make the child solution less fit. This value determines per chromosome how much chance there is that it will be passed on."))
	float JunkDNACrossoverProbability;

	// Mutation
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mutation", meta = (ToolTip = "Do roulette wheel selection for what kind of mutation will occur, otherwise consider every mutation a possibility"))
	bool AggregateSelectOne;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mutation", meta = (ToolTip = "Determines the way translation mutation is handled"))
	ETranslationMutationType TranslationMutationType;

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

	// Obstacle avoidance
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ObstacleAvoidance")
	bool ApplyObstacleAvoidance;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ObstacleAvoidance")
	float TraceDistance;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ObstacleAvoidance")
	float ObstacleAvoidanceBaseFitnessMultiplier;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ObstacleAvoidance")
	float ObstacleAvoidanceWeight;


private:
	void EvaluateFitness();
	void SelectionStep();
	void CrossoverStep();
	void MutationStep();
	void Purge();
	void ColorCodePathsByFitness();
	void LogGenerationInfo();

private:
	struct FGenerationInfo
	{
		int32 mGenerationNumber;
		int32 mCrossoverAmount;
		int32 mAmountOfTranslationMutations;
		int32 mAmountOfInsertionMutations;
		int32 mAmountOfDeletionMutations;
		float mAverageFitness;
		float mMaximumFitness;
		float mFitnessFactor;
		float mAverageAmountOfNodes;
	};

	FGenerationInfo mGenerationInfo;

	TArray<APath*> mPaths;
	TArray<APath*> mMatingPaths;
	float mTimer;
	float mTotalFitness;
};
