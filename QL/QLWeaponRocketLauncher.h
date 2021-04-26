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
#include "QLWeapon.h"
#include "QLWeaponRocketLauncher.generated.h"

class AQLRocketProjectile;
//------------------------------------------------------------
//------------------------------------------------------------
UCLASS()
class QL_API AQLWeaponRocketLauncher : public AQLWeapon
{
	GENERATED_BODY()

public:
    AQLWeaponRocketLauncher();

    virtual void OnFire() override;

    virtual void OnFireRelease() override;

    virtual void SpawnProjectile() override;
protected:
    virtual void Tick(float DeltaTime) override;

    virtual void PostInitializeComponents() override;

    UPROPERTY(EditDefaultsOnly, Category = "C++Property")
    TSubclassOf<AQLRocketProjectile> RocketProjectileClass;
};
