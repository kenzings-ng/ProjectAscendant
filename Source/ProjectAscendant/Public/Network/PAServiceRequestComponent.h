// Copyright Project Ascendant. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Crafting/PABlacksmithTypes.h"
#include "Economy/PAMerchantTypes.h"
#include "Itemization/PASavedItemInstance.h"
#include "PAServiceRequestComponent.generated.h"

class AActor;
class APlayerController;
class UPABlacksmithComponent;
class UPACurrencyComponent;
class UPAInventoryComponent;
class UPAMerchantComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FPAOnMerchantRequestConfirmed, int32, RequestId, bool, bSuccess, EPATransactionError, ErrorCode);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FPAOnForgeRequestConfirmed, int32, RequestId, bool, bSuccess, EPACraftingError, ErrorCode);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FPAOnMerchantRequestConfirmedNative, int32 /*RequestId*/, bool /*bSuccess*/, EPATransactionError /*ErrorCode*/);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FPAOnForgeRequestConfirmedNative, int32 /*RequestId*/, bool /*bSuccess*/, EPACraftingError /*ErrorCode*/);

/**
 * UPAServiceRequestComponent (X11b, DECISIONS §11 / ADR-0003 / control-manifest "Dedicated Server Authority")
 *
 * Network plumbing only (no gameplay rules): routes a player's Merchant / Blacksmith requests to the server.
 * Naming follows architecture.md §5.4: Server_Request[Action] (client -> server), Client_Confirm[Action] (server -> client).
 *
 * Why: UPAMerchantComponent / UPABlacksmithComponent live on NPC actors (wandering smuggler, forges) that no client
 * connection owns, so a client cannot send Server RPCs on them. This component lives on the player's
 * APlayerController (owning connection), exposes the Server RPCs, and on the server:
 *   1. identifies the requesting player as the PlayerController that owns this component (never a client parameter);
 *   2. resolves the target NPC component from the target actor (null / non-merchant / non-forge -> ServerRejected);
 *   3. calls the NPC component's authority-only ServerHandle* function, which validates ownership of the supplied
 *      Inventory / Wallet, interaction distance and State.InCombat for that requesting player, then mutates state;
 *   4. sends the outcome back to the owning client via Client_Confirm* (RequestId echoed), which broadcasts
 *      OnMerchantRequestConfirmed / OnForgeRequestConfirmed. UI must react to these, not to its own request.
 *
 * Net modes: dedicated server (client -> server -> client RPCs), listen-host and standalone/PIE (a locally controlled
 * PlayerController executes both Server and Client RPCs locally), so the same path works everywhere.
 *
 * bSuccess semantics: Merchant — the transaction happened. Forge — the forge operation returned true; an enhancement
 * whose roll failed is processed with bSuccess=false and ErrorCode=None (crft-002 AC-1/AC-2).
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJECTASCENDANT_API UPAServiceRequestComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPAServiceRequestComponent();

	/** Client-side: allocates a new request id to correlate a request with its Client_Confirm* reply. */
	UFUNCTION(BlueprintCallable, Category = "ProjectAscendant|ServiceRequest")
	int32 AllocateRequestId() { return ++LastRequestId; }

	/** Server-side: the player on whose behalf requests on this component run (owner chain -> APlayerController). */
	APlayerController* GetRequestingPlayerController() const;

	/** Server-side: target resolution. nullptr if TargetActor is null or carries no such component. */
	static UPAMerchantComponent* ResolveMerchant(const AActor* TargetActor);
	static UPABlacksmithComponent* ResolveForge(const AActor* TargetActor);

	/**
	 * Server-side: binds this player's itemization (saved-item) inventory used by the *ByUID forge requests.
	 * Per player (this component), not per forge. Not wired yet: no production code owns a player
	 * TArray<FPASavedItemInstance> (see X11b report); until bound, *ByUID requests are rejected (ServerRejected).
	 */
	void BindSavedItemInventory(TArray<FPASavedItemInstance>* InItems, UPACurrencyComponent* InWallet);

	// -------------------------------------------------------------------------
	// Merchant requests (client -> server)
	// -------------------------------------------------------------------------

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "ProjectAscendant|ServiceRequest")
	void Server_RequestMerchantBuy(int32 RequestId, AActor* MerchantActor, UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet, int32 CatalogIndex, int32 Quantity);

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "ProjectAscendant|ServiceRequest")
	void Server_RequestMerchantSell(int32 RequestId, AActor* MerchantActor, UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet, int32 SlotIndex, int32 Quantity);

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "ProjectAscendant|ServiceRequest")
	void Server_RequestMerchantSellAllJunk(int32 RequestId, AActor* MerchantActor, UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet);

	/** Buys back one of the requester's own buyback entries on that NPC, identified by its item instance UID. */
	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "ProjectAscendant|ServiceRequest")
	void Server_RequestMerchantBuyback(int32 RequestId, AActor* MerchantActor, UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet, const FGuid& ItemInstanceUID);

	// -------------------------------------------------------------------------
	// Blacksmith requests (client -> server). No cost / tier / output parameters: the server decides them.
	// -------------------------------------------------------------------------

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "ProjectAscendant|ServiceRequest")
	void Server_RequestForgeRepair(int32 RequestId, AActor* ForgeActor, UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet, int32 SlotIndex);

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "ProjectAscendant|ServiceRequest")
	void Server_RequestForgeSalvage(int32 RequestId, AActor* ForgeActor, UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet, int32 SlotIndex);

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "ProjectAscendant|ServiceRequest")
	void Server_RequestForgeEnhance(int32 RequestId, AActor* ForgeActor, UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet, int32 SlotIndex, bool bUseWard);

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "ProjectAscendant|ServiceRequest")
	void Server_RequestForgeUnlockSocket(int32 RequestId, AActor* ForgeActor, UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet, int32 SlotIndex);

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "ProjectAscendant|ServiceRequest")
	void Server_RequestForgeSocketGem(int32 RequestId, AActor* ForgeActor, UPAInventoryComponent* Inventory, int32 EquipmentSlotIndex, int32 SocketIndex, FName GemItemId);

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "ProjectAscendant|ServiceRequest")
	void Server_RequestForgeUnsocketGem(int32 RequestId, AActor* ForgeActor, UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet, int32 EquipmentSlotIndex, int32 SocketIndex);

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "ProjectAscendant|ServiceRequest")
	void Server_RequestForgeBossSoul(int32 RequestId, AActor* ForgeActor, UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet, FName BossSoulItemId, FName BossPartItemId, FName VoidOreItemId);

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "ProjectAscendant|ServiceRequest")
	void Server_RequestForgeExpandBackpack(int32 RequestId, AActor* ForgeActor, UPAInventoryComponent* Inventory, UPACurrencyComponent* Wallet);

	/** Itemization (item-007): item addressed by UID in this player's bound saved-item inventory. */
	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "ProjectAscendant|ServiceRequest")
	void Server_RequestForgeRepairByUID(int32 RequestId, AActor* ForgeActor, const FGuid& ItemInstanceUID);

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "ProjectAscendant|ServiceRequest")
	void Server_RequestForgeReforgeAffixByUID(int32 RequestId, AActor* ForgeActor, const FGuid& ItemInstanceUID, int32 AffixIndex);

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "ProjectAscendant|ServiceRequest")
	void Server_RequestForgeAddSocketByUID(int32 RequestId, AActor* ForgeActor, const FGuid& ItemInstanceUID);

	// -------------------------------------------------------------------------
	// Server -> owning client confirmations
	// -------------------------------------------------------------------------

	UFUNCTION(Client, Reliable)
	void Client_ConfirmMerchantRequest(int32 RequestId, bool bSuccess, EPATransactionError ErrorCode);

	UFUNCTION(Client, Reliable)
	void Client_ConfirmForgeRequest(int32 RequestId, bool bSuccess, EPACraftingError ErrorCode);

	UPROPERTY(BlueprintAssignable, Category = "ProjectAscendant|ServiceRequest")
	FPAOnMerchantRequestConfirmed OnMerchantRequestConfirmed;

	UPROPERTY(BlueprintAssignable, Category = "ProjectAscendant|ServiceRequest")
	FPAOnForgeRequestConfirmed OnForgeRequestConfirmed;

	/** C++ listeners (widgets) — fired together with the dynamic delegates above. */
	FPAOnMerchantRequestConfirmedNative OnMerchantRequestConfirmedNative;
	FPAOnForgeRequestConfirmedNative OnForgeRequestConfirmedNative;

private:
	/** Server: resolves the forge or replies ServerRejected. */
	UPABlacksmithComponent* ResolveForgeOrReject(int32 RequestId, const AActor* ForgeActor);
	/** Server: resolves the merchant or replies ServerRejected. */
	UPAMerchantComponent* ResolveMerchantOrReject(int32 RequestId, const AActor* MerchantActor);

	int32 LastRequestId = 0;

	TArray<FPASavedItemInstance>* BoundSavedItems = nullptr;
	TWeakObjectPtr<UPACurrencyComponent> BoundWallet;
};
