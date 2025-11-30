// Copyright (c) Yevhenii Selivanov.

#pragma once

#include "GameFramework/GameModeBase.h"

#include "BmrGameMode.generated.h"

/**
 * The custom game mode class
 */
UCLASS()
class BOMBER_API ABmrGameMode final : public AGameModeBase
{
	GENERATED_BODY()

public:
	/** Sets default values for this actor's properties */
	ABmrGameMode();

	/*********************************************************************************************
	 * Player Controllers
	 ********************************************************************************************* */
public:
	/** Get overall number of all player controllers. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	FORCEINLINE int32 GetPlayerControllersNum() const { return PlayerControllers.Num(); }

	/** Returns player controller by specified index.
	 * @see ABmrGameMode::PlayerControllers */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	class ABmrPlayerController* GetPlayerController(int32 Index) const;

	/** Returns index of the specified player controller. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "[Bomber]")
	FORCEINLINE int32 GetPlayerControllerIndex(const ABmrPlayerController* PlayerController) const { return PlayerControllers.IndexOfByKey(PlayerController); }

	/** Caches given player controller when it spawns. */
	UFUNCTION(BlueprintCallable, Category = "[Bomber]")
	void AddPlayerController(class ABmrPlayerController* PlayerController);

protected:
	/** Contains all player controllers joined this session.  */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Transient, AdvancedDisplay, Category = "[Bomber]", meta = (BlueprintProtected))
	TArray<TObjectPtr<class ABmrPlayerController>> PlayerControllers;

	/*********************************************************************************************
	 * Overrides
	 ********************************************************************************************* */
protected:
	/** Initializes the game. */
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

	/** Called after a successful login.  This is the first place it is safe to call replicated functions on the PlayerController. */
	virtual void PostLogin(APlayerController* NewPlayer) override;

	/** Called when a Controller with a PlayerState leaves the game or is destroyed. */
	virtual void Logout(AController* Exiting) override;

	/** Sets the name for a controller. */
	virtual void ChangeName(AController* Controller, const FString& NewName, bool bNameChange) override;

#if WITH_EDITOR
	/** Is called if start the game in 'Simulate in Editor' and then press 'Possess or eject player' button. */
	virtual bool SpawnPlayerFromSimulate(const FVector& NewLocation, const FRotator& NewRotation) override;
#endif // WITH_EDITOR
};