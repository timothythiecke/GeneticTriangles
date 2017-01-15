// Fill out your copyright notice in the Description page of Project Settings.

#include "GeneticTriangles.h"
#include "PathManager.h"

#include "Path.h"

// Sets default values
APathManager::APathManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Exposes the scene component so we may actually move the actor in the scene
	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	RootComponent = SceneComponent;
}


// Allow dispose handling before destructing
void APathManager::Dispose()
{
	this->Destroy();
}


// Called when the game starts or when spawned
void APathManager::BeginPlay()
{
	Super::BeginPlay();
}


// Called every frame
void APathManager::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

	if (AutoRun)
		mPreviousAnimationControlState = EAnimationControlState::Play;

	switch (mPreviousAnimationControlState)
	{
	case EAnimationControlState::Play:
		RunGenerationTimer(DeltaTime);
		break;
	case EAnimationControlState::Pause:
		LogGenerationInfo(); // Keep track of information on screen when paused (for now)
		break;
	case EAnimationControlState::Stop:
		StopRun();
		break;
	case EAnimationControlState::Limbo:
		break;
	default:
		break;
	}
}


void APathManager::RunGenerationTimer(const float inDeltaTime)
{
	// TimeBetweenGenerations may be altered anywhere
	mTimer -= inDeltaTime;
	if (mTimer < 0.0f)
	{
		mTimer = TimeBetweenGenerations;
		
		RunGeneration();
	}
}

void APathManager::RunGeneration()
{
	if (GenerationCount == 0)
	{
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

	if (Nodes.IsValidIndex(0) && Nodes.IsValidIndex(1) && Nodes[0]->IsValidLowLevelFast() && Nodes[1]->IsValidLowLevelFast())
	{
		EvaluateFitness();
		SelectionStep();
		CrossoverStep();
		MutationStep();
		EvaluateFitness();
		ColorCodePathsByFitness();

		mGenerationInfo.mGenerationNumber = GenerationCount++;

		LogGenerationInfo();
	}
	else
		UE_LOG(LogTemp, Warning, TEXT("APathManager::RunGeneration() >> One of the nodes is invalid!"));
}


void APathManager::EvaluateFitness()
{
	// What defines fitness for a path?
	// 1. SHORTEST / CLOSEST
	// -> Amount of chunks per path (less chunks == more fitness)
	// -> Length of a path (shorter l => higher f)
	// -> Distance of the final node in relation to the targeted node
	// -> Average orientation of the path

	// Fitness is calculated as an agreation of multiple fitness values

	// /////////////////////////
	// 1. DATA AND STATE CACHING
	// /////////////////////////
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

			check(path != nullptr);

			// Force path to snap to terrain if possible
			path->SnapToTerrain();

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

			// Trace & slope handling
			const TArray<FVector>& genetic_representation = path->GetGeneticRepresentation();
			for (int32 index = 1; index < genetic_representation.Num(); ++index)
			{
				const bool is_world_valid = GetWorld() != nullptr;
				const bool is_current_index_valid = genetic_representation.IsValidIndex(index);
				const bool is_previous_index_valid = genetic_representation.IsValidIndex(index - 1);

				if (is_world_valid && is_current_index_valid && is_previous_index_valid)
				{
					// Check for obstacles between previous and current node
					// If a hit result is detected, either one of the nodes is in an obstacle or an obstacle is blocking the way
					FHitResult obstacle_hit_result;
					if (GetWorld()->LineTraceSingleByChannel(obstacle_hit_result, genetic_representation[index - 1], genetic_representation[index], ECollisionChannel::ECC_GameTraceChannel1))
						path->MarkIsInObstacle();

					// Check for terrain traveling (hidden)
					FHitResult terrain_hit_result;
					if (GetWorld()->LineTraceSingleByChannel(terrain_hit_result, genetic_representation[index - 1], genetic_representation[index], ECollisionChannel::ECC_GameTraceChannel4))
						path->MarkTravelingThroughTerrain();
				}

				// Check if the head is able to see the target
				// This is the case if no obstacles are in the way
				if (is_world_valid && is_current_index_valid && index == genetic_representation.Num() - 1)
				{
					FHitResult hit_result;
					if (!GetWorld()->LineTraceSingleByChannel(hit_result, genetic_representation[index], Nodes.IsValidIndex(1) ? Nodes[1]->GetActorLocation() : FVector::ZeroVector, ECollisionChannel::ECC_GameTraceChannel2))
						path->MarkCanSeeTarget();
				}

				// Check if the path has reached the target
				if ((Nodes[1]->GetActorLocation() - genetic_representation.Last()).Size() < 100.0f) // @TODO: Magic value, need radius of target node
					path->MarkHasReachedTarget();

				// Check if the slope between this node and the previous is inbetween the expected bounds
				// Use dot product calculation between the vector between the two points and a vector with a constant Z
				if (UseSlopeFitnessEvaluation)
				{
					if (is_current_index_valid && is_previous_index_valid)
					{
						FVector direction = genetic_representation[index] - genetic_representation[index - 1];
						FVector collapsed_vector = direction;
						collapsed_vector.Z = 0.0f;

						direction.Normalize();
						collapsed_vector.Normalize();

						const float dot_product = FVector::DotProduct(direction, collapsed_vector);
						const float radians = FMath::Acos(dot_product);
						const float degrees = FMath::RadiansToDegrees(radians);

						// Check if radians are radians and degrees are degrees, as Acos doesn't specify what the return value is
						// Check Kismet
						// UE_LOG(LogTemp, Warning, TEXT("Radians: %f Degrees: %f"), radians, degrees);

						if (degrees > MaxSlopeToleranceAngle)
							path->MarkSlopeTooIntense();
					}
				}

				// Obstacle avoidance
				if (ApplyObstacleAvoidanceLogic)
				{
					if (GetWorld() != nullptr && is_current_index_valid)
					{
						TArray<FVector> trace_ends;
						
						if (TraceBehaviour == EObstacleTraceBehaviour::WindDirectionTracing)
						{
							trace_ends.Reserve(8);

							trace_ends.Add(FVector(1.0f, 0.0f, 0.0f) * TraceDistance); // East
							trace_ends.Add(FVector(1.0f, -1.0f, 0.0f) * TraceDistance); // South-East
							trace_ends.Add(FVector(0.0f, -1.0f, 0.0f) * TraceDistance); // South
							trace_ends.Add(FVector(-1.0f, -1.0f, 0.0f) * TraceDistance); // South-West
							trace_ends.Add(FVector(-1.0f, 0.0f, 0.0f) * TraceDistance); // West
							trace_ends.Add(FVector(-1.0f, 1.0f, 0.0f) * TraceDistance); // North-West
							trace_ends.Add(FVector(0.0f, 1.0f, 0.0f) * TraceDistance); // North
							trace_ends.Add(FVector(1.0f, 1.0f, 0.0f) * TraceDistance); // North-East

						}
						else
						{
							//@TODO
						}

						FVector start = genetic_representation[index];
						for (const FVector& end : trace_ends)
						{
							FHitResult hit_result;
							if (GetWorld()->LineTraceSingleByChannel(hit_result, start, start + end, ECollisionChannel::ECC_GameTraceChannel1))
								path->AddObstacleHitMultiplierChunk(0.125f);

							DrawDebugPoint(GetWorld(), start + end, 10, FColor::Cyan);
						}
					}
				}

				// Max length fitness
				if (UseMaxLengthFitness && is_current_index_valid && is_previous_index_valid)
				{
					const float length = (genetic_representation[index] - genetic_representation[index - 1]).Size();
					if (length > MaxEuclidianDistance)
						path->MarkDistanceBetweenChromosomesTooLarge();
				}
			}
		}
		else
			UE_LOG(LogTemp, Warning, TEXT("APathManager::EvaluateFitness >> mPaths contains an invalid APath* at index %d"), i);
	}
	
	// TODO: Currently should ALWAYS result in 5


	// ///////////////////////////////
	// 2. CALCULATE AND ASSIGN FITNESS
	// ///////////////////////////////
	mTotalFitness = 0.0f;
	int32 amount_of_nodes = 0;
	int32 offenders = 0;
	for (int32 i = 0; i < mPaths.Num(); ++i)
	{
		APath* path = nullptr;

		if (mPaths.IsValidIndex(i) && mPaths[i]->IsValidLowLevelFast())
		{
			path = mPaths[i];

			check(path != nullptr);

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

			// Determine if the path is able to see the target node
			float can_see_target_fitness = 0.0f;
			if (path->GetCanSeeTarget())
				can_see_target_fitness = CanSeeTargetWeight;
			
			// Path has reached target, mark fit
			float target_reached_fitness = 0.0f;
			if (path->GetHasReachedTarget())
				target_reached_fitness = TargetReachedWeight;

			// Should the path hit an obstacle, mark it unfit
			float obstacle_multiplier = 1.0f;
			if (path->GetIsInObstacle())
				obstacle_multiplier = ObstacleHitMultiplier;

			// Slope too intense for the path to continue on, mark unfit
			float slope_too_intense_multiplier = 1.0f;
			if (UseSlopeFitnessEvaluation && path->GetSlopeTooIntense())
				slope_too_intense_multiplier = SlopeTooIntenseMultiplier;

			// Path traveling through terrain?
			float traveling_through_terrain_multiplier = 1.0f;
			if (UseSlopeFitnessEvaluation && path->GetTravelingThroughTerrain())
				traveling_through_terrain_multiplier = PiercesTerrainMultiplier;

			// Obstacle avoidance?
			// @TODO
			float obstacle_avoidance_multiplier = 1.0f;
			float obstacle_avoidance_weight = 0.0f;
			if (ApplyObstacleAvoidanceLogic)
			{
				if (path->GetObstacleHitMultiplierChunk() > 0.0f)
				{
					obstacle_avoidance_multiplier = 0.0f;
					obstacle_avoidance_weight = 0.0f;
					++offenders;
				}
				else
				{
					obstacle_avoidance_weight = 100.0f;
				}
			}

			// Distance between points too large?
			float max_length_multiplier = 1.0f;
			if (UseMaxLengthFitness)
			{
				if (path->GetDistanceBetweenChromosomesTooLarge())
					max_length_multiplier = EuclidianOvershootMultiplier;
			}

			// Calculate final fitness based on the various weights and multipliers
			const float weight_fitness = ((AmountOfNodesWeight * node_amount_blend_value) +
											(ProximityToTargetedNodeWeight * proximity_blend_value) +
											(LengthWeight * length_blend_value) +
											can_see_target_fitness +
											target_reached_fitness +
											SlopeWeight +
											obstacle_avoidance_weight);
			const float weight_multiplier = obstacle_multiplier * slope_too_intense_multiplier * traveling_through_terrain_multiplier * max_length_multiplier;
			const float final_fitness = weight_fitness * weight_multiplier;

			path->SetFitnessValues(final_fitness, AmountOfNodesWeight * node_amount_blend_value);
			
			mTotalFitness += final_fitness;
			amount_of_nodes += path->GetGeneticRepresentation().Num();
		}
		else
			UE_LOG(LogTemp, Warning, TEXT("APathManager::EvaluateFitness >> mPaths contains an invalid APath* at index %d"), i);
	}

	//UE_LOG(LogTemp, Warning, TEXT("Offenders: %d"), offenders);

	AverageFitness = mTotalFitness / mPaths.Num();
	
	mGenerationInfo.mAverageFitness = AverageFitness;
	mGenerationInfo.mAverageAmountOfNodes = amount_of_nodes / (float)mPaths.Num();
	
	const float max_fitness = AmountOfNodesWeight + ProximityToTargetedNodeWeight + LengthWeight + CanSeeTargetWeight + TargetReachedWeight + SlopeWeight;
	mGenerationInfo.mMaximumFitness = max_fitness;
	mGenerationInfo.mFitnessFactor = AverageFitness / max_fitness;
		
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
	TArray<APath*> temp;
	temp.Reserve(PopulationCount);

	int32 successfull_crossover_amount = 0;

	// Loop over the paths and try to apply crossover
	for (int32 i = 0; i < mMatingPaths.Num(); i += 2)
	{
		const float R = FMath::FRandRange(0.0f, 100.0f);

		// Crossover for a pair happens if crossover probability is met
		if (R >= (100.0f - CrossoverProbability))
		{
			const APath* current_path = mMatingPaths[i];
			const APath* next_path = mMatingPaths[i + 1];
			const APath* smallest_path = nullptr;
			const APath* bigger_path = nullptr;
			
			if (current_path->GetAmountOfNodes() < next_path->GetAmountOfNodes())
			{
				smallest_path = current_path;
				bigger_path = next_path;
			}
			else
			{
				smallest_path = next_path;
				bigger_path = current_path;
			}

			const int32 num_chromosomes_small = smallest_path->GetAmountOfNodes();
			const int32 num_chromosomes_big = bigger_path->GetAmountOfNodes();

			const bool variadic_length = current_path->GetAmountOfNodes() != next_path->GetAmountOfNodes();

			APath* offspring_0 = GetWorld()->SpawnActor<APath>(GetTransform().GetLocation(), GetTransform().GetRotation().Rotator());;
			APath* offspring_1 = GetWorld()->SpawnActor<APath>(GetTransform().GetLocation(), GetTransform().GetRotation().Rotator());;

			// Do crossover operation depending on selected operator
			if (CrossoverOperator == ECrossoverOperator::SinglePoint)
			{
				const int32 crossover_index = FMath::RandRange(1, smallest_path->GetAmountOfNodes() - 1);
				for (int32 j = 0; j < num_chromosomes_big; ++j)
				{
					if (j < num_chromosomes_small)
					{
						// Do regular crossover when indices are valid
						if (j < crossover_index)
						{
							offspring_0->AddChromosome(smallest_path->GetChromosome(j));
							offspring_1->AddChromosome(bigger_path->GetChromosome(j));
						}
						else
						{
							offspring_0->AddChromosome(bigger_path->GetChromosome(j));
							offspring_1->AddChromosome(smallest_path->GetChromosome(j));
						}
					}
					else
					{
						// Append the rest of the chromosomes to the children when
						// The bigger path is more fit than the smaller one
						// Only interesting if we remove part of the fitness calculation
						if ((smallest_path->GetFitness() - smallest_path->GetAmountOfNodesFitness()) < (bigger_path->GetFitness() - bigger_path->GetAmountOfNodesFitness()))
						{
							offspring_0->AddChromosome(bigger_path->GetChromosome(j));
							offspring_1->AddChromosome(bigger_path->GetChromosome(j));
						}
					}
				}
			}
			else if (CrossoverOperator == ECrossoverOperator::DoublePoint)
			{
				const int32 first_crossover_index = FMath::FRandRange(1, smallest_path->GetAmountOfNodes() - 1);
				const int32 second_crossover_index = FMath::FRandRange(first_crossover_index + 1, smallest_path->GetAmountOfNodes() - 1);

				for (int32 j = 0; j < num_chromosomes_big; ++j)
				{
					if (j < num_chromosomes_small)
					{
						// Do double point crossover when indices are valid
						if (j < first_crossover_index)
						{
							offspring_0->AddChromosome(smallest_path->GetChromosome(j));
							offspring_1->AddChromosome(bigger_path->GetChromosome(j));
						}
						else if (j >= first_crossover_index && j < second_crossover_index)
						{
							offspring_0->AddChromosome(bigger_path->GetChromosome(j));
							offspring_1->AddChromosome(smallest_path->GetChromosome(j));
						}
						else if (j >= second_crossover_index)
						{
							offspring_0->AddChromosome(smallest_path->GetChromosome(j));
							offspring_1->AddChromosome(bigger_path->GetChromosome(j));
						}
					}
					else
					{
						if ((smallest_path->GetFitness() - smallest_path->GetAmountOfNodesFitness()) < (bigger_path->GetFitness() - bigger_path->GetAmountOfNodesFitness()))
						{
							offspring_0->AddChromosome(bigger_path->GetChromosome(j));
							offspring_1->AddChromosome(bigger_path->GetChromosome(j));
						}
					}
				}
			}
			else if (CrossoverOperator == ECrossoverOperator::Uniform)
			{
				for (int32 j = 0; j < num_chromosomes_big; ++j)
				{
					if (j < num_chromosomes_small)
					{
						const float bias = FMath::FRandRange(0.0f, 100.0f);
						if (bias < 50.0f)
						{
							offspring_0->AddChromosome(smallest_path->GetChromosome(j));
							offspring_1->AddChromosome(bigger_path->GetChromosome(j));
						}
						else
						{
							offspring_0->AddChromosome(bigger_path->GetChromosome(j));
							offspring_1->AddChromosome(smallest_path->GetChromosome(j));
						}
					}
					else 
					{
						if ((smallest_path->GetFitness() - smallest_path->GetAmountOfNodesFitness()) < (bigger_path->GetFitness() - bigger_path->GetAmountOfNodesFitness()))
						{
							offspring_0->AddChromosome(bigger_path->GetChromosome(j));
							offspring_1->AddChromosome(bigger_path->GetChromosome(j));
						}
					}
				}
			}

			offspring_0->DetermineGeneticRepresentation();
			offspring_1->DetermineGeneticRepresentation();

			temp.Add(offspring_0);
			temp.Add(offspring_1);

			++successfull_crossover_amount;
		}
		else // otherwise they are carried / copied over to the next generation
		{
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

	Purge(); // Get rid of the old paths

	mPaths = temp; // Keep track of the new paths

	mGenerationInfo.mCrossoverAmount = successfull_crossover_amount;
}


void APathManager::MutationStep()
{
	// Keep track of the mutation amount this generation
	int32 successful_translation_mutations = 0;
	int32 successful_insertion_mutations = 0;
	int32 successful_deletion_mutations = 0;

	for (APath* path : mPaths)
	{
		// Every path may be considered for mutation
		const float rand = FMath::FRandRange(0.0f, 100.0f);
		if (rand < MutationProbability)
		{
			// Determine which mutations occur
			bool do_translation_mutation = false;
			bool do_insertion_mutation = false;
			bool do_deletion_mutation = false;

			const float translate_point_probability = FMath::FRandRange(0, 100.0f);
			if (translate_point_probability < TranslatePointProbability)
				do_translation_mutation = true;

			const float insert_point_probability = FMath::FRandRange(0, 100.0f);
			if (insert_point_probability < InsertionProbability)
				do_insertion_mutation = true;

			// Only do insertion or deletion in the same mutation step
			if (!do_insertion_mutation)
			{
				const float deletion_probability = FMath::FRandRange(0, 100.0f);
				if (deletion_probability < DeletionProbability)
					do_deletion_mutation = true;
			}

			// Then do mutations
			if (do_translation_mutation)
			{
				path->MutateThroughTranslation(TranslationMutationType, MaxTranslationOffset);
				++successful_translation_mutations;
			}	
			if (do_insertion_mutation)
			{
				path->MutateThroughInsertion();
				++successful_insertion_mutations;
			}
			if (do_deletion_mutation)
			{
				path->MutateThroughDeletion();
				++successful_deletion_mutations;
			}
		}
	}

	// Keep track of the mutation amount
	mGenerationInfo.mAmountOfTranslationMutations = successful_translation_mutations;
	mGenerationInfo.mAmountOfInsertionMutations = successful_insertion_mutations;
	mGenerationInfo.mAmountOfDeletionMutations = successful_deletion_mutations;
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

	// Cache lowest & highest fitness first
	// @TODO: Do this in the loop of EvaluateFitness()
	for (const APath* path : mPaths)
	{
		const float fitness = path->GetFitness();

		if (fitness < lowest_fitness)
			lowest_fitness = fitness;
		if (fitness > highest_fitness)
			highest_fitness = fitness;
	}

	for (APath* path : mPaths)
	{
		check(path != nullptr);

		if (path->GetIsInObstacle() || path->GetSlopeTooIntense() || path->GetTravelingThroughTerrain() || path->GetDistanceBetweenChromosomesTooLarge())
		{
			// Completely unfit paths are marked grey
			path->SetColorCode(InvalidPathColor);
		}
		else
		{
			// Apply color coding based on the lowest and highest fitness values
			const float fitness = path->GetFitness();
			const float blend_value = (fitness - highest_fitness) / (lowest_fitness - highest_fitness);

			FColor red = FColor::Red;
			FColor green = FColor::Green;

			FColor blended;
			blended.A = 255;
			blended.R = FMath::Lerp(red.R, green.R, blend_value * 255);
			blended.G = FMath::Lerp(red.G, green.G, blend_value * 255);
			blended.B = FMath::Lerp(red.B, green.B, blend_value * 255);

			path->SetColorCode(blended);
		}
	}
}


void APathManager::LogGenerationInfo()
{
	if (GEngine != nullptr)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Black, TEXT("\n\n"));

		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("Average amount of nodes: ") + FString::SanitizeFloat(mGenerationInfo.mAverageAmountOfNodes));
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::White, TEXT("Fitness factor: ") + FString::SanitizeFloat(mGenerationInfo.mFitnessFactor));
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, TEXT("Maximum fitness: ") + FString::SanitizeFloat(mGenerationInfo.mMaximumFitness));
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::White, TEXT("Average fitness: ") + FString::SanitizeFloat(mGenerationInfo.mAverageFitness));
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Orange, TEXT("Amount of deletion mutations: ") + FString::FromInt(mGenerationInfo.mAmountOfDeletionMutations));
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("Amount of insertion mutations: ") + FString::FromInt(mGenerationInfo.mAmountOfInsertionMutations));
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Orange, TEXT("Amount of translation mutations: ") + FString::FromInt(mGenerationInfo.mAmountOfTranslationMutations));
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Amount of reproducing crossovers: ") + FString::FromInt(mGenerationInfo.mCrossoverAmount));
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("Generation #") + FString::FromInt(mGenerationInfo.mGenerationNumber));
	}
}



void APathManager::ChangeAnimationControlState(const EAnimationControlState inAnimationControlState)
{
	mNextAnimationControlState = inAnimationControlState;

	if (mNextAnimationControlState != mPreviousAnimationControlState)
	{
		if ((mPreviousAnimationControlState == EAnimationControlState::Limbo && mNextAnimationControlState == EAnimationControlState::Play) ||
			(mPreviousAnimationControlState == EAnimationControlState::Play && mNextAnimationControlState == EAnimationControlState::Pause) ||
			(mPreviousAnimationControlState == EAnimationControlState::Play && mNextAnimationControlState == EAnimationControlState::Stop) ||
			(mPreviousAnimationControlState == EAnimationControlState::Pause && mNextAnimationControlState == EAnimationControlState::Play) ||
			(mPreviousAnimationControlState == EAnimationControlState::Pause && mNextAnimationControlState == EAnimationControlState::Stop))
		{
			mPreviousAnimationControlState = mNextAnimationControlState;
		}
	}

	/**
	* Limbo  >  Play
	*   ^   v/  v ^   
	* Stop  <  Pause
	*/
}



void APathManager::StopRun()
{
	// @TODO: Serialize data

	
	// Reset data
	GenerationCount = 0;
	
	// Get rid of paths
	// Keep memory allocated
	for (APath* path : mPaths)
	{
		if (path != nullptr && path->IsValidLowLevel())
			path->Dispose();
	}
	mPaths.Empty(mPaths.Num());

	for (APath* path : mMatingPaths)
	{
		if (path != nullptr && path->IsValidLowLevel())
			path->Dispose();
	}
	mMatingPaths.Empty(mMatingPaths.Num());
	
	// Stop generation cycle
	mPreviousAnimationControlState = EAnimationControlState::Limbo;
	mNextAnimationControlState = EAnimationControlState::Limbo;
}
