// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

/**
 * 
 */
class GENETICTRIANGLES_API Gene
{
public:
	Gene() {};
	virtual ~Gene()
	{
		if (mDataPtr != nullptr)
		{
			delete mDataPtr;
			mDataPtr = nullptr;
		}
	};

	const void* GetGeneticValue() const { return mDataPtr; }

protected:
	void* mDataPtr;
};
