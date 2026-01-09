// Copyright (c) Yevhenii Selivanov

#include "Factories/BmrPoolFactory_Pawn.h"

// Bomber
#include "Actors/BmrPawn.h"
#include "Components/BmrMoverComponent.h"
#include "Data/TakeFromPoolPayload.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BmrPoolFactory_Pawn)

// Is overridden to handle Pawn-inherited classes
const UClass* UBmrPoolFactory_Pawn::GetObjectClass_Implementation() const
{
	return ABmrPawn::StaticClass();
}

// Is overridden to prevent SetActorTransform call that causes Mover simulation desyncs and location disagreements between simulation start location and actual component location
void UBmrPoolFactory_Pawn::OnTakeFromPool_Implementation(UObject* Object, const FTakeFromPoolPayload& Payload)
{
	// Calling grandparent Super, bypassing AActor's OnTakeFromPool to avoid SetActorTransform call that conflicts with Mover simulation
	UPoolFactory_UObject::OnTakeFromPool_Implementation(Object, Payload);

	const ABmrPawn* Pawn = CastChecked<ABmrPawn>(Object);
	UBmrMoverComponent* MoverComponent = Pawn->GetMoverComponent();
	checkf(MoverComponent, TEXT("ERROR: [%i] %hs:\n'MoverComponent' is null!"), __LINE__, __FUNCTION__);
	MoverComponent->TeleportToLocation(Payload.Transform.GetLocation());
}

// Is overridden to reset transform to the actor before returning the object to its pool
void UBmrPoolFactory_Pawn::OnReturnToPool_Implementation(UObject* Object)
{
	// Calling grandparent Super, bypassing AActor's OnReturnToPool to avoid SetActorTransform call that conflicts with Mover simulation
	UPoolFactory_UObject::OnReturnToPool_Implementation(Object);

	const ABmrPawn* Pawn = CastChecked<ABmrPawn>(Object);
	UBmrMoverComponent* MoverComponent = Pawn->GetMoverComponent();
	checkf(MoverComponent, TEXT("ERROR: [%i] %hs:\n'MoverComponent' is null!"), __LINE__, __FUNCTION__);
	MoverComponent->TeleportToLocation(MaxPos);
}