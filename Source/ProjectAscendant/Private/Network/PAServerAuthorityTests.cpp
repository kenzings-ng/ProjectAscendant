// Copyright Project Ascendant. All Rights Reserved.

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"
#include "Crafting/PABlacksmithComponent.h"
#include "Crafting/PABlacksmithSubsystem.h"
#include "Crafting/PABlacksmithTypes.h"
#include "Economy/PACurrencyComponent.h"
#include "Economy/PACurrencyTypes.h"
#include "Economy/PAMerchantComponent.h"
#include "Economy/PAMerchantTypes.h"
#include "Inventory/PAInventoryComponent.h"
#include "Inventory/PAItemStaticDataAsset.h"
#include "Itemization/PAServerItemGeneratorSubsystem.h"
#include "Network/PAServerRequestValidation.h"
#include "Network/PAServiceRequestComponent.h"
#include <type_traits>

#if WITH_DEV_AUTOMATION_TESTS

/**
 * X11a server-authority regression tests (DECISIONS §11, ADR-0003, control-manifest "Dedicated Server Authority").
 * X11b: every request is sent the way a client sends it — through the requesting player's UPAServiceRequestComponent
 * (owned by its PlayerController) against NPC-owned forge / shop actors (no player owner) — and the outcome is read
 * from the Client_Confirm* reply as well as from the mutated state.
 *
 * Covers:
 *  - Costs / forge tier are computed by the server; RPCs carry no cost/tier/output parameters.
 *  - Client-supplied Inventory / Wallet components owned by another player are rejected.
 *  - Interaction distance (300 cm, merchant-economy.md / blacksmithing AC-4) and State.InCombat are enforced on RPC paths.
 *  - Boss Soul forging output comes from the server recipe only.
 */

// -----------------------------------------------------------------------------
// Compile-time guarantees: the client RPC surface has no cost / tier / output parameters.
// -----------------------------------------------------------------------------
static_assert(std::is_same_v<decltype(&UPAServiceRequestComponent::Server_ForgeRepairItemByUID), void (UPAServiceRequestComponent::*)(int32, AActor*, const FGuid&)>,
	"X11a/X11b: repair-by-UID request must not accept a client cost");
static_assert(std::is_same_v<decltype(&UPAServiceRequestComponent::Server_ForgeReforgeAffixByUID), void (UPAServiceRequestComponent::*)(int32, AActor*, const FGuid&, int32)>,
	"X11a/X11b: reforge request must not accept client cost/shards");
static_assert(std::is_same_v<decltype(&UPAServiceRequestComponent::Server_ForgeAddSocketByUID), void (UPAServiceRequestComponent::*)(int32, AActor*, const FGuid&)>,
	"X11a/X11b: add-socket request must not accept client cost/shards/forge tier");
static_assert(std::is_same_v<decltype(&UPAServiceRequestComponent::Server_ForgeBossSoul),
	void (UPAServiceRequestComponent::*)(int32, AActor*, UPAInventoryComponent*, UPACurrencyComponent*, FName, FName, FName)>,
	"X11a/X11b: boss soul request must not accept a client-chosen output item");

namespace PAServerAuthorityTestHelper
{
	static AActor* SpawnLocatedActor(UWorld* World, UClass* Class, const FVector& Location, AActor* Owner)
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		Params.Owner = Owner;
		AActor* Actor = World->SpawnActor<AActor>(Class, Location, FRotator::ZeroRotator, Params);
		if (Actor && !Actor->GetRootComponent())
		{
			USceneComponent* Root = NewObject<USceneComponent>(Actor, TEXT("TestRoot"));
			Actor->SetRootComponent(Root);
			Root->RegisterComponent();
		}
		if (Actor)
		{
			Actor->SetActorLocation(Location);
		}
		return Actor;
	}

	struct FTestPlayer
	{
		APlayerController* PC = nullptr;
		APawn* Pawn = nullptr;
		UPAInventoryComponent* Inventory = nullptr;
		UPACurrencyComponent* Wallet = nullptr;
		UPAServiceRequestComponent* Router = nullptr; // X11b: lives on the PlayerController like in production
	};

	/** X11b: records Client_Confirm* replies delivered to a router (native mirror of the client delegates). */
	struct FConfirmLog
	{
		int32 MerchantCount = 0;
		int32 LastMerchantRequestId = INDEX_NONE;
		bool bLastMerchantSuccess = false;
		EPATransactionError LastMerchantError = EPATransactionError::None;

		int32 ForgeCount = 0;
		int32 LastForgeRequestId = INDEX_NONE;
		bool bLastForgeSuccess = false;
		EPACraftingError LastForgeError = EPACraftingError::None;
	};

	static TSharedRef<FConfirmLog> AttachConfirmLog(UPAServiceRequestComponent* Router)
	{
		TSharedRef<FConfirmLog> Log = MakeShared<FConfirmLog>();
		Router->OnMerchantRequestConfirmedNative.AddLambda([Log](int32 RequestId, bool bSuccess, EPATransactionError Error)
		{
			++Log->MerchantCount;
			Log->LastMerchantRequestId = RequestId;
			Log->bLastMerchantSuccess = bSuccess;
			Log->LastMerchantError = Error;
		});
		Router->OnForgeRequestConfirmedNative.AddLambda([Log](int32 RequestId, bool bSuccess, EPACraftingError Error)
		{
			++Log->ForgeCount;
			Log->LastForgeRequestId = RequestId;
			Log->bLastForgeSuccess = bSuccess;
			Log->LastForgeError = Error;
		});
		return Log;
	}

	static FTestPlayer SpawnPlayer(UWorld* World, const FVector& PawnLocation)
	{
		FTestPlayer Player;
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		Player.PC = World->SpawnActor<APlayerController>(APlayerController::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, Params);
		Player.Pawn = Cast<APawn>(SpawnLocatedActor(World, APawn::StaticClass(), PawnLocation, Player.PC));
		if (Player.PC && Player.Pawn)
		{
			Player.PC->SetPawn(Player.Pawn);
			Player.Inventory = NewObject<UPAInventoryComponent>(Player.Pawn, TEXT("TestInventory"));
			Player.Wallet = NewObject<UPACurrencyComponent>(Player.Pawn, TEXT("TestWallet"));
			Player.Router = NewObject<UPAServiceRequestComponent>(Player.PC, TEXT("TestRouter"));
			Player.Router->RegisterComponent();
		}
		return Player;
	}

	static void DestroyPlayer(FTestPlayer& Player)
	{
		if (Player.PC) { Player.PC->SetPawn(nullptr); }
		if (Player.Pawn) { Player.Pawn->Destroy(); }
		if (Player.PC) { Player.PC->Destroy(); }
		Player = FTestPlayer();
	}

	static UItemStaticDataAsset* MakeItem(FName ItemId, EPAItemCategory Category, EPAItemRarity Rarity, int32 BaseSellPrice, int32 MaxStack)
	{
		UItemStaticDataAsset* Asset = NewObject<UItemStaticDataAsset>();
		Asset->ItemId = ItemId;
		Asset->Category = Category;
		Asset->RarityTier = Rarity;
		Asset->BaseSellPrice = BaseSellPrice;
		Asset->MaxStackSize = MaxStack;
		return Asset;
	}
}

using namespace PAServerAuthorityTestHelper;

// =============================================================================
// 1. Server computes cost and forge tier
// =============================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPAServerAuthorityComputedCostsTest,
	"ProjectAscendant.Network.ServerAuthority.ServerComputedCostsAndTier",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPAServerAuthorityComputedCostsTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!TestNotNull(TEXT("World available"), World))
	{
		return false;
	}

	UGameInstance* GameInstance = NewObject<UGameInstance>();
	UPAServerItemGeneratorSubsystem* ItemGenerator = NewObject<UPAServerItemGeneratorSubsystem>(GameInstance);
	ItemGenerator->InitializeAffixDefinitions(nullptr);

	FTestPlayer PlayerA = SpawnPlayer(World, FVector(0.0f, 0.0f, 0.0f));
	// X11b: NPC forge (no player owner), reached through Player A's router.
	AActor* Forge = SpawnLocatedActor(World, AActor::StaticClass(), FVector(100.0f, 0.0f, 0.0f), nullptr);
	UPABlacksmithComponent* Blacksmith = NewObject<UPABlacksmithComponent>(Forge, TEXT("TestForge"));
	if (!TestNotNull(TEXT("Player A spawned"), PlayerA.Pawn) || !TestNotNull(TEXT("Forge component"), Blacksmith))
	{
		DestroyPlayer(PlayerA);
		return false;
	}

	EPACurrencyTransactionError CurrErr;
	PlayerA.Wallet->AddCurrency(EPACurrencyType::Gold, 10000, CurrErr);
	PlayerA.Wallet->AddCurrency(EPACurrencyType::SkillShards, 20, CurrErr);

	// --- Repair: cost charged == server formula (Rare base 300: ceil(300 * 0.25 * 0.4) = 30) ---
	TArray<FPASavedItemInstance> SavedItems;
	FPASavedItemInstance DamagedItem = ItemGenerator->GenerateItemInstance(FName("SteelMace"), 15, EPAItemRarity::Rare, EPAForgeTier::Tier2_Field);
	DamagedItem.CurrentDurability = 60.0f;
	SavedItems.Add(DamagedItem);
	const int32 FormulaCost = UPABlacksmithSubsystem::GetRepairCost(DamagedItem);
	TestEqual(TEXT("Repair formula cost is 30 for Rare at 60% durability"), FormulaCost, 30);

	TSharedRef<FConfirmLog> Log = AttachConfirmLog(PlayerA.Router);

	// Unbound saved-item inventory: fail closed (ServerRejected), nothing charged.
	PlayerA.Router->Server_ForgeRepairItemByUID(1, Forge, DamagedItem.ItemInstanceUID);
	TestEqual(TEXT("Unbound saved items: confirmation delivered"), Log->ForgeCount, 1);
	TestFalse(TEXT("Unbound saved items: rejected"), Log->bLastForgeSuccess);
	TestEqual(TEXT("Unbound saved items: ServerRejected"), Log->LastForgeError, EPACraftingError::ServerRejected);
	TestEqual(TEXT("Unbound saved items: durability untouched"), SavedItems[0].CurrentDurability, 60.0f);

	PlayerA.Router->BindSavedItemInventory(&SavedItems, PlayerA.Wallet);
	PlayerA.Router->Server_ForgeRepairItemByUID(2, Forge, DamagedItem.ItemInstanceUID);
	TestEqual(TEXT("Repair confirmation echoes request id"), Log->LastForgeRequestId, 2);
	TestTrue(TEXT("Repair confirmed as success"), Log->bLastForgeSuccess);
	TestEqual(TEXT("Repair RPC restored durability"), SavedItems[0].CurrentDurability, 100.0f);
	TestEqual(TEXT("Repair RPC charged exactly the server formula cost"), PlayerA.Wallet->GetGold(), 10000LL - FormulaCost);

	// --- AddSocket: forge tier comes from the server-side forge, cost from the socket index ---
	FPASavedItemInstance SocketItem = ItemGenerator->GenerateItemInstance(FName("ShadowDagger"), 25, EPAItemRarity::Rare, EPAForgeTier::Tier2_Field);
	SavedItems.Add(SocketItem);
	const int64 GoldBeforeSocket = PlayerA.Wallet->GetGold();
	const int64 ShardsBeforeSocket = PlayerA.Wallet->GetSkillShards();

	Blacksmith->SetForgeTier(EPABlacksmithTier::Tier1_Outpost);
	PlayerA.Router->Server_ForgeAddSocketByUID(3, Forge, SocketItem.ItemInstanceUID);
	TestFalse(TEXT("Tier 1 add-socket confirmed as failure"), Log->bLastForgeSuccess);
	TestFalse(TEXT("Tier 1 forge (server state) refuses socketing"), SavedItems[1].SocketSlots[0].bIsUnlocked);
	TestEqual(TEXT("No gold charged at Tier 1 forge"), PlayerA.Wallet->GetGold(), GoldBeforeSocket);

	Blacksmith->SetForgeTier(EPABlacksmithTier::Tier2_Wilderness);
	PlayerA.Router->Server_ForgeAddSocketByUID(4, Forge, SocketItem.ItemInstanceUID);
	TestTrue(TEXT("Tier 2 add-socket confirmed as success"), Log->bLastForgeSuccess);
	TestTrue(TEXT("Tier 2 forge (server state) unlocks socket 0"), SavedItems[1].SocketSlots[0].bIsUnlocked);
	TestEqual(TEXT("Socket 0 charged 1,000 Gold (itemization.md 7.2)"), PlayerA.Wallet->GetGold(), GoldBeforeSocket - 1000LL);
	TestEqual(TEXT("Socket 0 charged 3 Shards (itemization.md 7.2)"), PlayerA.Wallet->GetSkillShards(), ShardsBeforeSocket - 3LL);

	// --- Reforge cost constant is the GDD value (no client override exists) ---
	int32 ReforgeGold = 0;
	int32 ReforgeShards = 0;
	FPABlacksmithFormulas::GetReforgeAffixCost(ReforgeGold, ReforgeShards);
	TestEqual(TEXT("Reforge gold cost is 2,000"), ReforgeGold, 2000);
	TestEqual(TEXT("Reforge shard cost is 5"), ReforgeShards, 5);

	PlayerA.Router->BindSavedItemInventory(nullptr, nullptr);
	Forge->Destroy();
	DestroyPlayer(PlayerA);
	return true;
}

// =============================================================================
// 1b. Unlock-socket request charges the server socket cost (no free-socket bypass)
// =============================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPAServerAuthorityUnlockSocketChargedTest,
	"ProjectAscendant.Network.ServerAuthority.UnlockSocketChargedServerSide",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPAServerAuthorityUnlockSocketChargedTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!TestNotNull(TEXT("World available"), World))
	{
		return false;
	}

	FTestPlayer PlayerA = SpawnPlayer(World, FVector(0.0f, 0.0f, 0.0f));
	AActor* Forge = SpawnLocatedActor(World, AActor::StaticClass(), FVector(100.0f, 0.0f, 0.0f), nullptr);
	UPABlacksmithComponent* Blacksmith = NewObject<UPABlacksmithComponent>(Forge, TEXT("TestForge"));
	if (!TestNotNull(TEXT("Player A"), PlayerA.Pawn))
	{
		if (Forge) { Forge->Destroy(); }
		DestroyPlayer(PlayerA);
		return false;
	}
	Blacksmith->SetForgeTier(EPABlacksmithTier::Tier2_Wilderness);

	UItemStaticDataAsset* RareSword = MakeItem(FName("item_test_rare_sword"), EPAItemCategory::Equipment, EPAItemRarity::Rare, 300, 1);
	PlayerA.Inventory->AddItemToSlot(0, RareSword, 1);

	auto SocketCount = [&PlayerA]() { return PlayerA.Inventory->GetItemAtSlot(0)->DynamicData.SocketedGemIds.Num(); };

	// --- Insufficient gold (999 < 1,000): rejected, nothing deducted, no socket ---
	EPACurrencyTransactionError CurrErr;
	PlayerA.Wallet->AddCurrency(EPACurrencyType::Gold, 999, CurrErr);
	PlayerA.Wallet->AddCurrency(EPACurrencyType::SkillShards, 3, CurrErr);
	PlayerA.Router->Server_ForgeUnlockSocket(0, Forge, PlayerA.Inventory, PlayerA.Wallet, 0);
	TestEqual(TEXT("Insufficient gold: no socket opened"), SocketCount(), 0);
	TestEqual(TEXT("Insufficient gold: gold untouched"), PlayerA.Wallet->GetGold(), 999LL);
	TestEqual(TEXT("Insufficient gold: shards untouched"), PlayerA.Wallet->GetSkillShards(), 3LL);

	// --- Insufficient shards (2 < 3): rejected, nothing deducted, no socket ---
	PlayerA.Wallet->AddCurrency(EPACurrencyType::Gold, 1, CurrErr);       // 1,000 Gold
	PlayerA.Wallet->DeductCurrency(EPACurrencyType::SkillShards, 1, CurrErr); // 2 Shards
	PlayerA.Router->Server_ForgeUnlockSocket(0, Forge, PlayerA.Inventory, PlayerA.Wallet, 0);
	TestEqual(TEXT("Insufficient shards: no socket opened"), SocketCount(), 0);
	TestEqual(TEXT("Insufficient shards: gold untouched"), PlayerA.Wallet->GetGold(), 1000LL);
	TestEqual(TEXT("Insufficient shards: shards untouched"), PlayerA.Wallet->GetSkillShards(), 2LL);

	// --- Sufficient: socket 1 costs 1,000 Gold + 3 Shards ---
	PlayerA.Wallet->AddCurrency(EPACurrencyType::SkillShards, 1, CurrErr); // 3 Shards
	PlayerA.Router->Server_ForgeUnlockSocket(0, Forge, PlayerA.Inventory, PlayerA.Wallet, 0);
	TestEqual(TEXT("Socket 1 opened"), SocketCount(), 1);
	TestEqual(TEXT("Socket 1 charged 1,000 Gold"), PlayerA.Wallet->GetGold(), 0LL);
	TestEqual(TEXT("Socket 1 charged 3 Shards"), PlayerA.Wallet->GetSkillShards(), 0LL);

	// --- Socket 2 costs 3,000 Gold + 8 Shards ---
	PlayerA.Wallet->AddCurrency(EPACurrencyType::Gold, 3500, CurrErr);
	PlayerA.Wallet->AddCurrency(EPACurrencyType::SkillShards, 10, CurrErr);
	PlayerA.Router->Server_ForgeUnlockSocket(0, Forge, PlayerA.Inventory, PlayerA.Wallet, 0);
	TestEqual(TEXT("Socket 2 opened"), SocketCount(), 2);
	TestEqual(TEXT("Socket 2 charged 3,000 Gold"), PlayerA.Wallet->GetGold(), 500LL);
	TestEqual(TEXT("Socket 2 charged 8 Shards"), PlayerA.Wallet->GetSkillShards(), 2LL);

	Forge->Destroy();
	DestroyPlayer(PlayerA);
	return true;
}

// =============================================================================
// 2. Client-supplied Inventory / Wallet must belong to the requesting player
// =============================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPAServerAuthorityForeignOwnerTest,
	"ProjectAscendant.Network.ServerAuthority.ForeignOwnerComponentsRejected",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPAServerAuthorityForeignOwnerTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!TestNotNull(TEXT("World available"), World))
	{
		return false;
	}

	FTestPlayer PlayerA = SpawnPlayer(World, FVector(0.0f, 0.0f, 0.0f));
	FTestPlayer PlayerB = SpawnPlayer(World, FVector(50.0f, 0.0f, 0.0f));
	AActor* Forge = SpawnLocatedActor(World, AActor::StaticClass(), FVector(100.0f, 0.0f, 0.0f), nullptr);
	AActor* Shop = SpawnLocatedActor(World, AActor::StaticClass(), FVector(-100.0f, 0.0f, 0.0f), nullptr);
	UPABlacksmithComponent* Blacksmith = NewObject<UPABlacksmithComponent>(Forge, TEXT("TestForge"));
	UPAMerchantComponent* Merchant = NewObject<UPAMerchantComponent>(Shop, TEXT("TestMerchant"));
	if (!TestNotNull(TEXT("Player A"), PlayerA.Pawn) || !TestNotNull(TEXT("Player B"), PlayerB.Pawn))
	{
		DestroyPlayer(PlayerA);
		DestroyPlayer(PlayerB);
		return false;
	}

	TestTrue(TEXT("Router A's requesting player is PC A"), PlayerA.Router->GetRequestingPlayerController() == PlayerA.PC);
	TestTrue(TEXT("Router B's requesting player is PC B"), PlayerB.Router->GetRequestingPlayerController() == PlayerB.PC);
	TestTrue(TEXT("Inventory A resolves to PC A"), PAServerRequestValidation::IsComponentOwnedBy(PlayerA.Inventory, PlayerA.PC));
	TestFalse(TEXT("Inventory B does not resolve to PC A"), PAServerRequestValidation::IsComponentOwnedBy(PlayerB.Inventory, PlayerA.PC));

	EPACurrencyTransactionError CurrErr;
	PlayerA.Wallet->AddCurrency(EPACurrencyType::Gold, 1000, CurrErr);
	PlayerB.Wallet->AddCurrency(EPACurrencyType::Gold, 1000, CurrErr);

	UItemStaticDataAsset* Sword = MakeItem(FName("item_test_sword"), EPAItemCategory::Equipment, EPAItemRarity::Rare, 300, 1);
	FPAItemInstanceData Damaged;
	Damaged.CurrentDurability = 20.0f;
	PlayerA.Inventory->AddItemToSlot(0, Sword, 1, Damaged);
	PlayerB.Inventory->AddItemToSlot(0, Sword, 1, Damaged);

	// --- Blacksmith: foreign inventory / foreign wallet ---
	EPACraftingError CraftErr = EPACraftingError::None;
	TestFalse(TEXT("Validate: foreign inventory rejected"), Blacksmith->ValidateServerRequest(PlayerA.PC, PlayerB.Inventory, PlayerA.Wallet, CraftErr));
	TestEqual(TEXT("Validate: foreign inventory -> ServerRejected"), CraftErr, EPACraftingError::ServerRejected);
	TestFalse(TEXT("Validate: foreign wallet rejected"), Blacksmith->ValidateServerRequest(PlayerA.PC, PlayerA.Inventory, PlayerB.Wallet, CraftErr));
	TestEqual(TEXT("Validate: foreign wallet -> ServerRejected"), CraftErr, EPACraftingError::ServerRejected);

	TSharedRef<FConfirmLog> LogA = AttachConfirmLog(PlayerA.Router);
	TSharedRef<FConfirmLog> LogB = AttachConfirmLog(PlayerB.Router);

	PlayerA.Router->Server_ForgeRepair(10, Forge, PlayerB.Inventory, PlayerA.Wallet, 0);
	TestFalse(TEXT("Repair with B's inventory: confirmed as failure"), LogA->bLastForgeSuccess);
	TestEqual(TEXT("Repair with B's inventory: ServerRejected"), LogA->LastForgeError, EPACraftingError::ServerRejected);
	TestEqual(TEXT("Repair RPC with B's inventory: B's item untouched"), PlayerB.Inventory->GetItemAtSlot(0)->DynamicData.CurrentDurability, 20.0f);
	TestEqual(TEXT("Repair RPC with B's inventory: A's gold untouched"), PlayerA.Wallet->GetGold(), 1000LL);

	PlayerA.Router->Server_ForgeRepair(11, Forge, PlayerA.Inventory, PlayerB.Wallet, 0);
	TestEqual(TEXT("Repair RPC with B's wallet: A's item untouched"), PlayerA.Inventory->GetItemAtSlot(0)->DynamicData.CurrentDurability, 20.0f);
	TestEqual(TEXT("Repair RPC with B's wallet: B's gold untouched"), PlayerB.Wallet->GetGold(), 1000LL);

	// B's router acting on A's components: rejected (requesting player is B).
	PlayerB.Router->Server_ForgeRepair(20, Forge, PlayerA.Inventory, PlayerA.Wallet, 0);
	TestEqual(TEXT("B's router on A's components: confirmed to B"), LogB->ForgeCount, 1);
	TestEqual(TEXT("B's router on A's components: ServerRejected"), LogB->LastForgeError, EPACraftingError::ServerRejected);
	TestEqual(TEXT("B's router on A's components: A's item untouched"), PlayerA.Inventory->GetItemAtSlot(0)->DynamicData.CurrentDurability, 20.0f);
	TestEqual(TEXT("B's router on A's components: A's gold untouched"), PlayerA.Wallet->GetGold(), 1000LL);

	// Positive control: own components succeed (repair cost = ceil(300 * 0.25 * 0.8) = 60)
	PlayerA.Router->Server_ForgeRepair(12, Forge, PlayerA.Inventory, PlayerA.Wallet, 0);
	TestTrue(TEXT("Repair with own components: confirmed as success"), LogA->bLastForgeSuccess && LogA->LastForgeRequestId == 12);
	TestEqual(TEXT("Repair RPC with own components: repaired"), PlayerA.Inventory->GetItemAtSlot(0)->DynamicData.CurrentDurability, 100.0f);
	TestEqual(TEXT("Repair RPC with own components: charged 60 Gold"), PlayerA.Wallet->GetGold(), 940LL);

	// --- Merchant: selling someone else's item into someone else's wallet ---
	EPATransactionError TransErr = EPATransactionError::None;
	TestFalse(TEXT("Merchant validate: foreign inventory rejected"), Merchant->ValidateServerRequest(PlayerA.PC, PlayerB.Inventory, PlayerA.Wallet, TransErr));
	TestEqual(TEXT("Merchant validate: foreign inventory -> ServerRejected"), TransErr, EPATransactionError::ServerRejected);

	PlayerA.Router->Server_MerchantSellItem(13, Shop, PlayerB.Inventory, PlayerA.Wallet, 0, 1);
	TestFalse(TEXT("Sell with B's inventory: confirmed as failure"), LogA->bLastMerchantSuccess);
	TestEqual(TEXT("Sell with B's inventory: ServerRejected"), LogA->LastMerchantError, EPATransactionError::ServerRejected);
	TestNotNull(TEXT("Sell RPC with B's inventory: B's item still present"), PlayerB.Inventory->GetItemAtSlot(0));
	TestEqual(TEXT("Sell RPC with B's inventory: A's gold unchanged"), PlayerA.Wallet->GetGold(), 940LL);
	TestEqual(TEXT("Sell RPC with B's inventory: no buyback entry"), Merchant->GetBuybackCount(), 0);

	// Positive control: selling own item works (floor(300 * 0.30) = 90)
	PlayerA.Router->Server_MerchantSellItem(14, Shop, PlayerA.Inventory, PlayerA.Wallet, 0, 1);
	TestTrue(TEXT("Sell with own components: confirmed as success"), LogA->bLastMerchantSuccess && LogA->LastMerchantRequestId == 14);
	TestNull(TEXT("Sell RPC with own components: item removed"), PlayerA.Inventory->GetItemAtSlot(0));
	TestEqual(TEXT("Sell RPC with own components: received 90 Gold"), PlayerA.Wallet->GetGold(), 1030LL);

	// --- X11b: target validation. Non-merchant / non-forge / missing targets are rejected, nothing mutates ---
	AActor* PlainActor = SpawnLocatedActor(World, AActor::StaticClass(), FVector(10.0f, 0.0f, 0.0f), nullptr);
	PlayerB.Router->Server_ForgeRepair(21, PlainActor, PlayerB.Inventory, PlayerB.Wallet, 0);
	TestEqual(TEXT("Non-forge target: ServerRejected"), LogB->LastForgeError, EPACraftingError::ServerRejected);
	PlayerB.Router->Server_ForgeRepair(22, Shop, PlayerB.Inventory, PlayerB.Wallet, 0);
	TestEqual(TEXT("Merchant actor as forge target: ServerRejected"), LogB->LastForgeError, EPACraftingError::ServerRejected);
	PlayerB.Router->Server_ForgeRepair(23, nullptr, PlayerB.Inventory, PlayerB.Wallet, 0);
	TestEqual(TEXT("Null forge target: ServerRejected"), LogB->LastForgeError, EPACraftingError::ServerRejected);
	TestEqual(TEXT("Rejected forge targets: B's item untouched"), PlayerB.Inventory->GetItemAtSlot(0)->DynamicData.CurrentDurability, 20.0f);
	TestEqual(TEXT("Rejected forge targets: B's gold untouched"), PlayerB.Wallet->GetGold(), 1000LL);

	PlayerB.Router->Server_MerchantSellItem(24, Forge, PlayerB.Inventory, PlayerB.Wallet, 0, 1);
	TestFalse(TEXT("Non-merchant target: confirmed as failure"), LogB->bLastMerchantSuccess);
	TestEqual(TEXT("Non-merchant target: ServerRejected"), LogB->LastMerchantError, EPATransactionError::ServerRejected);
	PlayerB.Router->Server_MerchantSellItem(25, PlainActor, PlayerB.Inventory, PlayerB.Wallet, 0, 1);
	TestEqual(TEXT("Plain actor as merchant target: ServerRejected"), LogB->LastMerchantError, EPATransactionError::ServerRejected);
	TestNotNull(TEXT("Rejected merchant targets: B's item still present"), PlayerB.Inventory->GetItemAtSlot(0));
	TestEqual(TEXT("Rejected merchant targets: B's gold untouched"), PlayerB.Wallet->GetGold(), 1000LL);

	PlainActor->Destroy();
	Forge->Destroy();
	Shop->Destroy();
	DestroyPlayer(PlayerA);
	DestroyPlayer(PlayerB);
	return true;
}

// =============================================================================
// 3. Interaction distance (300 cm) and State.InCombat enforced on RPC paths
// =============================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPAServerAuthorityInteractionTest,
	"ProjectAscendant.Network.ServerAuthority.InteractionRangeAndCombatEnforced",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPAServerAuthorityInteractionTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!TestNotNull(TEXT("World available"), World))
	{
		return false;
	}

	FTestPlayer PlayerA = SpawnPlayer(World, FVector(0.0f, 0.0f, 0.0f));
	AActor* Forge = SpawnLocatedActor(World, AActor::StaticClass(), FVector(0.0f, 0.0f, 0.0f), nullptr);
	AActor* Shop = SpawnLocatedActor(World, AActor::StaticClass(), FVector(0.0f, 0.0f, 0.0f), nullptr);
	UPABlacksmithComponent* Blacksmith = NewObject<UPABlacksmithComponent>(Forge, TEXT("TestForge"));
	UPAMerchantComponent* Merchant = NewObject<UPAMerchantComponent>(Shop, TEXT("TestMerchant"));
	if (!TestNotNull(TEXT("Player A"), PlayerA.Pawn))
	{
		DestroyPlayer(PlayerA);
		return false;
	}

	EPACurrencyTransactionError CurrErr;
	PlayerA.Wallet->AddCurrency(EPACurrencyType::Gold, 1000, CurrErr);

	UItemStaticDataAsset* Sword = MakeItem(FName("item_test_sword"), EPAItemCategory::Equipment, EPAItemRarity::Rare, 300, 1);
	FPAItemInstanceData Damaged;
	Damaged.CurrentDurability = 20.0f;
	PlayerA.Inventory->AddItemToSlot(0, Sword, 1, Damaged);

	UItemStaticDataAsset* Potion = MakeItem(FName("item_test_potion"), EPAItemCategory::Consumable, EPAItemRarity::Common, 10, 99);
	FPAMerchantCatalogEntry CatalogEntry;
	CatalogEntry.ItemData = Potion;
	CatalogEntry.PriceGold = 10;
	CatalogEntry.AvailableStock = -1;
	Merchant->AddCatalogEntry(CatalogEntry);

	EPACraftingError CraftErr = EPACraftingError::None;
	EPATransactionError TransErr = EPATransactionError::None;
	TSharedRef<FConfirmLog> Log = AttachConfirmLog(PlayerA.Router);

	// --- Within 300 cm (290 cm): allowed ---
	PlayerA.Pawn->SetActorLocation(FVector(290.0f, 0.0f, 0.0f));
	TestTrue(TEXT("Forge: 290 cm accepted"), Blacksmith->ValidateServerRequest(PlayerA.PC, PlayerA.Inventory, PlayerA.Wallet, CraftErr));
	TestTrue(TEXT("Merchant: 290 cm accepted"), Merchant->ValidateServerRequest(PlayerA.PC, PlayerA.Inventory, PlayerA.Wallet, TransErr));

	// --- Out of range (310 cm): rejected with DistanceExceeded, RPCs mutate nothing ---
	PlayerA.Pawn->SetActorLocation(FVector(310.0f, 0.0f, 0.0f));
	TestFalse(TEXT("Forge: 310 cm rejected"), Blacksmith->ValidateServerRequest(PlayerA.PC, PlayerA.Inventory, PlayerA.Wallet, CraftErr));
	TestEqual(TEXT("Forge: error DistanceExceeded"), CraftErr, EPACraftingError::DistanceExceeded);
	TestFalse(TEXT("Merchant: 310 cm rejected"), Merchant->ValidateServerRequest(PlayerA.PC, PlayerA.Inventory, PlayerA.Wallet, TransErr));
	TestEqual(TEXT("Merchant: error DistanceExceeded"), TransErr, EPATransactionError::DistanceExceeded);

	PlayerA.Router->Server_ForgeRepair(1, Forge, PlayerA.Inventory, PlayerA.Wallet, 0);
	TestEqual(TEXT("Out-of-range repair: confirmed DistanceExceeded"), Log->LastForgeError, EPACraftingError::DistanceExceeded);
	TestFalse(TEXT("Out-of-range repair: confirmed as failure"), Log->bLastForgeSuccess);
	TestEqual(TEXT("Out-of-range repair RPC: item untouched"), PlayerA.Inventory->GetItemAtSlot(0)->DynamicData.CurrentDurability, 20.0f);
	TestEqual(TEXT("Out-of-range repair RPC: gold untouched"), PlayerA.Wallet->GetGold(), 1000LL);

	PlayerA.Router->Server_MerchantBuyItem(2, Shop, PlayerA.Inventory, PlayerA.Wallet, 0, 1);
	TestEqual(TEXT("Out-of-range buy: confirmed DistanceExceeded"), Log->LastMerchantError, EPATransactionError::DistanceExceeded);
	TestFalse(TEXT("Out-of-range buy: confirmed as failure"), Log->bLastMerchantSuccess);
	TestEqual(TEXT("Out-of-range buy RPC: no potion added"), PlayerA.Inventory->GetItemCount(FName("item_test_potion")), 0);
	TestEqual(TEXT("Out-of-range buy RPC: gold untouched"), PlayerA.Wallet->GetGold(), 1000LL);

	// --- In range but in combat (State.InCombat on the pawn's ASC): rejected with InCombat ---
	PlayerA.Pawn->SetActorLocation(FVector(100.0f, 0.0f, 0.0f));
	UAbilitySystemComponent* ASC = NewObject<UAbilitySystemComponent>(PlayerA.Pawn, TEXT("TestASC"));
	ASC->RegisterComponent();
	ASC->InitAbilityActorInfo(PlayerA.Pawn, PlayerA.Pawn);
	const FGameplayTag InCombatTag = FGameplayTag::RequestGameplayTag(FName("State.InCombat"), false);
	TestTrue(TEXT("State.InCombat tag is registered"), InCombatTag.IsValid());
	ASC->AddLooseGameplayTag(InCombatTag);
	TestTrue(TEXT("Pawn reported in combat"), PAServerRequestValidation::IsActorInCombat(PlayerA.Pawn));

	TestFalse(TEXT("Forge: in-combat rejected"), Blacksmith->ValidateServerRequest(PlayerA.PC, PlayerA.Inventory, PlayerA.Wallet, CraftErr));
	TestEqual(TEXT("Forge: error InCombat"), CraftErr, EPACraftingError::InCombat);
	TestFalse(TEXT("Merchant: in-combat rejected"), Merchant->ValidateServerRequest(PlayerA.PC, PlayerA.Inventory, PlayerA.Wallet, TransErr));
	TestEqual(TEXT("Merchant: error InCombat"), TransErr, EPATransactionError::InCombat);

	PlayerA.Router->Server_ForgeRepair(3, Forge, PlayerA.Inventory, PlayerA.Wallet, 0);
	TestEqual(TEXT("In-combat repair: confirmed InCombat"), Log->LastForgeError, EPACraftingError::InCombat);
	TestEqual(TEXT("In-combat repair RPC: item untouched"), PlayerA.Inventory->GetItemAtSlot(0)->DynamicData.CurrentDurability, 20.0f);
	PlayerA.Router->Server_MerchantBuyItem(4, Shop, PlayerA.Inventory, PlayerA.Wallet, 0, 1);
	TestEqual(TEXT("In-combat buy: confirmed InCombat"), Log->LastMerchantError, EPATransactionError::InCombat);
	TestEqual(TEXT("In-combat buy RPC: no potion added"), PlayerA.Inventory->GetItemCount(FName("item_test_potion")), 0);
	TestEqual(TEXT("In-combat requests: gold untouched"), PlayerA.Wallet->GetGold(), 1000LL);

	// --- Out of combat, in range (290 cm): both requests go through the router ---
	ASC->RemoveLooseGameplayTag(InCombatTag);
	PlayerA.Pawn->SetActorLocation(FVector(290.0f, 0.0f, 0.0f));
	PlayerA.Router->Server_ForgeRepair(5, Forge, PlayerA.Inventory, PlayerA.Wallet, 0);
	TestTrue(TEXT("In-range repair: confirmed as success"), Log->bLastForgeSuccess && Log->LastForgeRequestId == 5);
	TestEqual(TEXT("In-range repair: no error"), Log->LastForgeError, EPACraftingError::None);
	TestEqual(TEXT("In-range repair RPC: repaired"), PlayerA.Inventory->GetItemAtSlot(0)->DynamicData.CurrentDurability, 100.0f);
	TestEqual(TEXT("In-range repair RPC: charged 60 Gold"), PlayerA.Wallet->GetGold(), 940LL);
	PlayerA.Router->Server_MerchantBuyItem(6, Shop, PlayerA.Inventory, PlayerA.Wallet, 0, 1);
	TestTrue(TEXT("In-range buy: confirmed as success"), Log->bLastMerchantSuccess && Log->LastMerchantRequestId == 6);
	TestEqual(TEXT("In-range buy RPC: potion added"), PlayerA.Inventory->GetItemCount(FName("item_test_potion")), 1);
	TestEqual(TEXT("In-range buy RPC: charged 10 Gold"), PlayerA.Wallet->GetGold(), 930LL);

	Forge->Destroy();
	Shop->Destroy();
	DestroyPlayer(PlayerA);
	return true;
}

// =============================================================================
// 4. Boss Soul output is determined by the server recipe, not the client
// =============================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPAServerAuthorityBossSoulTest,
	"ProjectAscendant.Network.ServerAuthority.BossSoulOutputServerDetermined",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPAServerAuthorityBossSoulTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEngine->GetWorldContexts()[0].World();
	if (!TestNotNull(TEXT("World available"), World))
	{
		return false;
	}

	FTestPlayer PlayerA = SpawnPlayer(World, FVector(0.0f, 0.0f, 0.0f));
	AActor* Forge = SpawnLocatedActor(World, AActor::StaticClass(), FVector(100.0f, 0.0f, 0.0f), nullptr);
	UPABlacksmithComponent* Blacksmith = NewObject<UPABlacksmithComponent>(Forge, TEXT("TestForge"));
	if (!TestNotNull(TEXT("Player A"), PlayerA.Pawn))
	{
		DestroyPlayer(PlayerA);
		return false;
	}
	Blacksmith->SetForgeTier(EPABlacksmithTier::Tier3_Sanctuary);

	UItemStaticDataAsset* RecipeOutput = MakeItem(FName("item_boss_soul_golem_greatsword"), EPAItemCategory::Equipment, EPAItemRarity::Legendary, 5000, 1);
	UItemStaticDataAsset* MisconfiguredOutput = MakeItem(FName("item_boss_soul_cheap_output"), EPAItemCategory::Equipment, EPAItemRarity::Rare, 10, 1);

	FPABossSoulRecipe GoodRecipe;
	GoodRecipe.BossSoulItemId = FName("item_boss_soul_golem");
	GoodRecipe.OutputItemData = RecipeOutput;
	Blacksmith->AddBossSoulRecipe(GoodRecipe);

	FPABossSoulRecipe BadRecipe;
	BadRecipe.BossSoulItemId = FName("item_boss_soul_misconfigured");
	BadRecipe.OutputItemData = MisconfiguredOutput;
	Blacksmith->AddBossSoulRecipe(BadRecipe);

	EPACurrencyTransactionError CurrErr;
	PlayerA.Wallet->AddCurrency(EPACurrencyType::Gold, 20000, CurrErr);
	PlayerA.Inventory->AddItemToSlot(1, MakeItem(FName("item_boss_soul_golem"), EPAItemCategory::Material, EPAItemRarity::None, 0, 10), 1);
	PlayerA.Inventory->AddItemToSlot(2, MakeItem(FName("item_boss_soul_misconfigured"), EPAItemCategory::Material, EPAItemRarity::None, 0, 10), 1);
	PlayerA.Inventory->AddItemToSlot(3, MakeItem(FName("item_boss_horn"), EPAItemCategory::Material, EPAItemRarity::None, 0, 50), 8);
	PlayerA.Inventory->AddItemToSlot(4, MakeItem(FName("void_ore"), EPAItemCategory::Material, EPAItemRarity::None, 0, 999), 10);

	// Misconfigured (non-Legendary) recipe output is refused; nothing consumed.
	PlayerA.Router->Server_ForgeBossSoul(0, Forge, PlayerA.Inventory, PlayerA.Wallet, FName("item_boss_soul_misconfigured"), FName("item_boss_horn"), FName("void_ore"));
	TestEqual(TEXT("Non-Legendary recipe output refused: gold untouched"), PlayerA.Wallet->GetGold(), 20000LL);
	TestEqual(TEXT("Non-Legendary recipe output refused: soul kept"), PlayerA.Inventory->GetItemCount(FName("item_boss_soul_misconfigured")), 1);
	TestEqual(TEXT("Non-Legendary recipe output refused: no output item"), PlayerA.Inventory->GetItemCount(FName("item_boss_soul_cheap_output")), 0);

	// Unknown soul id (no server recipe) refused.
	PlayerA.Router->Server_ForgeBossSoul(0, Forge, PlayerA.Inventory, PlayerA.Wallet, FName("item_boss_soul_dragon"), FName("item_boss_horn"), FName("void_ore"));
	TestEqual(TEXT("No recipe: gold untouched"), PlayerA.Wallet->GetGold(), 20000LL);
	TestEqual(TEXT("No recipe: parts kept"), PlayerA.Inventory->GetItemCount(FName("item_boss_horn")), 8);

	// Valid request: output is exactly the server recipe's item.
	PlayerA.Router->Server_ForgeBossSoul(0, Forge, PlayerA.Inventory, PlayerA.Wallet, FName("item_boss_soul_golem"), FName("item_boss_horn"), FName("void_ore"));
	TestEqual(TEXT("Forged: 5,000 Gold charged"), PlayerA.Wallet->GetGold(), 15000LL);
	TestEqual(TEXT("Forged: soul consumed"), PlayerA.Inventory->GetItemCount(FName("item_boss_soul_golem")), 0);
	TestEqual(TEXT("Forged: 4 parts consumed"), PlayerA.Inventory->GetItemCount(FName("item_boss_horn")), 4);
	TestEqual(TEXT("Forged: 5 void ore consumed"), PlayerA.Inventory->GetItemCount(FName("void_ore")), 5);
	TestEqual(TEXT("Forged: output is the server recipe item"), PlayerA.Inventory->GetItemCount(FName("item_boss_soul_golem_greatsword")), 1);
	const FPAInventoryItemEntry* Output = PlayerA.Inventory->GetItemAtSlot(0);
	TestTrue(TEXT("Forged: output is Legendary"), Output && Output->StaticData && Output->StaticData->RarityTier == EPAItemRarity::Legendary);

	Forge->Destroy();
	DestroyPlayer(PlayerA);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
