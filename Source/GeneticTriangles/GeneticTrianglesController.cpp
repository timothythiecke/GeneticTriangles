// Fill out your copyright notice in the Description page of Project Settings.

#include "GeneticTriangles.h"
#include "GeneticTrianglesController.h"

#include "TriangleManager.h"
#include "UpdatedTriangleManager.h"
#include "PathManager.h"

#include "EngineUtils.h"

AGeneticTrianglesController::AGeneticTrianglesController()
{

}

void AGeneticTrianglesController::BeginPlay()
{
	// So basically
	// -> Bind to the input event GASpaceBar which will be fired by the engine
	// -> If the GASpaceBar event is merely a press
	// -> Call function GASpaceBar
	// -> Using this as binding object

	// Setup input bindings
	if (InputComponent != nullptr)
	{
		InputComponent->BindAction("GASpaceBar", EInputEvent::IE_Pressed, this, &AGeneticTrianglesController::GASpaceBar);
		InputComponent->BindAction("GAFitness", EInputEvent::IE_Pressed, this, &AGeneticTrianglesController::GAFitness);
		// ...
	}
}

void AGeneticTrianglesController::GASpaceBar()
{
	FindTriangleManager();

	// If there is a valid TriangleManager, start the algorithm
	if (mTriangleManager != nullptr)
		mTriangleManager->InitializePopulation();

	if (mUpdatedTriangleManager == nullptr)
	{
		for (TActorIterator<AActor> actor_iterator(GetWorld()); actor_iterator; ++actor_iterator)
		{
			AUpdatedTriangleManager* ptr = Cast<AUpdatedTriangleManager>(*actor_iterator);

			if (ptr != nullptr)
			{
				mUpdatedTriangleManager = ptr;
				break;
			}
		}
	}

	if (mUpdatedTriangleManager != nullptr)
	{
		if (mUpdatedTriangleManager->HasGeneratedTriangles())
			mUpdatedTriangleManager->RunGeneration();
		else
			mUpdatedTriangleManager->Initialize();
	}
}



void AGeneticTrianglesController::GAFitness()
{
	FindTriangleManager();

	if (mTriangleManager != nullptr)
		mTriangleManager->EvaluateFitness();
}



void AGeneticTrianglesController::FindTriangleManager()
{
	// Fetch the TriangleManager object in the world, if we haven't found one already
	// The look up becomes larger each time an actor is added to the world
	// We require nullptr handling regardless

	if (mTriangleManager != nullptr)
		return;

	for (TActorIterator<AActor> actor_iterator(GetWorld()); actor_iterator; ++actor_iterator)
	{
		ATriangleManager* ptr = Cast<ATriangleManager>(*actor_iterator);

		if (ptr != nullptr)
		{
			mTriangleManager = ptr;
			break;
		}
	}
}



void AGeneticTrianglesController::GeneratePopulation()
{
	FindTriangleManager();

	if (mTriangleManager != nullptr && !mTriangleManager->HasTriangles())
		mTriangleManager->InitializePopulation();
}



void AGeneticTrianglesController::EvaluateFitnessOfPopulation()
{
	FindTriangleManager();

	if (mTriangleManager != nullptr)
		mTriangleManager->EvaluateFitness();
}



void AGeneticTrianglesController::SelectPairsForReproduction()
{
	FindTriangleManager();

	if (mTriangleManager != nullptr)
		mTriangleManager->SelectionStep();
}



void AGeneticTrianglesController::CrossoverStep()
{
	FindTriangleManager();

	if (mTriangleManager != nullptr)
		mTriangleManager->CrossoverStep();
}



void AGeneticTrianglesController::Mutation()
{
	FindTriangleManager();

	if (mTriangleManager != nullptr)
	{
		mTriangleManager->MutationStep();
		++mTriangleManager->GenerationCount;
	}
}



void AGeneticTrianglesController::DoEverything()
{
	FindTriangleManager();
	
	if (mTriangleManager != nullptr)
	{
		if (!mTriangleManager->HasTriangles())
			mTriangleManager->InitializePopulation();
		
		mTriangleManager->EvaluateFitness();
		mTriangleManager->SelectionStep();
		mTriangleManager->CrossoverStep();
		mTriangleManager->MutationStep();

		++mTriangleManager->GenerationCount;
	}
}



void AGeneticTrianglesController::SetMutationRateBalancing(const bool UsesMutationRateBalancing)
{
	FindTriangleManager();

	if (mTriangleManager != nullptr)
		mTriangleManager->SetBalanceMutationRate(UsesMutationRateBalancing);
}



void AGeneticTrianglesController::RequestAnimationControlStateUpdate(const EAnimationControlState inAnimationControlState)
{
	FindPathManager();

	check(mPathManager != nullptr);

	mPathManager->ChangeAnimationControlState(inAnimationControlState);
}



void AGeneticTrianglesController::RequestDeserialization()
{
	FindPathManager();

	check(mPathManager != nullptr);

	mPathManager->DeserializeData();
}



int32 AGeneticTrianglesController::RequestKnowledgeOfGenerationCount()
{
	FindPathManager();

	check(mPathManager != nullptr);

	return mPathManager->GenerationCount;
}



void AGeneticTrianglesController::FindPathManager()
{
	if (mPathManager == nullptr)
	{
		for (TActorIterator<AActor> actor_iterator(GetWorld()); actor_iterator; ++actor_iterator)
		{
			APathManager* ptr = Cast<APathManager>(*actor_iterator);

			if (ptr != nullptr)
			{
				mPathManager = ptr;
				break;
			}
		}
	}
	else
	{
		check(mPathManager->IsValidLowLevel());
	}
}