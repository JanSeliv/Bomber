// Copyright (c) Yevhenii Selivanov.

#pragma once

#include "GameFramework/GameModeBase.h"
//---
#include "MyGameModeBase.generated.h"

/**
 * The custom game mode class
 */
UCLASS()
class BOMBER_API AMyGameModeBase final : public AGameModeBase
{
	GENERATED_BODY()

public:
	/** Sets default values for this actor's properties */
	AMyGameModeBase();

	/** Get overall number of all player controllers. */
	UFUNCTION(BlueprintPure, Category = "C++")
	FORCEINLINE int32 GetPlayerControllersNum() const { return PlayerControllersInternal.Num(); }

	/** Returns player controller by specified index.
	 * @see AMyGameModeBase::PlayerControllersInternal */
	UFUNCTION(BlueprintPure, Category = "C++")
	class AMyPlayerController* GetPlayerController(int32 Index) const;

protected:
	/** Contains all player controllers.  */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, Category = "C++", meta = (BlueprintProtected, DisplayName = "Player Controllers"))
	TArray<TObjectPtr<class AMyPlayerController>> PlayerControllersInternal; //[G]

	/** Called when the game starts or when spawned */
	virtual void BeginPlay() override;

	/** Called after a successful login.  This is the first place it is safe to call replicated functions on the PlayerController. */
	virtual void PostLogin(APlayerController* NewPlayer) override;

	/** Called when a Controller with a PlayerState leaves the game or is destroyed. */
	virtual void Logout(AController* Exiting) override;

#if WITH_EDITOR
	/** Is called if start the game in 'Simulate in Editor' and then press 'Possess or eject player' button. */
	virtual bool SpawnPlayerFromSimulate(const FVector& NewLocation, const FRotator& NewRotation) override;
#endif // WITH_EDITOR
};
