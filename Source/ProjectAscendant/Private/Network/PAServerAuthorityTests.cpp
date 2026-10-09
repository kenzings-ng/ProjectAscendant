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
#include <type_traits>

#if WITH_DEV_AUTOMATION_TESTS

/**
 * X11a server-authority regression tests (DECISIONS §11, ADR-0003, control-manifest "Dedicated Server Authority").
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
static_assert(std::is_same_v<decltype(&UPABlacksmithComponent::Server_RepairItem), void (UPABlacksmithComponent::*)(const FGuid&)>,
	"X11a: Server_RepairItem must not accept a client cost");
static_assert(std::is_same_v<decltype(&UPABlacksmithComponent::Server_ReforgeAffix), void (UPABlacksmithComponent::*)(const FGuid&, int32)>,
	"X11a: Server_ReforgeAffix must not accept client cost/shards");
static_assert(std::is_same_v<decltype(&UPABlacksmithComponent::Server_AddSocket), void (UPABlacksmithComponent::*)(const FGuid&)>,
	"X11a: Server_AddSocket must not accept client cost/shards/forge tier");
static_assert(std::is_same_v<decltype(&UPABlacksmithComponent::Server_RequestForgeBossSoul),
	void (UPABlacksmithComponent::*)(UPAInventoryComponent*, UPACurrencyComponent*, FName, FName, FName)>,
	"X11a: Server_RequestForgeBossSoul must not accept a client-chosen output item");

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
	};

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
	AActor* Forge = SpawnLocatedActor(World, AActor::StaticClass(), FVector(100.0f, 0.0f, 0.0f), PlayerA.PC);
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

	Blacksmith->BindSavedItemInventory(&SavedItems, PlayerA.Wallet);
	Blacksmith->Server_RepairItem(DamagedItem.ItemInstanceUID);
	TestEqual(TEXT("Repair RPC restored durability"), SavedItems[0].CurrentDurability, 100.0f);
	TestEqual(TEXT("Repair RPC charged exactly the server formula cost"), PlayerA.Wallet->GetGold(), 10000LL - FormulaCost);

	// --- AddSocket: forge tier comes from the server-side forge, cost from the socket index ---
	FPASavedItemInstance SocketItem = ItemGenerator->GenerateItemInstance(FName("ShadowDagger"), 25, EPAItemRarity::Rare, EPAForgeTier::Tier2_Field);
	SavedItems.Add(SocketItem);
	const int64 GoldBeforeSocket = PlayerA.Wallet->GetGold();
	const int64 ShardsBeforeSocket = PlayerA.Wallet->GetSkillShards();

	Blacksmith->SetForgeTier(EPABlacksmithTier::Tier1_Outpost);
	Blacksmith->Server_AddSocket(SocketItem.ItemInstanceUID);
	TestFalse(TEXT("Tier 1 forge (server state) refuses socketing"), SavedItems[1].SocketSlots[0].bIsUnlocked);
	TestEqual(TEXT("No gold charged at Tier 1 forge"), PlayerA.Wallet->GetGold(), GoldBeforeSocket);

	Blacksmith->SetForgeTier(EPABlacksmithTier::Tier2_Wilderness);
	Blacksmith->Server_AddSocket(SocketItem.ItemInstanceUID);
	TestTrue(TEXT("Tier 2 forge (server state) unlocks socket 0"), SavedItems[1].SocketSlots[0].bIsUnlocked);
	TestEqual(TEXT("Socket 0 charged 1,000 Gold (itemization.md 7.2)"), PlayerA.Wallet->GetGold(), GoldBeforeSocket - 1000LL);
	TestEqual(TEXT("Socket 0 charged 3 Shards (itemization.md 7.2)"), PlayerA.Wallet->GetSkillShards(), ShardsBeforeSocket - 3LL);

	// --- Reforge cost constant is the GDD value (no client override exists) ---
	int32 ReforgeGold = 0;
	int32 ReforgeShards = 0;
	FPABlacksmithFormulas::GetReforgeAffixCost(ReforgeGold, ReforgeShards);
	TestEqual(TEXT("Reforge gold cost is 2,000"), ReforgeGold, 2000);
	TestEqual(TEXT("Reforge shard cost is 5"), ReforgeShards, 5);

	Blacksmith->BindSavedItemInventory(nullptr, nullptr);
	Forge->Destroy();
	DestroyPlayer(PlayerA);
	return true;
}

// =============================================================================
// 1b. Server_RequestUnlockSocket charges the server socket cost (no free-socket bypass)
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
	AActor* Forge = SpawnLocatedActor(World, AActor::StaticClass(), FVector(100.0f, 0.0f, 0.0f), PlayerA.PC);
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
	Blacksmith->Server_RequestUnlockSocket(PlayerA.Inventory, PlayerA.Wallet, 0);
	TestEqual(TEXT("Insufficient gold: no socket opened"), SocketCount(), 0);
	TestEqual(TEXT("Insufficient gold: gold untouched"), PlayerA.Wallet->GetGold(), 999LL);
	TestEqual(TEXT("Insufficient gold: shards untouched"), PlayerA.Wallet->GetSkillShards(), 3LL);

	// --- Insufficient shards (2 < 3): rejected, nothing deducted, no socket ---
	PlayerA.Wallet->AddCurrency(EPACurrencyType::Gold, 1, CurrErr);       // 1,000 Gold
	PlayerA.Wallet->DeductCurrency(EPACurrencyType::SkillShards, 1, CurrErr); // 2 Shards
	Blacksmith->Server_RequestUnlockSocket(PlayerA.Inventory, PlayerA.Wallet, 0);
	TestEqual(TEXT("Insufficient shards: no socket opened"), SocketCount(), 0);
	TestEqual(TEXT("Insufficient shards: gold untouched"), PlayerA.Wallet->GetGold(), 1000LL);
	TestEqual(TEXT("Insufficient shards: shards untouched"), PlayerA.Wallet->GetSkillShards(), 2LL);

	// --- Sufficient: socket 1 costs 1,000 Gold + 3 Shards ---
	PlayerA.Wallet->AddCurrency(EPACurrencyType::SkillShards, 1, CurrErr); // 3 Shards
	Blacksmith->Server_RequestUnlockSocket(PlayerA.Inventory, PlayerA.Wallet, 0);
	TestEqual(TEXT("Socket 1 opened"), SocketCount(), 1);
	TestEqual(TEXT("Socket 1 charged 1,000 Gold"), PlayerA.Wallet->GetGold(), 0LL);
	TestEqual(TEXT("Socket 1 charged 3 Shards"), PlayerA.Wallet->GetSkillShards(), 0LL);

	// --- Socket 2 costs 3,000 Gold + 8 Shards ---
	PlayerA.Wallet->AddCurrency(EPACurrencyType::Gold, 3500, CurrErr);
	PlayerA.Wallet->AddCurrency(EPACurrencyType::SkillShards, 10, CurrErr);
	Blacksmith->Server_RequestUnlockSocket(PlayerA.Inventory, PlayerA.Wallet, 0);
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
	AActor* Forge = SpawnLocatedActor(World, AActor::StaticClass(), FVector(100.0f, 0.0f, 0.0f), PlayerA.PC);
	AActor* Shop = SpawnLocatedActor(World, AActor::StaticClass(), FVector(-100.0f, 0.0f, 0.0f), PlayerA.PC);
	UPABlacksmithComponent* Blacksmith = NewObject<UPABlacksmithComponent>(Forge, TEXT("TestForge"));
	UPAMerchantComponent* Merchant = NewObject<UPAMerchantComponent>(Shop, TEXT("TestMerchant"));
	if (!TestNotNull(TEXT("Player A"), PlayerA.Pawn) || !TestNotNull(TEXT("Player B"), PlayerB.Pawn))
	{
		DestroyPlayer(PlayerA);
		DestroyPlayer(PlayerB);
		return false;
	}

	TestTrue(TEXT("Requesting player of A-owned forge resolves to PC A"), Blacksmith->GetRequestingPlayerController() == PlayerA.PC);
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

	Blacksmith->Server_RequestRepair(PlayerB.Inventory, PlayerA.Wallet, 0);
	TestEqual(TEXT("Repair RPC with B's inventory: B's item untouched"), PlayerB.Inventory->GetItemAtSlot(0)->DynamicData.CurrentDurability, 20.0f);
	TestEqual(TEXT("Repair RPC with B's inventory: A's gold untouched"), PlayerA.Wallet->GetGold(), 1000LL);

	Blacksmith->Server_RequestRepair(PlayerA.Inventory, PlayerB.Wallet, 0);
	TestEqual(TEXT("Repair RPC with B's wallet: A's item untouched"), PlayerA.Inventory->GetItemAtSlot(0)->DynamicData.CurrentDurability, 20.0f);
	TestEqual(TEXT("Repair RPC with B's wallet: B's gold untouched"), PlayerB.Wallet->GetGold(), 1000LL);

	// Positive control: own components succeed (repair cost = ceil(300 * 0.25 * 0.8) = 60)
	Blacksmith->Server_RequestRepair(PlayerA.Inventory, PlayerA.Wallet, 0);
	TestEqual(TEXT("Repair RPC with own components: repaired"), PlayerA.Inventory->GetItemAtSlot(0)->DynamicData.CurrentDurability, 100.0f);
	TestEqual(TEXT("Repair RPC with own components: charged 60 Gold"), PlayerA.Wallet->GetGold(), 940LL);

	// --- Merchant: selling someone else's item into someone else's wallet ---
	EPATransactionError TransErr = EPATransactionError::None;
	TestFalse(TEXT("Merchant validate: foreign inventory rejected"), Merchant->ValidateServerRequest(PlayerA.PC, PlayerB.Inventory, PlayerA.Wallet, TransErr));
	TestEqual(TEXT("Merchant validate: foreign inventory -> ServerRejected"), TransErr, EPATransactionError::ServerRejected);

	Merchant->Server_RequestSellItem(PlayerB.Inventory, PlayerA.Wallet, 0, 1);
	TestNotNull(TEXT("Sell RPC with B's inventory: B's item still present"), PlayerB.Inventory->GetItemAtSlot(0));
	TestEqual(TEXT("Sell RPC with B's inventory: A's gold unchanged"), PlayerA.Wallet->GetGold(), 940LL);
	TestEqual(TEXT("Sell RPC with B's inventory: no buyback entry"), Merchant->GetBuybackCount(), 0);

	// Positive control: selling own item works (floor(300 * 0.30) = 90)
	Merchant->Server_RequestSellItem(PlayerA.Inventory, PlayerA.Wallet, 0, 1);
	TestNull(TEXT("Sell RPC with own components: item removed"), PlayerA.Inventory->GetItemAtSlot(0));
	TestEqual(TEXT("Sell RPC with own components: received 90 Gold"), PlayerA.Wallet->GetGold(), 1030LL);

	// --- NPC-owned forge (no player owner): every RPC is rejected server-side ---
	AActor* NpcForge = SpawnLocatedActor(World, AActor::StaticClass(), FVector(10.0f, 0.0f, 0.0f), nullptr);
	UPABlacksmithComponent* NpcBlacksmith = NewObject<UPABlacksmithComponent>(NpcForge, TEXT("NpcForge"));
	TestNull(TEXT("NPC forge has no requesting player"), NpcBlacksmith->GetRequestingPlayerController());
	NpcBlacksmith->Server_RequestRepair(PlayerB.Inventory, PlayerB.Wallet, 0);
	TestEqual(TEXT("NPC forge RPC rejected: B's item untouched"), PlayerB.Inventory->GetItemAtSlot(0)->DynamicData.CurrentDurability, 20.0f);
	TestEqual(TEXT("NPC forge RPC rejected: B's gold untouched"), PlayerB.Wallet->GetGold(), 1000LL);

	NpcForge->Destroy();
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
	AActor* Forge = SpawnLocatedActor(World, AActor::StaticClass(), FVector(0.0f, 0.0f, 0.0f), PlayerA.PC);
	AActor* Shop = SpawnLocatedActor(World, AActor::StaticClass(), FVector(0.0f, 0.0f, 0.0f), PlayerA.PC);
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

	Blacksmith->Server_RequestRepair(PlayerA.Inventory, PlayerA.Wallet, 0);
	TestEqual(TEXT("Out-of-range repair RPC: item untouched"), PlayerA.Inventory->GetItemAtSlot(0)->DynamicData.CurrentDurability, 20.0f);
	TestEqual(TEXT("Out-of-range repair RPC: gold untouched"), PlayerA.Wallet->GetGold(), 1000LL);

	Merchant->Server_RequestBuyItem(PlayerA.Inventory, PlayerA.Wallet, 0, 1);
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

	Blacksmith->Server_RequestRepair(PlayerA.Inventory, PlayerA.Wallet, 0);
	TestEqual(TEXT("In-combat repair RPC: item untouched"), PlayerA.Inventory->GetItemAtSlot(0)->DynamicData.CurrentDurability, 20.0f);
	Merchant->Server_RequestBuyItem(PlayerA.Inventory, PlayerA.Wallet, 0, 1);
	TestEqual(TEXT("In-combat buy RPC: no potion added"), PlayerA.Inventory->GetItemCount(FName("item_test_potion")), 0);

	// --- Out of combat, in range: both RPCs go through ---
	ASC->RemoveLooseGameplayTag(InCombatTag);
	Blacksmith->Server_RequestRepair(PlayerA.Inventory, PlayerA.Wallet, 0);
	TestEqual(TEXT("In-range repair RPC: repaired"), PlayerA.Inventory->GetItemAtSlot(0)->DynamicData.CurrentDurability, 100.0f);
	TestEqual(TEXT("In-range repair RPC: charged 60 Gold"), PlayerA.Wallet->GetGold(), 940LL);
	Merchant->Server_RequestBuyItem(PlayerA.Inventory, PlayerA.Wallet, 0, 1);
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
	AActor* Forge = SpawnLocatedActor(World, AActor::StaticClass(), FVector(100.0f, 0.0f, 0.0f), PlayerA.PC);
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
	Blacksmith->Server_RequestForgeBossSoul(PlayerA.Inventory, PlayerA.Wallet, FName("item_boss_soul_misconfigured"), FName("item_boss_horn"), FName("void_ore"));
	TestEqual(TEXT("Non-Legendary recipe output refused: gold untouched"), PlayerA.Wallet->GetGold(), 20000LL);
	TestEqual(TEXT("Non-Legendary recipe output refused: soul kept"), PlayerA.Inventory->GetItemCount(FName("item_boss_soul_misconfigured")), 1);
	TestEqual(TEXT("Non-Legendary recipe output refused: no output item"), PlayerA.Inventory->GetItemCount(FName("item_boss_soul_cheap_output")), 0);

	// Unknown soul id (no server recipe) refused.
	Blacksmith->Server_RequestForgeBossSoul(PlayerA.Inventory, PlayerA.Wallet, FName("item_boss_soul_dragon"), FName("item_boss_horn"), FName("void_ore"));
	TestEqual(TEXT("No recipe: gold untouched"), PlayerA.Wallet->GetGold(), 20000LL);
	TestEqual(TEXT("No recipe: parts kept"), PlayerA.Inventory->GetItemCount(FName("item_boss_horn")), 8);

	// Valid request: output is exactly the server recipe's item.
	Blacksmith->Server_RequestForgeBossSoul(PlayerA.Inventory, PlayerA.Wallet, FName("item_boss_soul_golem"), FName("item_boss_horn"), FName("void_ore"));
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
