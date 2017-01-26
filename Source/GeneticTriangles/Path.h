// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

// Engine includes
#include "GameFramework/Actor.h"

// API includes
#include "Disposable.h"

#include "Path.generated.h"

UCLASS()
class GENETICTRIANGLES_API APath : public AActor, public IDisposable
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actors' properties
	APath();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	virtual void Dispose();

	virtual void PostInit(const int32 inMinAmountOfNodes = 0.0f, const int32 inMaxAmountOfNodes = 0.0f);
	virtual void DetermineGeneticRepresentation();
	void SetGeneticRepresentation(const TArray<FVector>& inGeneticRepresentation);

	void RandomizeValues(const AActor* inStartingNode, const float inMaxVariation);

	void SetFitnessValues(const float inFitness, const float inNodesFitness) { mFitness = inFitness; mAmountOfNodesFitness = inNodesFitness; }
	float GetFitness() const { return mFitness; }

	float GetLength() const { return mLength; }
	int32 GetAmountOfNodes() const { return mGeneticRepresentation.Num(); }
	FVector GetLocationOfFinalNode() const;
	const TArray<FVector>& GetGeneticRepresentation() const { return mGeneticRepresentation; }

	void AddChromosome(const FVector& inChromosome);
	void InsertChromosome(const FVector& inChromosome, const int32 inIndex = 0);
	void RemoveChromosome(const int32 inIndex);
	FVector GetChromosome(const int32 inIndex) const;

	void MutateThroughTranslation(const ETranslationMutationType inTranslationMutationType, const float inMaxTranslationOffset);
	void MutateThroughInsertion();
	void MutateThroughDeletion();

	void SnapToTerrain();

	void SetColorCode(const FColor& inColor) { mColor = inColor; }
	FColor GetColorCode() const { return mColor; }

	void MarkIsInObstacle() { mIsInObstacle = true; }
	bool GetIsInObstacle() const { return mIsInObstacle; }

	void MarkCanSeeTarget() { mCanSeeTarget = true; }
	bool GetCanSeeTarget() const { return mCanSeeTarget; }

	void MarkHasReachedTarget() { mHasReachedTarget = true; }
	bool GetHasReachedTarget() const { return mHasReachedTarget; }

	void MarkSlopeTooIntense() { mSlopeTooIntense = true; }
	bool GetSlopeTooIntense() const { return mSlopeTooIntense; }

	void MarkTravelingThroughTerrain() { mTravelingThroughTerrain = true; }
	bool GetTravelingThroughTerrain() const { return mTravelingThroughTerrain; }

	float GetAmountOfNodesFitness() const { return mAmountOfNodesFitness; }

	void AddObstacleHitMultiplierChunk(const float inChunk) { mObstacleHitMultiplierChunk += inChunk; }
	float GetObstacleHitMultiplierChunk() const { return mObstacleHitMultiplierChunk; }

	void MarkDistanceBetweenChromosomesTooLarge() { mDistanceBetweenChromosomesTooLarge = true; }
	bool GetDistanceBetweenChromosomesTooLarge() const { return mDistanceBetweenChromosomesTooLarge; }

	void MarkFittestSolution() { mFittestSolution = true; }
	bool GetFittestSolution() const { return mFittestSolution; }

public:
	UPROPERTY(BlueprintReadWrite)
	USceneComponent* SceneComponent = nullptr;
	
private:
	TArray<FVector> mGeneticRepresentation;
	UTextRenderComponent* mTextRenderComponent = nullptr;
	FColor mColor = FColor::Black;
	
	float mFitness = 0.0f;
	float mAmountOfNodesFitness = 0.0f;
	float mLength = 0.0f;
	float mObstacleHitMultiplierChunk = 0.0f;
	
	bool mIsInObstacle = false;
	bool mCanSeeTarget = false;
	bool mHasReachedTarget = false;
	bool mSlopeTooIntense = false;
	bool mTravelingThroughTerrain = false;
	bool mDistanceBetweenChromosomesTooLarge = false;
	bool mFittestSolution = false;
};
