// Fill out your copyright notice in the Description page of Project Settings.

#include "GeneticTriangles.h"
#include "IGene.h"


// This function does not need to be modified.
UIGene::UIGene(const class FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
}

// Add default functionality here for any IIGene functions that are not pure virtual.

void IIGene::Dispose()
{
	if (mManagedPtr != nullptr)
	{
		delete mManagedPtr;
		mManagedPtr = nullptr;
	}
}