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
#include "Components/PostProcessComponent.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "QLPortal.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "Camera/CameraComponent.h"

//------------------------------------------------------------
//------------------------------------------------------------
AQLAbilityTimeTravel::AQLAbilityTimeTravel()
{
    QLName = FName(TEXT("TimeTravel"));
    PostProcessComponent = CreateDefaultSubobject<UPostProcessComponent>(TEXT("PostProcessComponent"));
    PostProcessComponent->bEnabled = false;

    TimeTravelTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("TimeTravelTimeline"));
    TimeTravelTimelineInterpFunction.BindUFunction(this, FName(TEXT("TimeTravelCallback")));

    SoundIdx = 0;

    PortalClass = AQLPortal::StaticClass();
    NearPortal = nullptr;
    FarPortal = nullptr;
}

//------------------------------------------------------------
//------------------------------------------------------------
void AQLAbilityTimeTravel::BeginPlay()
{
    Super::BeginPlay();

    FTransform transform;

    NearPortal = GetWorld()->SpawnActorDeferred<AQLPortal>(PortalClass, transform);
    UGameplayStatics::FinishSpawningActor(NearPortal, transform);
    NearPortal->AttachToActor(this, FAttachmentTransformRules::KeepRelativeTransform);
    NearPortal->SetActorRelativeLocation(FVector(0.0f, -100.0f, 0.0f));
    NearPortal->SetActorRelativeRotation(FRotator(0.0f, 180.0f, 0.0f));

    FarPortal = GetWorld()->SpawnActorDeferred<AQLPortal>(PortalClass, transform);
    UGameplayStatics::FinishSpawningActor(FarPortal, transform);
    FarPortal->AttachToActor(this, FAttachmentTransformRules::KeepRelativeTransform);
    FarPortal->SetActorRelativeLocation(FVector(100.0f, 100.0f, 0.0f));
    FarPortal->SetActorRelativeRotation(FRotator(0.0f, 180.0f, 0.0f));

    if (NearPortal && FarPortal)
    {
        NearPortal->SetSpouse(FarPortal);
        FarPortal->SetSpouse(NearPortal);
    }
}

//------------------------------------------------------------
//------------------------------------------------------------
void AQLAbilityTimeTravel::PostInitializeComponents()
{
    Super::PostInitializeComponents();

    if (PostProcessComponent)
    {
        if (MaterialTimeTravel)
        {
            FWeightedBlendable WeightedBlendable;
            DynamicMaterialTimeTravel = UKismetMaterialLibrary::CreateDynamicMaterialInstance(this, MaterialTimeTravel);
            WeightedBlendable.Object = DynamicMaterialTimeTravel.Get();
            WeightedBlendable.Weight = 1.0f;
            PostProcessComponent->Settings.WeightedBlendables.Array.Add(WeightedBlendable);
        }
    }

    if (TimeTravelTimeline && TimeTravelCurve)
    {
        TimeTravelTimeline->AddInterpFloat(TimeTravelCurve, TimeTravelTimelineInterpFunction, FName(TEXT("TimeTravel")));
    }
}

//------------------------------------------------------------
//------------------------------------------------------------
void AQLAbilityTimeTravel::Tick(float DeltaTime)
{

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

    if (!MyCharacter)
    {
        return;
    }

    // check which actor, near or far is actually close to the actor
    // and use that actor as the near actor
    // use distance squared directly to reduce calculation
    float d1 = FVector::DistSquared(MyCharacter->GetActorLocation(), NearActor->GetActorLocation());
    float d2 = FVector::DistSquared(MyCharacter->GetActorLocation(), FarActor->GetActorLocation());
    if (d1 > d2)
    {
        SwapNearAndFarActor();
    }


    FTransform transform = CalculateShadowCharacterTransform();

    bool bTeleportSuccess = MyCharacter->TeleportTo(transform.GetLocation(),
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
    MyCharacter->GetController()->SetControlRotation(transform.GetRotation().Rotator());

    SwapNearAndFarActor();

    // enable postprocessing effect
    PostProcessComponent->bEnabled = true;

    // play sound
    constexpr char* MySoundNames[2] = { "OutsiderAppear", "OutsiderDisappear" };
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

    // postprocessing animation
    if (TimeTravelTimeline && TimeTravelCurve)
    {
        TimeTravelTimeline->PlayFromStart();
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
FTransform AQLAbilityTimeTravel::CalculateShadowCharacterTransform()
{
    FTransform transform;

    if (!NearActor.IsValid() ||
        !FarActor.IsValid() ||
        !AbilityManager.IsValid() ||
        !AbilityManager->GetUser())
    {
        return transform;
    }

    AQLCharacter* MyCharacter = AbilityManager->GetUser();

    if (!MyCharacter)
    {
        return transform;
    }

    FVector NewLocation = MyCharacter->GetActorLocation() - NearActor->GetActorLocation() + FarActor->GetActorLocation();

    // do not rotate character
    // instead rotate the controller
    FRotator NewRotation = MyCharacter->GetController()->GetControlRotation() - NearActor->GetActorRotation() + FarActor->GetActorRotation();

    transform.SetLocation(NewLocation);
    transform.SetRotation(NewRotation.Quaternion());

    return transform;
}

//------------------------------------------------------------
//------------------------------------------------------------
FTransform AQLAbilityTimeTravel::CalculateNearAndFarPortalTransform()
{
    if (AbilityManager.IsValid())
    {
        AQLCharacter* MyCharacter = AbilityManager->GetUser();
        if (MyCharacter)
        {

        }
    }

    FTransform transform;

    return transform;
}

//------------------------------------------------------------
//------------------------------------------------------------
void AQLAbilityTimeTravel::SwapNearAndFarActor()
{
    TWeakObjectPtr<AActor> Temp = NearActor;
    NearActor = FarActor;
    FarActor = Temp;
}

//------------------------------------------------------------
//------------------------------------------------------------
void AQLAbilityTimeTravel::TimeTravelCallback(float Val)
{
    if (DynamicMaterialTimeTravel.IsValid())
    {
        DynamicMaterialTimeTravel->SetScalarParameterValue("InterpParam", Val);
    }
}

//------------------------------------------------------------
//------------------------------------------------------------
void AQLAbilityTimeTravel::OnAbilitySetCurrent()
{
    Super::OnAbilitySetCurrent();

    if (AbilityManager.IsValid())
    {
        AQLCharacter* MyCharacter = AbilityManager->GetUser();
        if (MyCharacter)
        {
            this->AttachToComponent(MyCharacter->GetFirstPersonMesh(), FAttachmentTransformRules::KeepRelativeTransform);
            this->SetActorRelativeLocation(FVector(300.0f, 0.0f, 100.0f));

            UStaticMeshComponent* abilityMesh = GetStaticMeshComponent();
            if (abilityMesh)
            {
                abilityMesh->SetVisibility(true);
            }
        }
    }
}

//------------------------------------------------------------
//------------------------------------------------------------
void AQLAbilityTimeTravel::Debug()
{
    //QLUtility::Log("FAR PORTAL SCC");
    //FarPortal->Debug();

    QLUtility::Log("NEAR PORTAL SCC");
    NearPortal->Debug();

    QLUtility::Log("PLAYER");
    if (AbilityManager.IsValid())
    {
        AQLCharacter* MyCharacter = AbilityManager->GetUser();
        UCameraComponent* MyCamera = MyCharacter->GetFirstPersonCameraComponent();
        QLUtility::Log(MyCamera->GetComponentLocation().ToString());
        QLUtility::Log(MyCamera->GetComponentRotation().ToString());
    }
}