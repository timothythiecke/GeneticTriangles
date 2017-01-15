// Fill out your copyright notice in the Description page of Project Settings.

#include "GeneticTriangles.h"
#include "Path.h"

#include "PathManager.h"

// Sets default values
APath::APath() :
	SceneComponent(nullptr),
	mFitness(0.0f),
	mLength(0.0f),
	mAmountOfNodesFitness(0.0f),
	mObstacleHitMultiplierChunk(0.0f),
	mIsInObstacle(false),
	mCanSeeTarget(false),
	mHasReachedTarget(false),
	mSlopeTooIntense(false),
	mTravelingThroughTerrain(false),
	mDistanceBetweenChromosomesTooLarge(false)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PostPhysics;

	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	RootComponent = SceneComponent;

	//mTextRenderComponent = CreateDefaultSubobject<UTextRenderComponent>(TEXT("TextRenderComponent"));

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
			float width = 2.0f;

			if (mIsInObstacle || mSlopeTooIntense || mTravelingThroughTerrain || mDistanceBetweenChromosomesTooLarge)
				width = 1.0f;

			DrawDebugLine(GetWorld(), mGeneticRepresentation[i], mGeneticRepresentation[i + 1], mColor, true, 0.1f, 0, width);
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
	mGeneticRepresentation.Empty();
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
	if (mGeneticRepresentation.IsValidIndex(inIndex))
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
void APath::MutateThroughTranslation(const ETranslationMutationType inTranslationMutationType, const float inMaxTranslationOffset)
{
	if (inTranslationMutationType == ETranslationMutationType::AllAtOnce) // All chromosomes (except the first one) are mutated
	{
		for (int32 i = 1; i < mGeneticRepresentation.Num(); ++i)
		{
			mGeneticRepresentation[i] += FVector(FMath::FRandRange(-inMaxTranslationOffset, inMaxTranslationOffset), FMath::FRandRange(-inMaxTranslationOffset, inMaxTranslationOffset), 0.0f);
		}
	}
	else if (inTranslationMutationType == ETranslationMutationType::AnyButStart) // A random chromosome (except the first one) is mutated
	{
		const int32 chromosome_to_mutate_index = FMath::RandRange(1, mGeneticRepresentation.Num() - 1);
		mGeneticRepresentation[chromosome_to_mutate_index] += FVector(FMath::FRandRange(-inMaxTranslationOffset, inMaxTranslationOffset), FMath::FRandRange(-inMaxTranslationOffset, inMaxTranslationOffset), 0.0f);
	}
	else if (inTranslationMutationType == ETranslationMutationType::HeadFalloff) // The final chromosome, all other chromosomes are mutated in the same way but with linear falloff applied
	{
		FVector offset = FVector(FMath::FRandRange(-inMaxTranslationOffset, inMaxTranslationOffset), FMath::FRandRange(-inMaxTranslationOffset, inMaxTranslationOffset), 0.0f);

		for (int32 i = mGeneticRepresentation.Num() - 1; i > 1; --i)
		{
			const float multiplier = i / (float)(mGeneticRepresentation.Num() - 1);
			mGeneticRepresentation[i] += offset * multiplier;
		}
	}
	else if (inTranslationMutationType == ETranslationMutationType::HeadOnly) // The final chromsome is mutated
	{
		mGeneticRepresentation.Last() += FVector(FMath::FRandRange(-inMaxTranslationOffset, inMaxTranslationOffset), FMath::FRandRange(-inMaxTranslationOffset, inMaxTranslationOffset), 0.0f);
	}
}



/**
* Inserts a point / chromosome in the genetic representation
* Insertion happens anywhere after the first chromosome
* The chromosome that gets added will be in the middle of the the element before inserting and the previous
*/
void APath::MutateThroughInsertion()
{
	if (mGeneticRepresentation.Num() >= 2)
	{
		const uint32 insertion_index = FMath::RandRange(1, mGeneticRepresentation.Num() - 1);
		if (mGeneticRepresentation.IsValidIndex(insertion_index) && mGeneticRepresentation.IsValidIndex(insertion_index - 1))
		{
			FVector mid_point = (mGeneticRepresentation[insertion_index] + mGeneticRepresentation[insertion_index - 1]) / 2.0f;
			mGeneticRepresentation.Insert(mid_point, insertion_index);
		}
	}
}



/**
* Removes a point / chromosome from the genetic representation
* Removal happens anywhere after the first chromosome, which means the head may be killed off
*/
void APath::MutateThroughDeletion()
{
	if (mGeneticRepresentation.Num() > 2)
	{
		const uint32 deletion_index = FMath::RandRange(1, mGeneticRepresentation.Num() - 1);
		RemoveChromosome(deletion_index);
	}
}


/**
* If possible, forces the path to snap its chromosomes to a terrain
*/
void APath::SnapToTerrain()
{
	for (int32 i = 1; i < mGeneticRepresentation.Num(); ++i)
	{
		if (GetWorld() != nullptr)
		{
			FHitResult positive_vertical_hit_result;
			if (GetWorld()->LineTraceSingleByChannel(positive_vertical_hit_result, mGeneticRepresentation[i], mGeneticRepresentation[i] + FVector(0.0f, 0.0f, 100.0f), ECollisionChannel::ECC_GameTraceChannel3))
				mGeneticRepresentation[i] = positive_vertical_hit_result.Location;
				
			FHitResult negative_vertical_hit_result;
			if (GetWorld()->LineTraceSingleByChannel(negative_vertical_hit_result, mGeneticRepresentation[i], mGeneticRepresentation[i] + FVector(0.0f, 0.0f, -100.0f), ECollisionChannel::ECC_GameTraceChannel3))
				mGeneticRepresentation[i] = negative_vertical_hit_result.Location;
		}
	}
}