// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

// Engine includes
#include "GameFramework/Actor.h"

// API includes
#include "Disposable.h"

#include "Path.generated.h"

// Forward declaration
enum class ETranslationMutationType : uint8;

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

	void SetFitness(const float inFitness) { mFitness = inFitness; }
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

	void SetColorCode(const FColor& inColor) { mColor = inColor; }

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

	void SnapToTerrain();

public:
	UPROPERTY(BlueprintReadWrite)
	USceneComponent* SceneComponent;
	
private:
	TArray<FVector> mGeneticRepresentation;
	UTextRenderComponent* mTextRenderComponent = nullptr;
	FColor mColor;
	float mFitness;
	float mLength;
	bool mIsInObstacle;
	bool mCanSeeTarget;
	bool mHasReachedTarget;
	bool mSlopeTooIntense;
	bool mTravelingThroughTerrain;
};
