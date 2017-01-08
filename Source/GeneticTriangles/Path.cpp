// Fill out your copyright notice in the Description page of Project Settings.

#include "GeneticTriangles.h"
#include "Path.h"


// Sets default values
APath::APath() :
	SceneComponent(nullptr),
	mLength(0.0f),
	mIsInObstacle(false)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	RootComponent = SceneComponent;

	mTextRenderComponent = CreateDefaultSubobject<UTextRenderComponent>(TEXT("TextRenderComponent"));

	mColor = FColor(FMath::RandRange(0, 255), FMath::RandRange(0, 255), FMath::RandRange(0, 255), 255);
}

// Called when the game starts or when spawned
void APath::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APath::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

	for (int32 i = 0; i < mGeneticRepresentation.Num(); ++i)
	{
		if (mGeneticRepresentation.IsValidIndex(i + 1))
		{
			DrawDebugLine(GetWorld(), mGeneticRepresentation[i], mGeneticRepresentation[i + 1], mColor, true, 0.1f, 0, 2.0f);
		}
	}
}


void APath::Dispose()
{
	this->Destroy();
}



void APath::PostInit(const int32 inMinAmountOfNodes, const int32 inMaxAmountOfNodes)
{
	const int32 amount_of_positions = FMath::RandRange(inMinAmountOfNodes, inMaxAmountOfNodes);
	
	mGeneticRepresentation.Reserve(amount_of_positions);

	for (int32 i = 0; i < amount_of_positions; ++i)
		mGeneticRepresentation.Add(FVector());
}



void APath::DetermineGeneticRepresentation()
{
	// Calculate any values that might be useful for the fitness evaluation

	for (int32 i = 0; i < mGeneticRepresentation.Num() - 1; ++i) // Allows us to safely check for the last position without having to worry about going out of bounds
		mLength += (mGeneticRepresentation[i + 1] - mGeneticRepresentation[i]).Size();

	check(mLength > 0.0f);
}



void APath::SetGeneticRepresentation(const TArray<FVector>& inGeneticRepresentation)
{
	mGeneticRepresentation.Reserve(inGeneticRepresentation.Num());

	for (const FVector& location : inGeneticRepresentation)
	{
		mGeneticRepresentation.Add(location);
	}

	DetermineGeneticRepresentation();
}



void APath::RandomizeValues(const AActor* inStartingNode, const float inMaxVariation)
{
	if (inStartingNode == nullptr)
		return;

	// The first point of a path will always be the first node
	mGeneticRepresentation[0] = inStartingNode->GetTransform().GetLocation();

	// Use the previous point to calculate a new random location
	for (int32 i = 1; i < mGeneticRepresentation.Num(); ++i)
		mGeneticRepresentation[i] = FVector(
											FMath::FRandRange(-inMaxVariation, inMaxVariation), 
											FMath::FRandRange(-inMaxVariation, inMaxVariation), 
											0.0f) + 
											mGeneticRepresentation[i-1];
}



FVector APath::GetLocationOfFinalNode() const
{
	// If the following occurs then the path is most likely uninitialized
	if (mGeneticRepresentation.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("APath::GetLocationOfFinalNode >> Attempting access of empty path! Returning zero vector."));
		return FVector::ZeroVector;
	}

	return mGeneticRepresentation[mGeneticRepresentation.Num() - 1];
}



void APath::AddChromosome(const FVector& inChromosome)
{
	mGeneticRepresentation.Add(inChromosome);
}



void APath::InsertChromosome(const FVector& inChromosome, const int32 inIndex)
{
	mGeneticRepresentation.Insert(inChromosome, inIndex);
}



void APath::RemoveChromosome(const int32 inIndex)
{
	mGeneticRepresentation.RemoveAt(inIndex);
}



FVector APath::GetChromosome(const int32 inIndex) const
{
	return mGeneticRepresentation.IsValidIndex(inIndex) ? mGeneticRepresentation[inIndex] : FVector::ZeroVector;
}


/**
* Mutates the path through translating points
*
*/
void APath::MutateThroughTranslation(const bool inHeadBias, const bool inFullMutation, const float inMaxTranslationOffset)
{
	if (inHeadBias)
	{
		if (inFullMutation) // Mutate the entire path, starting with the head, then apply linear falloff to all points
		{
			FVector offset = FVector(FMath::FRandRange(-inMaxTranslationOffset, inMaxTranslationOffset), FMath::FRandRange(-inMaxTranslationOffset, inMaxTranslationOffset), 0.0f);

			for (int32 i = mGeneticRepresentation.Num() - 1; i > 1; --i)
			{
				const float multiplier = i / (float)(mGeneticRepresentation.Num() - 1);
				mGeneticRepresentation[i] += offset * multiplier;
			}
		}
		else // Mutate only the head
		{
			FVector offset = FVector(FMath::FRandRange(-inMaxTranslationOffset, inMaxTranslationOffset), FMath::FRandRange(-inMaxTranslationOffset, inMaxTranslationOffset), 0.0f);
			mGeneticRepresentation.Last() += offset;
		}
	}
	else
	{
		if (inFullMutation) // Mutate all chromsomes
		{
			for (int32 i = 1; i < mGeneticRepresentation.Num(); ++i)
			{
				FVector offset = FVector(FMath::FRandRange(-inMaxTranslationOffset, inMaxTranslationOffset), FMath::FRandRange(-inMaxTranslationOffset, inMaxTranslationOffset), 0.0f);
				mGeneticRepresentation[i] += offset;
			}
		}
		else // Mutate a single random chromosome (excluding the starting chromsome)
		{
			const uint32 mutating_chromosome_index = FMath::RandRange(1, mGeneticRepresentation.Num() - 1);
			FVector offset = FVector(FMath::FRandRange(-inMaxTranslationOffset, inMaxTranslationOffset), FMath::FRandRange(-inMaxTranslationOffset, inMaxTranslationOffset), 0.0f);
			mGeneticRepresentation[mutating_chromosome_index] += offset;
		}
	}
}



/**
* Inserts a point / chromosome in the genetic representation
* Insertion should happen between the starting and final node
* The chromosome that gets added will be in the middle of the the element before inserting and the previous
*/
void APath::MutateThroughInsertion()
{
	if (mGeneticRepresentation.Num() >= 2)
	{
		const uint32 insertion_index = FMath::RandRange(1, mGeneticRepresentation.Num() - 2);
		if (mGeneticRepresentation.IsValidIndex(insertion_index) && mGeneticRepresentation.IsValidIndex(insertion_index - 1))
		{
			FVector mid_point = (mGeneticRepresentation[insertion_index] + mGeneticRepresentation[insertion_index - 1]) / 2.0f;
			mGeneticRepresentation.Insert(mid_point, insertion_index);
		}
	}
}



/**
* Removes a point / chromosome from the genetic representation
* Removal happens between the starting and final node
*/
void APath::MutateThroughDeletion()
{
	if (mGeneticRepresentation.Num() > 2)
	{
		const uint32 deletion_index = FMath::RandRange(1, mGeneticRepresentation.Num() - 2);
		if (mGeneticRepresentation.IsValidIndex(deletion_index))
			mGeneticRepresentation.RemoveAt(deletion_index);
	}
}