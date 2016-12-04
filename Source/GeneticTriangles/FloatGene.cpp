// Fill out your copyright notice in the Description page of Project Settings.

#include "GeneticTriangles.h"
#include "FloatGene.h"

FloatGene::FloatGene()
{
	mManagedPtr = new float;
}

FloatGene::~FloatGene()
{
	Dispose();
}



void FloatGene::SetGeneticValue(const float inNewGeneticValue)
{
	if (mManagedPtr != nullptr)
		*((float*) mManagedPtr) = inNewGeneticValue;
}
