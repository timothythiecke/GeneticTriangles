// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "IGene.h"

/**
 * 
 */
class GENETICTRIANGLES_API FloatGene : public IIGene
{
public:
	FloatGene();
	virtual ~FloatGene();

	void SetGeneticValue(const float inNewGeneticValue);

private:
};
