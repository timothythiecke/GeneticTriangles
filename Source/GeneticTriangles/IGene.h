// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "IGene.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UIGene : public UInterface
{
	GENERATED_UINTERFACE_BODY()
};

/**
 * 
 */
class GENETICTRIANGLES_API IIGene
{
	GENERATED_IINTERFACE_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	void* GetPointerToGeneticValue() const { return mManagedPtr; }
	virtual void Dispose();

protected:
	void* mManagedPtr;
};
