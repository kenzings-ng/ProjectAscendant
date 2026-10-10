// Copyright Project Ascendant. All Rights Reserved.

#include "Network/PAServiceRequestComponent.h"
#include "Crafting/PABlacksmithComponent.h"
#include "Economy/PACurrencyComponent.h"
#include "Economy/PAMerchantComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "Inventory/PAInventoryComponent.h"
#include "Network/PAServerRequestValidation.h"

UPAServiceRequestComponent::UPAServiceRequestComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	// Component RPCs require a replicated component; it lives on the PlayerController (owning connection only).
	SetIsReplicatedByDefault(true);
}

APlayerController* UPAServiceRequestComponent::GetRequestingPlayerController() const
{
	return PAServerRequestValidation::ResolveOwningPlayerController(GetOwner());
}

UPAMerchantComponent* UPAServiceRequestComponent::ResolveMerchant(const AActor* TargetActor)
{
	return IsValid(TargetActor) ? TargetActor->FindComponentByClass<UPAMerchantComponent>() : nullptr;
}

UPABlacksmithComponent* UPAServiceRequestComponent::ResolveForge(const AActor* TargetActor)
{
	return IsValid(TargetActor) ? TargetActor->FindComponentByClass<UPABlacksmithComponent>() : nullptr;
}

void UPAServiceRequestComponent::BindSavedItemInventory(TArray<FPASavedItemInstance>* InItems, UPACurrencyComponent* InWallet)
{
	BoundSavedItems = InItems;
	BoundWallet = InWallet;
}

UPAMerchantComponent* UPAServiceRequestComponent::ResolveMerchantOrReject(int32 RequestId, const AActor* MerchantActor)
{
	UPAMerchantComponent* Merchant = ResolveMerchant(MerchantActor);
	if (!Merchant || !GetOwner() || !GetOwner()->HasAuthority())
	{
		Client_ConfirmMerchantRequest(RequestId, false, EPATransactionError::ServerRejected);
		return nullptr;
	}
	return Merchant;
}

UPABlacksmithComponent* UPAServiceRequestComponent::ResolveForgeOrReject(int32 RequestId, const AActor* ForgeActor)
{
	UPABlacksmithComponent* Forge = ResolveForge(ForgeActor);
	if (!Forge || !GetOwner() || !GetOwner()->HasAuthority())
	{
		Client_ConfirmForgeRequest(RequestId, false, EPACraftingError::ServerRejected);
		return nullptr;
	}
	return Forge;
}

// -----------------------------------------------------------------------------
// Merchant
// -----------------------------------------------------------------------------

bool UPAServiceRequestComponent::Server_RequestMerchantBuy_Validate(int32 RequestId, AActor* MerchantActor, UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet, int32 CatalogIndex, int32 Quantity)
{
	return CatalogIndex >= 0 && Quantity > 0;
}

void UPAServiceRequestComponent::Server_RequestMerchantBuy_Implementation(int32 RequestId, AActor* MerchantActor, UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet, int32 CatalogIndex, int32 Quantity)
{
	if (UPAMerchantComponent* Merchant = ResolveMerchantOrReject(RequestId, MerchantActor))
	{
		EPATransactionError Err = EPATransactionError::None;
		const bool bOk = Merchant->ServerHandleBuyItem(GetRequestingPlayerController(), Inventory, Wallet, CatalogIndex, Quantity, Err);
		Client_ConfirmMerchantRequest(RequestId, bOk, Err);
	}
}

bool UPAServiceRequestComponent::Server_RequestMerchantSell_Validate(int32 RequestId, AActor* MerchantActor, UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet, int32 SlotIndex, int32 Quantity)
{
	return SlotIndex >= 0 && Quantity > 0;
}

void UPAServiceRequestComponent::Server_RequestMerchantSell_Implementation(int32 RequestId, AActor* MerchantActor, UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet, int32 SlotIndex, int32 Quantity)
{
	if (UPAMerchantComponent* Merchant = ResolveMerchantOrReject(RequestId, MerchantActor))
	{
		EPATransactionError Err = EPATransactionError::None;
		const bool bOk = Merchant->ServerHandleSellItem(GetRequestingPlayerController(), Inventory, Wallet, SlotIndex, Quantity, Err);
		Client_ConfirmMerchantRequest(RequestId, bOk, Err);
	}
}

bool UPAServiceRequestComponent::Server_RequestMerchantSellAllJunk_Validate(int32 RequestId, AActor* MerchantActor, UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet)
{
	return true;
}

void UPAServiceRequestComponent::Server_RequestMerchantSellAllJunk_Implementation(int32 RequestId, AActor* MerchantActor, UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet)
{
	if (UPAMerchantComponent* Merchant = ResolveMerchantOrReject(RequestId, MerchantActor))
	{
		EPATransactionError Err = EPATransactionError::None;
		const bool bOk = Merchant->ServerHandleSellAllJunk(GetRequestingPlayerController(), Inventory, Wallet, Err);
		Client_ConfirmMerchantRequest(RequestId, bOk, Err);
	}
}

bool UPAServiceRequestComponent::Server_RequestMerchantBuyback_Validate(int32 RequestId, AActor* MerchantActor, UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet, const FGuid& ItemInstanceUID)
{
	// Never kick the client for an unknown / empty UID: the handler replies BuybackEmpty instead.
	return true;
}

void UPAServiceRequestComponent::Server_RequestMerchantBuyback_Implementation(int32 RequestId, AActor* MerchantActor, UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet, const FGuid& ItemInstanceUID)
{
	if (UPAMerchantComponent* Merchant = ResolveMerchantOrReject(RequestId, MerchantActor))
	{
		EPATransactionError Err = EPATransactionError::None;
		const bool bOk = Merchant->ServerHandleBuybackItem(GetRequestingPlayerController(), Inventory, Wallet, ItemInstanceUID, Err);
		Client_ConfirmMerchantRequest(RequestId, bOk, Err);
	}
}

// -----------------------------------------------------------------------------
// Blacksmith
// -----------------------------------------------------------------------------

bool UPAServiceRequestComponent::Server_RequestForgeRepair_Validate(int32 RequestId, AActor* ForgeActor, UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet, int32 SlotIndex)
{
	return SlotIndex >= 0;
}

void UPAServiceRequestComponent::Server_RequestForgeRepair_Implementation(int32 RequestId, AActor* ForgeActor, UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet, int32 SlotIndex)
{
	if (UPABlacksmithComponent* Forge = ResolveForgeOrReject(RequestId, ForgeActor))
	{
		EPACraftingError Err = EPACraftingError::None;
		const bool bOk = Forge->ServerHandleRepair(GetRequestingPlayerController(), Inventory, Wallet, SlotIndex, Err);
		Client_ConfirmForgeRequest(RequestId, bOk, Err);
	}
}

bool UPAServiceRequestComponent::Server_RequestForgeSalvage_Validate(int32 RequestId, AActor* ForgeActor, UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet, int32 SlotIndex)
{
	return SlotIndex >= 0;
}

void UPAServiceRequestComponent::Server_RequestForgeSalvage_Implementation(int32 RequestId, AActor* ForgeActor, UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet, int32 SlotIndex)
{
	if (UPABlacksmithComponent* Forge = ResolveForgeOrReject(RequestId, ForgeActor))
	{
		EPACraftingError Err = EPACraftingError::None;
		const bool bOk = Forge->ServerHandleSalvage(GetRequestingPlayerController(), Inventory, Wallet, SlotIndex, Err);
		Client_ConfirmForgeRequest(RequestId, bOk, Err);
	}
}

bool UPAServiceRequestComponent::Server_RequestForgeEnhance_Validate(int32 RequestId, AActor* ForgeActor, UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet, int32 SlotIndex, bool bUseWard)
{
	return SlotIndex >= 0;
}

void UPAServiceRequestComponent::Server_RequestForgeEnhance_Implementation(int32 RequestId, AActor* ForgeActor, UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet, int32 SlotIndex, bool bUseWard)
{
	if (UPABlacksmithComponent* Forge = ResolveForgeOrReject(RequestId, ForgeActor))
	{
		EPACraftingError Err = EPACraftingError::None;
		const bool bOk = Forge->ServerHandleEnhance(GetRequestingPlayerController(), Inventory, Wallet, SlotIndex, bUseWard, Err);
		Client_ConfirmForgeRequest(RequestId, bOk, Err);
	}
}

bool UPAServiceRequestComponent::Server_RequestForgeUnlockSocket_Validate(int32 RequestId, AActor* ForgeActor, UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet, int32 SlotIndex)
{
	return SlotIndex >= 0;
}

void UPAServiceRequestComponent::Server_RequestForgeUnlockSocket_Implementation(int32 RequestId, AActor* ForgeActor, UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet, int32 SlotIndex)
{
	if (UPABlacksmithComponent* Forge = ResolveForgeOrReject(RequestId, ForgeActor))
	{
		EPACraftingError Err = EPACraftingError::None;
		const bool bOk = Forge->ServerHandleUnlockSocket(GetRequestingPlayerController(), Inventory, Wallet, SlotIndex, Err);
		Client_ConfirmForgeRequest(RequestId, bOk, Err);
	}
}

bool UPAServiceRequestComponent::Server_RequestForgeSocketGem_Validate(int32 RequestId, AActor* ForgeActor, UPAInventoryComponent* Inventory, int32 EquipmentSlotIndex, int32 SocketIndex, FName GemItemId)
{
	return EquipmentSlotIndex >= 0 && SocketIndex >= 0;
}

void UPAServiceRequestComponent::Server_RequestForgeSocketGem_Implementation(int32 RequestId, AActor* ForgeActor, UPAInventoryComponent* Inventory, int32 EquipmentSlotIndex, int32 SocketIndex, FName GemItemId)
{
	if (UPABlacksmithComponent* Forge = ResolveForgeOrReject(RequestId, ForgeActor))
	{
		EPACraftingError Err = EPACraftingError::None;
		const bool bOk = Forge->ServerHandleSocketGem(GetRequestingPlayerController(), Inventory, EquipmentSlotIndex, SocketIndex, GemItemId, Err);
		Client_ConfirmForgeRequest(RequestId, bOk, Err);
	}
}

bool UPAServiceRequestComponent::Server_RequestForgeUnsocketGem_Validate(int32 RequestId, AActor* ForgeActor, UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet, int32 EquipmentSlotIndex, int32 SocketIndex)
{
	return EquipmentSlotIndex >= 0 && SocketIndex >= 0;
}

void UPAServiceRequestComponent::Server_RequestForgeUnsocketGem_Implementation(int32 RequestId, AActor* ForgeActor, UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet, int32 EquipmentSlotIndex, int32 SocketIndex)
{
	if (UPABlacksmithComponent* Forge = ResolveForgeOrReject(RequestId, ForgeActor))
	{
		EPACraftingError Err = EPACraftingError::None;
		const bool bOk = Forge->ServerHandleUnsocketGem(GetRequestingPlayerController(), Inventory, Wallet, EquipmentSlotIndex, SocketIndex, Err);
		Client_ConfirmForgeRequest(RequestId, bOk, Err);
	}
}

bool UPAServiceRequestComponent::Server_RequestForgeBossSoul_Validate(int32 RequestId, AActor* ForgeActor, UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet, FName BossSoulItemId, FName BossPartItemId, FName VoidOreItemId)
{
	return true;
}

void UPAServiceRequestComponent::Server_RequestForgeBossSoul_Implementation(int32 RequestId, AActor* ForgeActor, UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet, FName BossSoulItemId, FName BossPartItemId, FName VoidOreItemId)
{
	if (UPABlacksmithComponent* Forge = ResolveForgeOrReject(RequestId, ForgeActor))
	{
		EPACraftingError Err = EPACraftingError::None;
		const bool bOk = Forge->ServerHandleForgeBossSoul(GetRequestingPlayerController(), Inventory, Wallet, BossSoulItemId, BossPartItemId, VoidOreItemId, Err);
		Client_ConfirmForgeRequest(RequestId, bOk, Err);
	}
}

bool UPAServiceRequestComponent::Server_RequestForgeExpandBackpack_Validate(int32 RequestId, AActor* ForgeActor, UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet)
{
	return true;
}

void UPAServiceRequestComponent::Server_RequestForgeExpandBackpack_Implementation(int32 RequestId, AActor* ForgeActor, UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet)
{
	if (UPABlacksmithComponent* Forge = ResolveForgeOrReject(RequestId, ForgeActor))
	{
		EPACraftingError Err = EPACraftingError::None;
		const bool bOk = Forge->ServerHandleExpandBackpack(GetRequestingPlayerController(), Inventory, Wallet, Err);
		Client_ConfirmForgeRequest(RequestId, bOk, Err);
	}
}

bool UPAServiceRequestComponent::Server_RequestForgeRepairByUID_Validate(int32 RequestId, AActor* ForgeActor, const FGuid& ItemInstanceUID)
{
	return ItemInstanceUID.IsValid();
}

void UPAServiceRequestComponent::Server_RequestForgeRepairByUID_Implementation(int32 RequestId, AActor* ForgeActor, const FGuid& ItemInstanceUID)
{
	if (UPABlacksmithComponent* Forge = ResolveForgeOrReject(RequestId, ForgeActor))
	{
		EPACraftingError Err = EPACraftingError::None;
		const bool bOk = Forge->ServerHandleRepairItemByUID(GetRequestingPlayerController(), BoundSavedItems, BoundWallet.Get(), ItemInstanceUID, Err);
		Client_ConfirmForgeRequest(RequestId, bOk, Err);
	}
}

bool UPAServiceRequestComponent::Server_RequestForgeReforgeAffixByUID_Validate(int32 RequestId, AActor* ForgeActor, const FGuid& ItemInstanceUID, int32 AffixIndex)
{
	return ItemInstanceUID.IsValid() && AffixIndex >= 0;
}

void UPAServiceRequestComponent::Server_RequestForgeReforgeAffixByUID_Implementation(int32 RequestId, AActor* ForgeActor, const FGuid& ItemInstanceUID, int32 AffixIndex)
{
	if (UPABlacksmithComponent* Forge = ResolveForgeOrReject(RequestId, ForgeActor))
	{
		EPACraftingError Err = EPACraftingError::None;
		const bool bOk = Forge->ServerHandleReforgeAffixByUID(GetRequestingPlayerController(), BoundSavedItems, BoundWallet.Get(), ItemInstanceUID, AffixIndex, Err);
		Client_ConfirmForgeRequest(RequestId, bOk, Err);
	}
}

bool UPAServiceRequestComponent::Server_RequestForgeAddSocketByUID_Validate(int32 RequestId, AActor* ForgeActor, const FGuid& ItemInstanceUID)
{
	return ItemInstanceUID.IsValid();
}

void UPAServiceRequestComponent::Server_RequestForgeAddSocketByUID_Implementation(int32 RequestId, AActor* ForgeActor, const FGuid& ItemInstanceUID)
{
	if (UPABlacksmithComponent* Forge = ResolveForgeOrReject(RequestId, ForgeActor))
	{
		EPACraftingError Err = EPACraftingError::None;
		const bool bOk = Forge->ServerHandleAddSocketByUID(GetRequestingPlayerController(), BoundSavedItems, BoundWallet.Get(), ItemInstanceUID, Err);
		Client_ConfirmForgeRequest(RequestId, bOk, Err);
	}
}

// -----------------------------------------------------------------------------
// Confirmations (run on the owning client; locally on listen-host / standalone)
// -----------------------------------------------------------------------------

void UPAServiceRequestComponent::Client_ConfirmMerchantRequest_Implementation(int32 RequestId, bool bSuccess, EPATransactionError ErrorCode)
{
	OnMerchantRequestConfirmedNative.Broadcast(RequestId, bSuccess, ErrorCode);
	OnMerchantRequestConfirmed.Broadcast(RequestId, bSuccess, ErrorCode);
}

void UPAServiceRequestComponent::Client_ConfirmForgeRequest_Implementation(int32 RequestId, bool bSuccess, EPACraftingError ErrorCode)
{
	OnForgeRequestConfirmedNative.Broadcast(RequestId, bSuccess, ErrorCode);
	OnForgeRequestConfirmed.Broadcast(RequestId, bSuccess, ErrorCode);
}
