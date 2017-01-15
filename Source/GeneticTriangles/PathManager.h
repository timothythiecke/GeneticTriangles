// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

// Engine includes
#include "GameFramework/Actor.h"

// API includes
#include "Disposable.h"
#include "Enums.h"

#include "PathManager.generated.h"

// Forward decl
class APath;

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

	void ChangeAnimationControlState(const EAnimationControlState inAnimationControlState);
	void DeserializeData();

public:
	UPROPERTY(BlueprintReadWrite, meta = (Tooltip = "The transform component of the path manager, to be exposed to the editor."))
	USceneComponent* SceneComponent = nullptr;

	UPROPERTY(BlueprintReadOnly)
	int32 GenerationCount = 0;

	UPROPERTY(BlueprintReadOnly)
	float AverageFitness = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "Nodes A and B in the world"))
	TArray<AActor*> Nodes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ToolTip = "Start generation run automatically in Editor"))
	bool AutoRun = false;

	// Customization for user
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization", meta = (ToolTip = "The amount of seconds before a new generation is run"))
	float TimeBetweenGenerations = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization", meta = (ToolTip = "The color of invalid / unfit paths"))
	FColor InvalidPathColor = FColor::Black;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customization", meta = (ToolTip = "The amount of creatures to spawn", UIMin=2))
	int32 PopulationCount = 20;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customization", meta = (ToolTip = "Maximum distance between each point of the paths in the first generation"))
	float MaxInitialVariation = 40.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customization", meta = (ToolTip = "The least amount of points in path in the first generation", UIMin = 2, UIMax = 20))
	int32 MinAmountOfPointsPerPathAtStartup = 2;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customization", meta = (ToolTip = "The maximum amount of points in path in the first generation", UIMin = 2, UIMax = 20))
	int32 MaxAmountOfPointsPerPathAtStartup = 10;

	// Standard fitness
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fitness", meta = (ToolTip = "The less nodes a path has, the fitter it is", UIMin=0.0f))
	float AmountOfNodesWeight = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fitness", meta = (ToolTip = "The closer the final node of a path is to the targeted node, the fitter the path is", UIMin = 0.0f))
	float ProximityToTargetedNodeWeight = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fitness", meta = (ToolTip = "Shorter paths are fitter than longer paths", UIMin = 0.0f))
	float LengthWeight = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fitness", meta = (ToolTip = "If the path can see the target, then it is on the right track and thus fitter", UIMin = 0.0f))
	float CanSeeTargetWeight = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fitness", meta = (ToolTip = "When the target node has been reached, apply this fitness weight to the fitness calculation for a path", UIMin = 0.0f))
	float TargetReachedWeight = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fitness", meta = (ToolTip = "If a path crosses an obstacle, then it is marked unfit. During fitness calculation, it will use this multiplier", UIMin =0.0f, UIMax=1.0f))
	float ObstacleHitMultiplier = 0.0f;

	// --- Slope fitness
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Slope Fitness", meta = (ToolTip = "If the slope of a path between its chromosomes is in between", UIMin = 0.0f))
	bool UseSlopeFitnessEvaluation = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Slope Fitness", meta = (ToolTip = "If the slope of a path between its chromosomes is in between", UIMin = 0.0f))
	float SlopeWeight = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Slope Fitness", meta = (ToolTip = "If the slope of a path between two points is too large, the path will be marked unfit", UIMin = 0.0f, UIMax = 1.0f))
	float SlopeTooIntenseMultiplier = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Slope Fitness", meta = (ToolTip = "When the path travels through terrain, this multiplier kicks in", UIMin = 0.0f, UIMax = 1.0f))
	float PiercesTerrainMultiplier = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Slope Fitness", meta = (ToolTip = "The maximum angle to use during slope fitness evaluation", UIMin = 0.0f, UIMax = 90.0f))
	float MaxSlopeToleranceAngle = 45.0f;

	// --- Max length fitness
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Length Fitness", meta = (ToolTip = "Mark paths unfit if the distance between its nodes is larger than the one specified. This is to avoid paths ignoring valleys in terrain."))
	bool UseMaxLengthFitness = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Length Fitness", meta = (ToolTip = "Distance between chromosomes", UIMin = 0.0f))
	float MaxEuclidianDistance = 40.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Length Fitness", meta = (UIMin = 0.0f, UIMax = 1.0f))
	float EuclidianOvershootMultiplier = 0.0f;

	//
	// @TODO: Need some form of different selection methods
	// Roulette wheel, stochastic, elitism

	// Crossover
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crossover", meta = (ToolTip = "The probability of a pair mating with each other, the pairs will reproduce asexually otherwise", UIMin = 0.0f, UIMax = 100.0f))
	float CrossoverProbability = 70.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crossover", meta = (ToolTip = "Decides the crossover operator to use, which will hugely affect the way offspring are generated"))
	ECrossoverOperator CrossoverOperator = ECrossoverOperator::SinglePoint;

	// Mutation
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mutation", meta = (ToolTip = "Determines the way translation mutation is handled"))
	ETranslationMutationType TranslationMutationType = ETranslationMutationType::AnyButStart;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mutation", meta = (ToolTip="The probability that a path may mutate", UIMin = 0.0f, UIMax = 100.0f))
	float MutationProbability = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mutation", meta = (ToolTip = "The path will mutate by translating chromosomes in its genetic makeup", UIMin = 0.0f, UIMax = 100.0f))
	float TranslatePointProbability = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mutation", meta = (ToolTip = "The path will mutate by inserting a new chromosome in its genetic makeup", UIMin = 0.0f, UIMax = 1.0f))
	float InsertionProbability = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mutation", meta = (ToolTip = "The path will mutate by deleting a chromsome from its genetic makeup", UIMin = 0.0f, UIMax = 100.0f))
	float DeletionProbability = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mutation", meta = (ToolTip = "When applying a translation mutation, use this value as the maximum radius", UIMin = 0.0f))
	float MaxTranslationOffset = 40.0f;

	// Obstacle avoidance
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ObstacleAvoidance", meta = (ToolTip = "During fitness evaluation, check if any of the nodes are too close to an obstacle and apply fitness multipliers."))
	bool ApplyObstacleAvoidanceLogic = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ObstacleAvoidance", meta = (ToolTip = "Determines the way rays will be traced from a chromsome"))
	EObstacleTraceBehaviour TraceBehaviour = EObstacleTraceBehaviour::WindDirectionTracing;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ObstacleAvoidance", meta = (ToolTip = "The distance or radius in euclidian space between a chromosome and the end point"))
	float TraceDistance = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ObstacleAvoidance")
	float ObstacleAvoidanceBaseFitnessMultiplier = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ObstacleAvoidance")
	float ObstacleAvoidanceWeight = 100.0f;


private:
	void RunGenerationTimer(const float inDeltaTime);
	void RunGeneration();

	void InitializeRun();
	void EvaluateFitness();
	void SelectionStep();
	void CrossoverStep();
	void MutationStep();
	void Purge();
	void ColorCodePathsByFitness();
	void LogGenerationInfo();
	void AddGenerationInfoToSerializableData();

	void StopRun();
	void SerializeData();

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

	EAnimationControlState mNextAnimationControlState = EAnimationControlState::Limbo;
	EAnimationControlState mPreviousAnimationControlState = EAnimationControlState::Limbo;

	struct FPathSerializationData
	{
		int32 mNodeAmount;
		TArray<FVector> mGeneticRepresentation;
	};

	using FDataBlob = TArray<TArray<FPathSerializationData>>;
	FDataBlob mSerializationData;
	FDataBlob mDeserializationData;
};
