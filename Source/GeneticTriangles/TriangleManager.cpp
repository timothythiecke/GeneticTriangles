// Fill out your copyright notice in the Description page of Project Settings.

#include "GeneticTriangles.h"
#include "TriangleManager.h"

#include "Triangle.h"

// Sets default values
ATriangleManager::ATriangleManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ATriangleManager::BeginPlay()
{
	Super::BeginPlay();

	mActualMutationRate = MutationRate;
}

// Called every frame
void ATriangleManager::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

	if (mUsesMutationRateBalancing && AverageFitness > MutationRateThreshold)
	{
		// output = output_start + ((output_end - output_start) / (input_end - input_start)) * (input - input_start)

		mActualMutationRate = MutationRate + ((0.0f - MutationRate) / (1.0f - 0.5f)) * (AverageFitness - 0.5f);
	}
	else
		mActualMutationRate = MutationRate;
}


// Starts the genetic algorithm
void ATriangleManager::InitializePopulation()
{
	// Create a set amount of fenotypes from which the genetic algorithm shall start

	mTriangles.Reserve(PopulationSize);

	FVector x;
	FVector y;

	for (int32 i = 0; i < PopulationSize; ++i)
	{
		FTransform transform;

		if (i != 0 && (i % 10 == 0))
		{
			// Reset X
			// Shift Y
			x.X = 0;
			y.Y += 50.0f;
		}

		transform.SetLocation(x + y);

		x.X += 50.0f;

		ATriangle* triangle_ptr = GetWorld()->SpawnActor<ATriangle>(x + y, GetTransform().GetRotation().Rotator());

		ensure(triangle_ptr != nullptr);

		//triangle_ptr->mLocation = x + y;

		triangle_ptr->PostInit();
		triangle_ptr->DetermineGeneticRepresentation();

		//triangle_ptr->SetActorLocation(x + y);

		mTriangles.Add(triangle_ptr);
	}

	// Assume that we created the exact amount of triangles as desired
	ensure(mTriangles.Num() == PopulationSize);

	if (GEngine != nullptr)
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Emerald, TEXT("Initialized triangles successfully"));
}



void ATriangleManager::EvaluateFitness()
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
	mMappedTrianglesContiguous.Reserve(PopulationSize);

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
	
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, TEXT("Sorted triangles | fitness | descending | normalized"));
}


// Performance is currently extremely fast
void ATriangleManager::SelectionStep()
{
	// Generate a random number R between 0 and 1
	// Step over the triangles
	// Sum of all triangles (accumulation)
	// Accumulated value greater than random number? -> select for breeding

	// Keep doing this until all have been removed from the mapped array

	// Roulette wheel selection
	mTrianglesSortedByMatingOrder.Empty();
	mTrianglesSortedByMatingOrder.Reserve(PopulationSize);

	float lost_fitness = 0.0f;
	while (mMappedTrianglesContiguous.Num() > 0)
	{
		const float R = FMath::RandRange(0.0f, 1.0f - lost_fitness); // We normalized the fitness values
		float accumulated_fitness = 0.0f;

		MappedTriangle triangle_to_remove(nullptr, -1.0f);

		for (int32 i = 0; i < mMappedTrianglesContiguous.Num(); ++i)
		{
			accumulated_fitness += mMappedTrianglesContiguous[i].fitness;

			if (accumulated_fitness >= R)
			{
				triangle_to_remove = mMappedTrianglesContiguous[i];
			
				lost_fitness += triangle_to_remove.fitness;
				break;
			}
		}

		if (triangle_to_remove.ptr == nullptr)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Detected non normalized fitness value"));
			break;
		}

		mTrianglesSortedByMatingOrder.Add(triangle_to_remove.ptr);
		mMappedTrianglesContiguous.Remove(triangle_to_remove);
	}

	//ensure(mTrianglesSortedByMatingOrder.Num() == PopulationSize);
	if (mTrianglesSortedByMatingOrder.Num() == PopulationSize)
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, TEXT("Triangles have been sorted by mating order ") + FString::FromInt(mTrianglesSortedByMatingOrder.Num()));
	else
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("Something went wrong with sorting the triangles by their mating order!"));
}



void ATriangleManager::CrossoverStep()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, TEXT("Starting crossover algorithm"));
	
	// Selection of which chrosome as well

	// Single point
	// Double point
	// Cut and splice -> not possible with current setup
	// Three parent crossover -> not possible with current sorting and arrays and stuff

	// So for now let's go with single point crossover

	// Single point for segment (IE: per point)
	// Single point per gene
	// Should we use only one single point value, per chromosome, per gene etc...
	// All of this should be able to be changed by the user

	// For now, we will choose single point crossover
	// For each chromosome or gene use this crossover

	TArray<ATriangle*> temp;
	temp.Reserve(PopulationSize);

	for (int32 i = 0; i < mTrianglesSortedByMatingOrder.Num(); i += 2) // += 2 because we mate by pairs
	{
		TArray<float> new_genetic_representation_for_first_child;
		new_genetic_representation_for_first_child.Reserve(9);

		TArray<float> new_genetic_representation_for_second_child;
		new_genetic_representation_for_second_child.Reserve(9);

		auto gen_rep_0 = mTrianglesSortedByMatingOrder[i]->GetGeneticRepresentation();
		auto gen_rep_1 = mTrianglesSortedByMatingOrder[i+1]->GetGeneticRepresentation(); // Care for out of bounds (should be fine)

		for (int32 j = 0; j < gen_rep_0.Num(); j += 3) // Jump to next location
		{
			const float crossover_point = FMath::RandRange(0.0f, 1.0f);
			
			new_genetic_representation_for_first_child.Add(FMath::Lerp(gen_rep_0[j], gen_rep_1[j], crossover_point));
			new_genetic_representation_for_first_child.Add(FMath::Lerp(gen_rep_0[j+1], gen_rep_1[j+1], crossover_point));
			new_genetic_representation_for_first_child.Add(FMath::Lerp(gen_rep_0[j+2], gen_rep_1[j+2], crossover_point));

			new_genetic_representation_for_second_child.Add(FMath::Lerp(gen_rep_0[j], gen_rep_1[j], 1.0f - crossover_point));
			new_genetic_representation_for_second_child.Add(FMath::Lerp(gen_rep_0[j + 1], gen_rep_1[j + 1], 1.0f - crossover_point));
			new_genetic_representation_for_second_child.Add(FMath::Lerp(gen_rep_0[j + 2], gen_rep_1[j + 2], 1.0f - crossover_point));
		}

		ATriangle* first_child_triangle = GetWorld()->SpawnActor<ATriangle>(GetTransform().GetLocation(), GetTransform().GetRotation().Rotator());
		first_child_triangle->SetGeneticRepresentation(new_genetic_representation_for_first_child);
		first_child_triangle->ReconstructFromGeneticRepresentation();

		ATriangle* second_child_triangle = GetWorld()->SpawnActor<ATriangle>(GetTransform().GetLocation(), GetTransform().GetRotation().Rotator());
		second_child_triangle->SetGeneticRepresentation(new_genetic_representation_for_second_child);
		second_child_triangle->ReconstructFromGeneticRepresentation();

		temp.Add(first_child_triangle);
		temp.Add(second_child_triangle);
	}

	PurgeOld();

	mTriangles = temp;

	if (mTriangles.Num() == temp.Num())
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("Successfully did single point crossover!"));
}



void ATriangleManager::MutationStep()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, TEXT("Starting mutation algorithm"));

	// Consider mutation for all individuals
	// Consider mutation for select individuals
	// Consider mutation for only one individual

	// Mutation rate decreses when average fitness heightens

	// Consider mutation for one chromosome
	// Consider mutation for all chromosomes
	
	// Consider mutation for one gene
	// Consider mutation for all genes

	// Consider different mutation chance for any of the above
	// Consider only a fixed mutation chance for any of the above

	// For now, go with any of the population may mutate
	// But only mutate one point
	for (ATriangle* triangle : mTriangles)
	{
		const float chance = FMath::RandRange(0.0f, 100.0f);

		if (chance < mActualMutationRate)
		{
			const TArray<FVector>& points = triangle->GetPoints();
			const int mutating_point_index = FMath::RandRange(0, points.Num()-1);

			triangle->MutateChromosome(mutating_point_index, MaxMutationAxisOffset);
			triangle->DetermineGeneticRepresentation();
			triangle->ReconstructFromGeneticRepresentation();

			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, TEXT("Triangle has mutated!"));
		}
	}
}



void ATriangleManager::PurgeOld()
{
	for (int i = mTrianglesSortedByMatingOrder.Num() - 1; i > -1; --i)
	{
		mTrianglesSortedByMatingOrder[i]->Destroy();
	}
}



void ATriangleManager::GenerateNew()
{

}