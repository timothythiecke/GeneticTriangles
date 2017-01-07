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
	// Sets default values for this actor's properties
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

	void Mutate();

	void SetColorCode(const FColor& color) { mColor = color; }

public:
	UPROPERTY(BlueprintReadWrite)
	USceneComponent* SceneComponent;
	
private:
	TArray<FVector> mGeneticRepresentation;
	UTextRenderComponent* mTextRenderComponent = nullptr;
	FColor mColor;
	float mFitness;
	float mLength;
};
