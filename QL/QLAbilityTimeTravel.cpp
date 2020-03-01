//------------------------------------------------------------
// Quarter Life
//
// GNU General Public License v3.0
//
//  (\-/)
// (='.'=)
// (")-(")o
//------------------------------------------------------------


#include "QLAbilityTimeTravel.h"
#include "QLUtility.h"
#include "QLAbilityManager.h"
#include "QLCharacter.h"

//------------------------------------------------------------
//------------------------------------------------------------
AQLAbilityTimeTravel::AQLAbilityTimeTravel()
{
    QLName = FName(TEXT("TimeTravel"));

    SoundIdx = 0;
}

//------------------------------------------------------------
//------------------------------------------------------------
void AQLAbilityTimeTravel::BeginPlay()
{
    Super::BeginPlay();
}

//------------------------------------------------------------
//------------------------------------------------------------
void AQLAbilityTimeTravel::PostInitializeComponents()
{
    Super::PostInitializeComponents();
}

//------------------------------------------------------------
//------------------------------------------------------------
void AQLAbilityTimeTravel::OnUse()
{
    Super::OnUse();

    if (!NearActor.IsValid() ||
        !FarActor.IsValid() ||
        !AbilityManager.IsValid() ||
        !AbilityManager->GetUser())
    {
        return;
    }

    AQLCharacter* MyCharacter = AbilityManager->GetUser();

    // check which actor, near or far is actually close to the actor
    // and use that actor as the near actor
    // use distance squared directly to reduce calculation
    float d1 = FVector::DistSquared(MyCharacter->GetActorLocation(), NearActor->GetActorLocation());
    float d2 = FVector::DistSquared(MyCharacter->GetActorLocation(), FarActor->GetActorLocation());
    if (d1 > d2)
    {
        SwapNearAndFarActor();
    }


    FVector NewLocation = MyCharacter->GetActorLocation() - NearActor->GetActorLocation() + FarActor->GetActorLocation();
    bool bTeleportSuccess = MyCharacter->TeleportTo(NewLocation,
        FRotator::ZeroRotator,
        false, // not a test, but actual teleport
        false); // check if the actor can be teleported

    if (!bTeleportSuccess)
    {
        return;
    }

    // now that teleport succeeds
    // do not rotate character
    // instead rotate the controller
    FRotator NewRotation = MyCharacter->GetController()->GetControlRotation() - NearActor->GetActorRotation() + FarActor->GetActorRotation();
    MyCharacter->GetController()->SetControlRotation(NewRotation);

    SwapNearAndFarActor();

    constexpr char* MySoundNames[2] = { "EnergyDagger", "EnergySword" };

    PlaySoundFireAndForget(FName(MySoundNames[SoundIdx]));

    // change sound index for the next play
    if (SoundIdx == 0)
    {
        SoundIdx = 1;
    }
    else if (SoundIdx == 1)
    {
        SoundIdx = 0;
    }
}

//------------------------------------------------------------
//------------------------------------------------------------
void AQLAbilityTimeTravel::OnAbilityEnd()
{
    Super::OnAbilityEnd();
}

//------------------------------------------------------------
//------------------------------------------------------------
void AQLAbilityTimeTravel::SetNearAndFarActors(AActor* NearActorExt, AActor* FarActorExt)
{
    NearActor = NearActorExt;
    FarActor = FarActorExt;
}

//------------------------------------------------------------
//------------------------------------------------------------
void AQLAbilityTimeTravel::SwapNearAndFarActor()
{
    TWeakObjectPtr<AActor> Temp = NearActor;
    NearActor = FarActor;
    FarActor = Temp;
}

