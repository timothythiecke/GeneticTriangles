// Fill out your copyright notice in the Description page of Project Settings.

#include "GeneticTriangles.h"
#include "PathManager.h"

#include "Path.h"

// Sets default values
APathManager::APathManager() :
	SceneComponent(nullptr),
	PopulationCount(0),
	MaxInitialVariation(40.0f),
	MinAmountOfPointsPerPathAtStartup(5),
	MaxAmountOfPointsPerPathAtStartup(5),
	TimeBetweenGenerations(1.0f),
	CrossoverProbability(70.0f),
	MutationProbability(5.0f),
	mTimer(TimeBetweenGenerations),
	AverageFitness(0.0f),
	AmountOfNodesWeight(100.0f),
	ProximityToTargetedNodeWeight(100.0f),
	LengthWeight(100.0f)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Exposes the scene component so we may actually move the actor in the scene
	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	RootComponent = SceneComponent;
}

// Called when the game starts or when spawned
void APathManager::BeginPlay()
{
	Super::BeginPlay();
	
	// Create population
	mPaths.Empty();
	mPaths.Reserve(PopulationCount);

	for (int32 i = 0; i < PopulationCount; ++i)
	{
		APath* path = GetWorld()->SpawnActor<APath>(GetTransform().GetLocation(), GetTransform().GetRotation().Rotator());

		ensure(path != nullptr);

		path->PostInit(MinAmountOfPointsPerPathAtStartup, MaxAmountOfPointsPerPathAtStartup);
		path->RandomizeValues(Nodes[0], MaxInitialVariation);
		path->DetermineGeneticRepresentation();

		mPaths.Add(path);
	}
}

// Called every frame
void APathManager::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

	// Start a countdown so we run a generation each interval
	mTimer -= DeltaTime;
	if (mTimer < 0.0f)
	{
		mTimer = TimeBetweenGenerations;

		RunGeneration();
	}
}


// Allow dispose handling before destructing
void APathManager::Dispose()
{
	this->Destroy();
}



void APathManager::RunGeneration()
{
	if (GEngine != nullptr)
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("Starting run for generation ") + FString::FromInt(GenerationCount));

	EvaluateFitness();
	SelectionStep();
	CrossoverStep();
	MutationStep();
	EvaluateFitness();
	ColorCodePathsByFitness();

	++GenerationCount;

	// Write empty line
	if (GEngine != nullptr)
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT(""));
}


void APathManager::EvaluateFitness()
{
	if (GEngine != nullptr)
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("Starting fitness evaluation..."));

	// What defines fitness for a path?
	// 1. SHORTEST / CLOSEST
	// -> Amount of chunks per path (less chunks == more fitness)
	// -> Length of a path (shorter l => higher f)
	// -> Distance of the final node in relation to the targeted node
	// -> Average orientation of the path

	// Fitness is calculated as an agreation of multiple fitness values

	// /////////////
	// 1. CACHE DATA
	// /////////////
	// Determine the least and most amount of nodes as this will influence the way fitness is calculated as well
	int32 least_amount_of_nodes = INT32_MAX;
	int32 most_amount_of_nodes = 0;

	// Same goes for the distances between the final point of a path and the targetted node
	float closest_distance = TNumericLimits<float>::Max();
	float furthest_distance = 0.0f;
	FVector targetting_location = Nodes[1]->GetActorLocation(); // Assumes this has been filled in through the editor

	// And again, same goes for the length of the path
	float shortest_path_length = TNumericLimits<float>::Max();
	float longest_path_length = 0.0f;

	for (int32 i = 0; i < mPaths.Num(); ++i)
	{
		APath* path = nullptr;

		if (mPaths.IsValidIndex(i) && mPaths[i]->IsValidLowLevelFast())
		{
			path = mPaths[i];

			// Node amount calculation
			const int32 node_amount = path->GetAmountOfNodes();

			if (node_amount < least_amount_of_nodes)
				least_amount_of_nodes = node_amount;

			if (node_amount > most_amount_of_nodes)
				most_amount_of_nodes = node_amount;

			// Distance calculations
			const float distance_to_targetting_node = (targetting_location - path->GetLocationOfFinalNode()).Size();

			if (distance_to_targetting_node < closest_distance)
				closest_distance = distance_to_targetting_node;

			if (distance_to_targetting_node > furthest_distance)
				furthest_distance = distance_to_targetting_node;

			// Length calculation
			const float path_length = path->GetLength();
			
			if (path_length < shortest_path_length)
				shortest_path_length = path_length;

			if (path_length > longest_path_length)
				longest_path_length = path_length;
		}
		else
			UE_LOG(LogTemp, Warning, TEXT("APathManager::EvaluateFitness >> mPaths contains an invalid APath* at index %d"), i);
	}
	
	// TODO: Currently should ALWAYS result in 5


	// ///////////////////////////////
	// 2. CALCULATE AND ASSIGN FITNESS
	// ///////////////////////////////
	mTotalFitness = 0.0f;
	
	for (int32 i = 0; i < mPaths.Num(); ++i)
	{
		APath* path = nullptr;

		if (mPaths.IsValidIndex(i) && mPaths[i]->IsValidLowLevelFast())
		{
			path = mPaths[i];

			// Final node in relation to targetting node
			// Length
			// Amount of nodes

			// Blend value
			// Y = (X- X0) / (X1 - X0)

			// Need zero handling
			float node_amount_blend_value = 0.0f;
			if (least_amount_of_nodes - most_amount_of_nodes != 0)
			{
				node_amount_blend_value = (path->GetAmountOfNodes() - most_amount_of_nodes) / (least_amount_of_nodes - most_amount_of_nodes);
			}

			float proximity_blend_value = 0.0f;
			if (FMath::Abs(closest_distance - furthest_distance) > 0.1f)
			{
				proximity_blend_value = ((targetting_location - path->GetLocationOfFinalNode()).Size() - furthest_distance) / (closest_distance - furthest_distance);
			}

			float length_blend_value = 0.0f;
			if (FMath::Abs(shortest_path_length - longest_path_length) > 0.1f)
			{
				length_blend_value = (path->GetLength() - longest_path_length) / (shortest_path_length - longest_path_length);
			}

			const float final_fitness = (AmountOfNodesWeight * node_amount_blend_value) + 
										(ProximityToTargetedNodeWeight * proximity_blend_value) +
										(LengthWeight * length_blend_value);

			path->SetFitness(final_fitness);
			
			mTotalFitness += final_fitness;
		}
		else
			UE_LOG(LogTemp, Warning, TEXT("APathManager::EvaluateFitness >> mPaths contains an invalid APath* at index %d"), i);
	}

	AverageFitness = mTotalFitness / mPaths.Num();
	
	// ////////////////////////////////////
	// 3. SORT PATHS BY FITNESS, DESCENDING
	// ////////////////////////////////////
	mPaths.Sort([&](const APath& lhs, const APath& rhs) 
	{
		return lhs.GetFitness() > rhs.GetFitness();
	});
}



void APathManager::SelectionStep()
{
	if (GEngine != nullptr)
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Orange, TEXT("Starting selection step..."));

	mMatingPaths.Empty();
	mMatingPaths.Reserve(PopulationCount);

	// Still roulette wheel sampling
	while (mMatingPaths.Num() < PopulationCount)
	{
		const float R = FMath::FRand();
		float accumulated_fitness = 0.0f;

		for (int32 i = 0; i < mPaths.Num(); ++i)
		{
			APath* path = nullptr;

			if (mPaths.IsValidIndex(i) && mPaths[i]->IsValidLowLevelFast())
			{
				path = mPaths[i];

				accumulated_fitness += path->GetFitness() / mTotalFitness;

				if (accumulated_fitness >= R)
				{
					mMatingPaths.Add(path);
					break;
				}
			}
			else
				UE_LOG(LogTemp, Warning, TEXT("APathManager::SelectionStep >> mPaths contains an invalid APath* at index %d"), i);
		}
	}

}



void APathManager::CrossoverStep()
{
	if (GEngine != nullptr)
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Crossover step..."));

	TArray<APath*> temp;
	temp.Reserve(PopulationCount);

	int32 successfull_crossover_amount = 0;

	for (int32 i = 0; i < mMatingPaths.Num(); i += 2)
	{
		const float R = FMath::FRandRange(0.0f, 100.0f);

		if (R >= (100.0f - CrossoverProbability))
		{
			const APath* current_path = mMatingPaths[i];
			const APath* next_path = mMatingPaths[i + 1];
			const APath* smallest_path = current_path;
			const APath* bigger_path = next_path;

			// Compare the two paths based on their node amount
			// Operator < might be better for this
			if (current_path->GetAmountOfNodes() > next_path->GetAmountOfNodes())
			{
				smallest_path = next_path;
				bigger_path = current_path;
			}

			// Code below can be put in a switch
			if (CrossoverOperator == ECrossoverOperator::SinglePoint)
			{
				const int crossover_point = FMath::RandRange(0, smallest_path->GetAmountOfNodes());
				APath* offspring_0 = GetWorld()->SpawnActor<APath>(GetTransform().GetLocation(), GetTransform().GetRotation().Rotator());;
				APath* offspring_1 = GetWorld()->SpawnActor<APath>(GetTransform().GetLocation(), GetTransform().GetRotation().Rotator());;

				int32 index = 0;
				for (const FVector& ref : bigger_path->GetGeneticRepresentation())
				{
					// Junk data evaluation
					if (index >= smallest_path->GetAmountOfNodes())
					{
						const float appending_chance = FMath::RandRange(0.0f, 100.0f);

						if (appending_chance < JunkDNACrossoverProbability)
							offspring_0->AddChromosome(bigger_path->GetChromosome(index));

						const float appending_chance_next = FMath::RandRange(0.0f, 100.0f);
						if (appending_chance_next < JunkDNACrossoverProbability)
							offspring_1->AddChromosome(bigger_path->GetChromosome(index));
					}
					else
					{
						if (index < crossover_point)
						{
							offspring_0->AddChromosome(smallest_path->GetChromosome(index));
							offspring_1->AddChromosome(bigger_path->GetChromosome(index));
						}
						else
						{
							offspring_0->AddChromosome(bigger_path->GetChromosome(index));
							offspring_1->AddChromosome(smallest_path->GetChromosome(index));
						}
					}

					++index;
				}

				offspring_0->DetermineGeneticRepresentation();
				offspring_1->DetermineGeneticRepresentation();

				temp.Add(offspring_0);
				temp.Add(offspring_1);

				++successfull_crossover_amount;
			}
			else if (CrossoverOperator == ECrossoverOperator::Uniform)
			{
				APath* offspring_0 = GetWorld()->SpawnActor<APath>(GetTransform().GetLocation(), GetTransform().GetRotation().Rotator());;
				APath* offspring_1 = GetWorld()->SpawnActor<APath>(GetTransform().GetLocation(), GetTransform().GetRotation().Rotator());;

				int32 index = 0;
				for (const FVector& ref : bigger_path->GetGeneticRepresentation())
				{
					if (index >= smallest_path->GetAmountOfNodes())
					{
						// Evaluate junk data
						// Both offsrping have a shot of copying the junk data of the less fit parent
						const float junk_chance = FMath::FRandRange(0.0f, 100.0f);
						if (junk_chance < JunkDNACrossoverProbability)
							offspring_0->AddChromosome(ref);

						const float junk_chance_next = FMath::FRandRange(0.0f, 100.0f);
						if (junk_chance_next < JunkDNACrossoverProbability)
							offspring_1->AddChromosome(ref);
					}
					else
					{
						// Uniform does crossover per chromosome
						const float bias = FMath::FRandRange(0.0f, 100.0f);
						if (bias < 50.0f)
						{
							offspring_0->AddChromosome(smallest_path->GetChromosome(index));
							offspring_1->AddChromosome(bigger_path->GetChromosome(index));
						}
						else
						{
							offspring_0->AddChromosome(bigger_path->GetChromosome(index));
							offspring_1->AddChromosome(smallest_path->GetChromosome(index));
						}
					}

					++index;
				}

				offspring_0->DetermineGeneticRepresentation();
				offspring_1->DetermineGeneticRepresentation();

				temp.Add(offspring_0);
				temp.Add(offspring_1);

				++successfull_crossover_amount;
			}
		}
		else
		{
			// Unable to crossover
			// Parents are duplicated to the next generation
			APath* duplicate_0 = GetWorld()->SpawnActor<APath>(GetTransform().GetLocation(), GetTransform().GetRotation().Rotator());
			duplicate_0->SetGeneticRepresentation(mMatingPaths[i]->GetGeneticRepresentation());
			duplicate_0->DetermineGeneticRepresentation();
			temp.Add(duplicate_0);

			APath* duplicate_1 = GetWorld()->SpawnActor<APath>(GetTransform().GetLocation(), GetTransform().GetRotation().Rotator());
			duplicate_1->SetGeneticRepresentation(mMatingPaths[i+1]->GetGeneticRepresentation());
			duplicate_1->DetermineGeneticRepresentation();
			temp.Add(duplicate_1);
		}
	}

	Purge();

	mPaths = temp;

	if (GEngine != nullptr)
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Amount of crossovers: ") +FString::FromInt(successfull_crossover_amount));
}


void APathManager::MutationStep()
{
	// Mutate a node
	// Mutate multiple nodes
	// Insert a node
	// Delete a node

	if (GEngine != nullptr)
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Orange, TEXT("Mutation step..."));

	int32 successfull_mutations = 0;

	for (APath* path : mPaths)
	{
		const float rand = FMath::FRandRange(0.0f, 100.0f);

		if (rand < MutationProbability)
		{
			path->Mutate();
			++successfull_mutations;
		}
	}

	if (GEngine != nullptr)
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Orange, TEXT("Amount of mutations: ") + FString::FromInt(successfull_mutations));
}



void APathManager::Purge()
{
	for (int32 i = mMatingPaths.Num() - 1; i > -1; --i)
	{
		if (mMatingPaths.IsValidIndex(i) && mMatingPaths[i]->IsValidLowLevelFast())
			mMatingPaths[i]->Destroy();

		if (mPaths.IsValidIndex(i) && mPaths[i]->IsValidLowLevelFast())
			mPaths[i]->Destroy();
	}
}



void APathManager::ColorCodePathsByFitness()
{
	float lowest_fitness = TNumericLimits<float>::Max();
	float highest_fitness = 0.0f;

	for (const APath* path : mPaths)
	{
		const float fitness = path->GetFitness();

		if (fitness < lowest_fitness)
			lowest_fitness = fitness;
		if (fitness > highest_fitness)
			highest_fitness = fitness;
	}

	for (int32 i = 0; i < mPaths.Num(); ++i)
	{
		const float fitness = mPaths[i]->GetFitness();
		const float blend_value = (fitness - highest_fitness) / (lowest_fitness - highest_fitness);

		FColor red = FColor::Red;
		FColor green = FColor::Green;

		FColor blended;
		blended.A = 255;
		blended.R = FMath::Lerp(red.R, green.R, blend_value * 255);
		blended.G = FMath::Lerp(red.G, green.G, blend_value * 255);
		blended.B = FMath::Lerp(red.B, green.B, blend_value * 255);

		mPaths[i]->SetColorCode(blended);
	}
}