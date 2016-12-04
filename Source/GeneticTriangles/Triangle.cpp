// Fill out your copyright notice in the Description page of Project Settings.

#include "GeneticTriangles.h"
#include "Triangle.h"

#include "FloatGene.h"

// Sets default values
ATriangle::ATriangle()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Generate color once
	mColor = FColor(FMath::RandRange(0, 255), FMath::RandRange(0, 255), FMath::RandRange(0, 255), 255);
}

void ATriangle::PostInit()
{
	/*const FVector actor_location = FVector(FMath::RandRange(-100, 100), FMath::RandRange(-100, 100), FMath::RandRange(-100, 100));
	SetActorLocation(actor_location);*/

	mPoints.Reserve(3);
	for (uint8 i = 0; i < 3; ++i)
	{
		// Consider only relative points
		const FVector point_location = /*actor_location +*/
			FVector(FMath::RandRange(-mAbsMaxDistanceFromMidPoint, mAbsMaxDistanceFromMidPoint),
				FMath::RandRange(-mAbsMaxDistanceFromMidPoint, mAbsMaxDistanceFromMidPoint),
				FMath::RandRange(-mAbsMaxDistanceFromMidPoint, mAbsMaxDistanceFromMidPoint));

		mPoints.Add(point_location);
	}
}

// Called when the game starts or when spawned
void ATriangle::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ATriangle::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

	// Visualize the triangle by drawing lines between each point
	DrawDebugLine(GetWorld(), GetActorLocation() + mPoints[0], GetActorLocation() + mPoints[1], mColor, true, 0.1f, 0, 0.5f);
	DrawDebugLine(GetWorld(), GetActorLocation() + mPoints[1], GetActorLocation() + mPoints[2], mColor, true, 0.1f, 0, 0.5f);
	DrawDebugLine(GetWorld(), GetActorLocation() + mPoints[2], GetActorLocation() + mPoints[0], mColor, true, 0.1f, 0, 0.5f);
}



void ATriangle::DetermineGeneticRepresentation()
{
	/*// Reserve space to fit all elements
	mGeneticRepresentation.Empty();
	mGeneticRepresentation.Reserve(CHAR_BIT * sizeof(float) * 3 * 3); // 8 bits per byte, assume 4 bytes per float, 3 floats per vector, 3 vectors per triangle => 288, then assume a crossover segment length of 32

	// Use union method
	union
	{
		float input;
		long output;
	} data;

	for (int i = 0; i < mPoints.Num(); ++i)
	{
		// X
		data.input = mPoints[i].X;
		std::bitset<sizeof(float) * CHAR_BIT> bits_x(data.output); // Outputs 32 bits

		for (int j = 0; j < sizeof(float) * CHAR_BIT; ++j)
			mGeneticRepresentation.Add(bits_x[i]);

		// Y
		data.input = mPoints[i].Y;
		std::bitset<sizeof(float) * CHAR_BIT> bits_y(data.output); // Outputs 32 bits

		for (int j = 0; j < sizeof(float) * CHAR_BIT; ++j)
			mGeneticRepresentation.Add(bits_y[j]);

		// Z
		data.input = mPoints[i].Z;
		std::bitset<sizeof(float) * CHAR_BIT> bits_z(data.output); // Outputs 32 bits

		for (int j = 0; j < sizeof(float) * CHAR_BIT; ++j)
			mGeneticRepresentation.Add(bits_z[j]);
	}*/

	/*for (int i = 0; i < mGeneticRepresentationWithGenes.Num(); ++i)
	{
		IIGene* ptr = mGeneticRepresentationWithGenes[i];

		if (ptr != nullptr)
		{
			ptr->Dispose();
			ptr = nullptr;
		}
	}

	mGeneticRepresentationWithGenes.Empty();
	mGeneticRepresentationWithGenes.Reserve(9); // 3 locations * 3 floats

	FloatGene* float_gene_ptr = new FloatGene();
	mGeneticRepresentationWithGenes.Add();*/

	// @TODO: This needs to be done with genes!
	mGeneticRepresentationWithFloats.Empty();
	mGeneticRepresentationWithFloats.Reserve(9);

	for (const FVector& point : mPoints)
	{
		mGeneticRepresentationWithFloats.Add(point.X);
		mGeneticRepresentationWithFloats.Add(point.Y);
		mGeneticRepresentationWithFloats.Add(point.Z);
	}
}



// Based on the genetic representation array, reconstruct the points of this triangle
void ATriangle::ReconstructFromGeneticRepresentation()
{
	mPoints.Empty();
	mPoints.Reserve(3);

	for (int i = 0; i < mGeneticRepresentationWithFloats.Num(); i += 3)
	{
		mPoints.Add(FVector(mGeneticRepresentationWithFloats[i], mGeneticRepresentationWithFloats[i + 1], mGeneticRepresentationWithFloats[i + 2]));
	}
}



void ATriangle::SetGeneticRepresentation(const TArray<float>& inNewGeneticRepresentation)
{
	mGeneticRepresentationWithFloats.Reserve(inNewGeneticRepresentation.Num());

	// Use for loop to copy values, move semantic for array might not work here
	for (const float value : inNewGeneticRepresentation)
		mGeneticRepresentationWithFloats.Add(value);
}



// Mutates one of the points by offsetting one of the points in a random direction in a limited radius
void ATriangle::MutateChromosome(const int inPointIndex, const float inMaxMutationAxisOffset)
{
	FVector random_direction = FVector(FMath::RandRange(-1.0f, 1.0f), FMath::RandRange(-1.0f, 1.0f), FMath::RandRange(-1.0f, 1.0f));
	random_direction *= inMaxMutationAxisOffset;

	mPoints[inPointIndex] += random_direction;
}