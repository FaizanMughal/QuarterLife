//------------------------------------------------------------
// Quarter Life
//
// GNU General Public License v3.0
//
//  (\-/)
// (='.'=)
// (")-(")o
//------------------------------------------------------------

#pragma once

#include "CoreMinimal.h"
#include "QLMovementStyle.generated.h"

//------------------------------------------------------------
//------------------------------------------------------------
UENUM(BlueprintType)
enum class EQLMovementStyle : uint8
{
    Default UMETA(DisplayName = "Default"),
    QuakeVanilla UMETA(DisplayName = "QuakeVanilla"), // vanilla quake 3, VQ3
    QuakeCPMA UMETA(DisplayName = "QuakeCPMA"), // Challenge ProMode Arena
    DoomGuy UMETA(DisplayName = "DoomGuy"),
    QuakeCrouchSlide UMETA(DisplayName = "QuakeCrouchSlide"), // Slash, Strogg & Peeker
};
