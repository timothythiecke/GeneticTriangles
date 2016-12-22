// Fill out your copyright notice in the Description page of Project Settings.

#include "GeneticTriangles.h"
#include "UpdatedTriangleManager.h"

#include "Triangle.h"

// Sets default values
AUpdatedTriangleManager::AUpdatedTriangleManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AUpdatedTriangleManager::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void AUpdatedTriangleManager::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

	if (mTriangles.Num() > 0)
	{
		mConstTimer -= DeltaTime;

		if (mConstTimer < 0.0f)
		{
			mConstTimer = GenerationDelay;
			RunGeneration();
		}
	}
}



void AUpdatedTriangleManager::Initialize()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, TEXT("Hello world"));

	mTriangles.Reserve(PopulationCount);

	for (int32 i = 0; i < PopulationCount; ++i)
	{
		FTransform transform;

		ATriangle* triangle_ptr = GetWorld()->SpawnActor<ATriangle>(transform.GetLocation(), transform.GetRotation().Rotator());
		ensure(triangle_ptr != nullptr);
		triangle_ptr->PostInit();
		triangle_ptr->DetermineGeneticRepresentation();

		mTriangles.Add(triangle_ptr);
	}

	// Assume that we created the exact amount of triangles as desired
	ensure(mTriangles.Num() == PopulationCount);

	FMath::RandInit(RandomSeed);

	if (GEngine != nullptr)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Emerald, TEXT("Initialized triangles successfully"));
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Emerald, TEXT("Random seed applied"));
	}
}



void AUpdatedTriangleManager::RunGeneration()
{
	EvaluateFitness();
	SelectionStep();
	CrossoverStep();
	MutationStep();

	++GenerationCount;

	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::White, TEXT("Finished generation ") + FString::FromInt(GenerationCount));
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Black, TEXT(""));
}



void AUpdatedTriangleManager::EvaluateFitness()
{
	// Fitness will be evaluated
	// The triangle will be more fit if one of the angles between one of it's segments is close to 90 degrees.

	// We are targetting a dot product of zero (== perpendicular)
	// Filter out the dot product which is closest to zero (use abs)
	// One minus the dot product so we map them correctly 0.0f (least fit) -> 1.0f (most fit)
	// Map the triangle pointers to their fitness value
	// Sort this map by value

	float total_fitness = 0.0f;

	mMappedTrianglesContiguous.Empty();
	mMappedTrianglesContiguous.Reserve(PopulationCount);

	for (const ATriangle* triangle : mTriangles)
	{
		if (triangle != nullptr)
		{
			const TArray<FVector>& points = triangle->GetPoints();

			ensure(points.IsValidIndex(0) && points.IsValidIndex(1) && points.IsValidIndex(2));

			// @TODO: Important to actually use vectors...

			FVector vec01 = points[1] - points[0]; // Vector 0->1
			FVector vec02 = points[2] - points[0]; // Vector 0->2
			FVector vec12 = points[2] - points[1]; // Vector 1->2

			vec01.Normalize();
			vec02.Normalize();
			vec12.Normalize();

			const float dot0 = FMath::Abs(FVector::DotProduct(vec01, vec02));
			const float dot1 = FMath::Abs(FVector::DotProduct(vec12, -vec01));
			const float dot2 = FMath::Abs(FVector::DotProduct(-vec12, -vec02));

			float smallest = 1.1f; // Dot product will never return anything above 1.0f (even considering abs)

			if (dot0 < smallest)
				smallest = dot0;
			else if (dot1 < smallest)
				smallest = dot1;
			else if (dot2 < smallest)
				smallest = dot2;

			const float fitness_score = 1.0f - smallest;

			ensure(fitness_score >= 0.0f);

			total_fitness += fitness_score;

			mMappedTrianglesContiguous.Add(MappedTriangle(const_cast<ATriangle*>(triangle), fitness_score));
		}
	}

	// Sorts the array by value (descending)
	mMappedTrianglesContiguous.Sort([&](const MappedTriangle& lhs, const MappedTriangle& rhs)
	{
		return lhs.fitness > rhs.fitness;
	});

	// Then normalize the values
	for (int i = 0; i < mMappedTrianglesContiguous.Num(); ++i)
	{
		mMappedTrianglesContiguous[i].fitness /= total_fitness;
	}

	// Calculate average fitness, will be useful to balance the mutation rate
	AverageFitness = total_fitness / mMappedTrianglesContiguous.Num();

	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, TEXT("Sorted triangles | fitness | descending | normalized"));
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, TEXT("Average fitness: ") + FString::SanitizeFloat(AverageFitness));
}



void AUpdatedTriangleManager::SelectionStep()
{
	// Roulette wheel sampling
	// While the number of elements in the mating array is less than the population count
	// Sample the population
	// Replacement is allowed (the more fit solutions have more chance to reproduce)
	
	// When we are done, the amount of elements in the mating order should be equal
	// If less, append a random solution
	// If more, truncate

	mMatingTriangles.Empty();
	mMatingTriangles.Reserve(PopulationCount);

	ATriangle* previous_selected_triangle = nullptr;
	ATriangle* next_selected_triangle = nullptr;

	while (mMatingTriangles.Num() < PopulationCount)
	{
		const float R = FMath::FRand();
		float accumulated_fitness = 0.0f;
		bool continue_to_next_selection = false;

		for (int32 i = 0; i < mMappedTrianglesContiguous.Num(); ++i)
		{
			accumulated_fitness += mMappedTrianglesContiguous[i].fitness;
			next_selected_triangle = mMappedTrianglesContiguous[i].ptr;

			if (accumulated_fitness >= R)
			{
				if (AllowsSelfMating)
				{
					previous_selected_triangle = next_selected_triangle;
					mMatingTriangles.Add(next_selected_triangle);

					continue_to_next_selection = true;
					break;
				}
				else
				{
					if (next_selected_triangle == previous_selected_triangle)
					{
						// @TODO: Need to access pointers or reroll?
					}
				}
			}
		}

		if (continue_to_next_selection)
			continue;
	}

	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("Selected for reproducing: ") + FString::FromInt(mMatingTriangles.Num()));
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("Expected: ") + FString::FromInt(PopulationCount));
}



void AUpdatedTriangleManager::CrossoverStep()
{
	// Loops over the mating array
	// Generate random number R
	// If R > Pc
	// Allow crossover
	// Single point
	// All genes crossover with the same ran

	TArray<ATriangle*> temp;
	temp.Reserve(PopulationCount);

	for (int32 i = 0; i < mMatingTriangles.Num(); i+=2)
	{
		const float R = FMath::FRand();

		TArray<float> gen_0;
		gen_0.Reserve(9);

		TArray<float> gen_1;
		gen_1.Reserve(9);

		auto old_genetic_representation_0 = mMatingTriangles[i]->GetGeneticRepresentation();
		auto old_genetic_representation_1 = mMatingTriangles[i + 1]->GetGeneticRepresentation(); // Care for out of bounds (should be fine)


		if (R >= (1.0f - CrossoverProbability)) // inverse is necessary 
		{
			for (int32 j = 0; j < old_genetic_representation_0.Num(); j += 3) // Jump to next location
			{
				const float crossover_point = FMath::FRand();

				gen_0.Add(FMath::Lerp(old_genetic_representation_0[j], old_genetic_representation_1[j], crossover_point));
				gen_0.Add(FMath::Lerp(old_genetic_representation_0[j + 1], old_genetic_representation_1[j + 1], crossover_point));
				gen_0.Add(FMath::Lerp(old_genetic_representation_0[j + 2], old_genetic_representation_1[j + 2], crossover_point));

				gen_1.Add(FMath::Lerp(old_genetic_representation_0[j], old_genetic_representation_1[j], 1.0f - crossover_point));
				gen_1.Add(FMath::Lerp(old_genetic_representation_0[j + 1], old_genetic_representation_1[j + 1], 1.0f - crossover_point));
				gen_1.Add(FMath::Lerp(old_genetic_representation_0[j + 2], old_genetic_representation_1[j + 2], 1.0f - crossover_point));
			}

			ATriangle* triangle_0 = GetWorld()->SpawnActor<ATriangle>(GetTransform().GetLocation(), GetTransform().GetRotation().Rotator());
			triangle_0->SetGeneticRepresentation(gen_0);
			triangle_0->ReconstructFromGeneticRepresentation();

			ATriangle* triangle_1 = GetWorld()->SpawnActor<ATriangle>(GetTransform().GetLocation(), GetTransform().GetRotation().Rotator());
			triangle_1->SetGeneticRepresentation(gen_1);
			triangle_1->ReconstructFromGeneticRepresentation();

			temp.Add(triangle_0);
			temp.Add(triangle_1);
		}
		else
		{
			// Crossover is not possible
			// Duplicate the parents

			ATriangle* duplicate_0 = GetWorld()->SpawnActor<ATriangle>(GetTransform().GetLocation(), GetTransform().GetRotation().Rotator());
			duplicate_0->SetGeneticRepresentation(old_genetic_representation_0);
			duplicate_0->ReconstructFromGeneticRepresentation();

			ATriangle* duplicate_1 = GetWorld()->SpawnActor<ATriangle>(GetTransform().GetLocation(), GetTransform().GetRotation().Rotator());
			duplicate_1->SetGeneticRepresentation(old_genetic_representation_1);
			duplicate_1->ReconstructFromGeneticRepresentation();

			temp.Add(duplicate_0);
			temp.Add(duplicate_1);
		}
	}
	
	Purge();

	mTriangles = temp;

	if (mTriangles.Num() == temp.Num())
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("Successfully did single point crossover!"));
}



void AUpdatedTriangleManager::MutationStep()
{
	int mutation_count = 0;

	for (ATriangle* triangle : mTriangles)
	{
		const float chance = FMath::FRandRange(0.0f, 100.0f);

		if (chance < MutationProbability)
		{
			++mutation_count;

			const TArray<FVector>& points = triangle->GetPoints();
			const int mutating_point_index = FMath::RandRange(0, points.Num() - 1);

			triangle->MutateChromosome(mutating_point_index, MaxMutationAxisOffset);
			triangle->DetermineGeneticRepresentation();
			triangle->ReconstructFromGeneticRepresentation();
		}
	}
	
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, TEXT("Mutation amount: ") + FString::FromInt(mutation_count));
}



void AUpdatedTriangleManager::Purge()
{
	for (int32 i = mMatingTriangles.Num() - 1; i > -1; --i)
	{
		if (mMatingTriangles.IsValidIndex(i))
			mMatingTriangles[i]->Destroy();

		if (mTriangles.IsValidIndex(i))
			mTriangles[i]->Destroy();
	}
}