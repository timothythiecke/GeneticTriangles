#pragma once

#include "Enums.generated.h"

UENUM(BlueprintType)
enum class ECrossoverOperator : uint8
{
	SinglePoint UMETA(DisplayName = "Single Point"),
	DoublePoint UMETA(DisplayName = "Double Point"),
	Uniform UMETA(DisplayName = "Uniform")
};

UENUM(BlueprintType, Meta = (Bitflags))
enum EMutationType
{
	TranslatePoint = 1	UMETA(DisplayName = "TranslatePointMutation"),
	Insertion = 2		UMETA(DisplayName = "InsertionMutation"),
	Deletion = 3		UMETA(DisplayName = "DeletionMutation")
};
ENUM_CLASS_FLAGS(EMutationType)

/**
* AnyButStart: A random chromosome is selected (except the starting one) and is mutated
* HeadOnly: The last chromosome in the genetic representation is mutated
* HeadFalloff: The last chromosome in the genetic representation is mutated, the same mutation is applied with a linear falloff for each subsequent chromosome
* AllAtOnce: All chromsomes (except the starting one) are mutated at the same time
*/
UENUM(BlueprintType)
enum class ETranslationMutationType : uint8
{
	AnyButStart UMETA(DisplayName = "AnyButStart"),
	HeadOnly UMETA(DisplayName = "HeadOnly"),
	HeadFalloff UMETA(DisplayName = "HeadFalloff"),
	AllAtOnce UMETA(DisplayName = "AllAtOnce")
};

UENUM(BlueprintType)
enum class EObstacleTraceBehaviour : uint8
{
	WindDirectionTracing UMETA(DisplayName = "WindDirectionTracing"),
	CircleTracing UMETA(DisplayName = "CircleTracing")
};

UENUM(BlueprintType)
enum class EAnimationControlState : uint8
{
	Play, // Start or continue running generations
	Pause, // Pause generation run
	Stop, // Stop generation run, serialize data, clear paths
	Limbo // Do nothing, wait for user to press play

		  /**
		  * Limbo -> Play
		  * Play -> Pause
		  *		-> Stop
		  * Pause -> Play
		  *		-> Stop
		  * Stop -> Limbo
		  */
};