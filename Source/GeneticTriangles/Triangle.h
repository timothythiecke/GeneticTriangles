// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "Triangle.generated.h"

// Forward declaration
class IIGene;

UCLASS()
class GENETICTRIANGLES_API ATriangle : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATriangle();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	const TArray<FVector>& GetPoints() const { return mPoints; }
	TArray<FVector>& GetPoints() { return mPoints; }
	TArray<float>& GetGeneticRepresentationWithFloats() { return mGeneticRepresentationWithFloats; }

	void ReconstructFromGeneticRepresentation();
	void DetermineGeneticRepresentation();

	void SetGeneticRepresentation(const TArray<float>& inNewGeneticRepresentation);
	TArray<float>& GetGeneticRepresentation() { return mGeneticRepresentationWithFloats; }

	void PostInit();

	void MutateChromosome(const int inPointIndex, const float inMaxMutationAxisOffset);

	FVector mLocation;

	//void Copy()
private:
	TArray<FVector> mPoints; ///< The points that will represent the triangle, these points are considered in relative space of the triangle actor
	FColor			mColor;  ///< The color with which we will visualize the triangle in 3D space
	float			mAbsMaxDistanceFromMidPoint = 80.0f; ///< The maximum distance from which a point can be generated

	TArray<IIGene*> mGeneticRepresentationWithGenes; ///< @TODO: Update name
	TArray<float> mGeneticRepresentationWithFloats;

private:

};
