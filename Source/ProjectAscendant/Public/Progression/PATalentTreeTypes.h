// Copyright Project Ascendant. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/PACharacterSelectTypes.h"
#include "PATalentTreeTypes.generated.h"

/**
 * EPATalentTreeError
 *
 * Mã lỗi khi thực hiện mở khóa node hoặc tẩy điểm trên Cây Kỹ Năng (Skill Tree).
 */
UENUM(BlueprintType)
enum class EPATalentTreeError : uint8
{
	None                    = 0 UMETA(DisplayName = "None"),
	InvalidNodeId           = 1 UMETA(DisplayName = "Invalid Talent Node ID"),
	NodeAlreadyUnlocked     = 2 UMETA(DisplayName = "Talent Node Already Unlocked"),
	PrerequisiteNotMet      = 3 UMETA(DisplayName = "Prerequisite Node Not Unlocked"),
	InsufficientLevel       = 4 UMETA(DisplayName = "Character Level Too Low"),
	InsufficientSkillPoints = 5 UMETA(DisplayName = "Insufficient Skill Points"),
	NoNodesToReset          = 6 UMETA(DisplayName = "No Unlocked Nodes To Reset"),
	NotAuthoritative        = 7 UMETA(DisplayName = "Server Authority Required"),
	ServerRejected          = 8 UMETA(DisplayName = "Server Rejected Request")
};

/**
 * FPATalentNode
 *
 * Định nghĩa một node tài năng nội tại (Passive Talent Node) trên Cây Kỹ Năng.
 */
USTRUCT(BlueprintType)
struct PROJECTASCENDANT_API FPATalentNode
{
	GENERATED_BODY()

	/** Mã định danh duy nhất của Node (ví dụ: Vanguard_Ironclad_1) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Talent")
	FName NodeId;

	/** Tên hiển thị của Node */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Talent")
	FText NodeName;

	/** Nhánh kỹ năng (0 = Nhánh 1, 1 = Nhánh 2, 2 = Nhánh 3) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Talent")
	int32 BranchIndex = 0;

	/** Bậc của node trong nhánh (1 = Cơ bản, 2 = Nâng cao, 3 = Đỉnh phong) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Talent")
	int32 Tier = 1;

	/** Yêu cầu cấp độ nhân vật tối thiểu để mở khóa */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Talent")
	int32 RequiredLevel = 1;

	/** Node tiên quyết bắt buộc phải mở khóa trước (NAME_None nếu là Tier 1) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Talent")
	FName PrerequisiteNodeId = NAME_None;

	/** Chi phí Điểm Kỹ Năng tiêu hao (mặc định 1) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Talent")
	int32 SkillPointCost = 1;

	// --- Passive Stat Modifiers (AC-2) ---

	/** Tăng sức mạnh tấn công (AttackPower) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Talent|Modifiers")
	float BonusAttackPower = 0.0f;

	/** Tăng thanh thế đứng tối đa (MaxPosture) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Talent|Modifiers")
	float BonusMaxPosture = 0.0f;

	/** Giảm thời gian hồi chiêu Dash (tính bằng giây) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Talent|Modifiers")
	float DashCooldownReduction = 0.0f;

	/** Tăng giáp phòng ngự (Armor) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Talent|Modifiers")
	float BonusArmor = 0.0f;

	/** Tăng sinh mệnh tối đa (MaxHealth) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Talent|Modifiers")
	float BonusMaxHealth = 0.0f;
};

/**
 * FPATalentTreeDatabase
 *
 * Cơ sở dữ liệu tĩnh định nghĩa cấu trúc 3 nhánh chuyên môn hóa cho từng Chức nghiệp (AC-1).
 */
struct PROJECTASCENDANT_API FPATalentTreeDatabase
{
	static const TArray<FPATalentNode>& GetTalentTreeForClass(EPACharacterClass CharacterClass)
	{
		static const TArray<FPATalentNode> VanguardTree = BuildVanguardTree();
		static const TArray<FPATalentNode> RangerTree = BuildRangerTree();
		static const TArray<FPATalentNode> ArcanistTree = BuildArcanistTree();
		static const TArray<FPATalentNode> EmptyTree;

		switch (CharacterClass)
		{
		case EPACharacterClass::Vanguard:
			return VanguardTree;
		case EPACharacterClass::Ranger:
			return RangerTree;
		case EPACharacterClass::Arcanist:
			return ArcanistTree;
		default:
			return EmptyTree;
		}
	}

private:
	static TArray<FPATalentNode> BuildVanguardTree()
	{
		TArray<FPATalentNode> Nodes;
		// --- Branch 0: Ironclad (Phòng Ngự / Thế Đứng) ---
			{
				FPATalentNode N1;
				N1.NodeId = FName("Vanguard_Ironclad_1");
				N1.NodeName = FText::FromString(TEXT("Thế Thủ Thép"));
				N1.BranchIndex = 0;
				N1.Tier = 1;
				N1.RequiredLevel = 1;
				N1.BonusMaxPosture = 10.0f;
				N1.BonusArmor = 5.0f;
				Nodes.Add(N1);

				FPATalentNode N2;
				N2.NodeId = FName("Vanguard_Ironclad_2");
				N2.NodeName = FText::FromString(TEXT("Bất Động Như Sơn"));
				N2.BranchIndex = 0;
				N2.Tier = 2;
				N2.RequiredLevel = 5;
				N2.PrerequisiteNodeId = FName("Vanguard_Ironclad_1");
				N2.BonusMaxPosture = 20.0f;
				N2.BonusMaxHealth = 50.0f;
				Nodes.Add(N2);

				FPATalentNode N3;
				N3.NodeId = FName("Vanguard_Ironclad_3");
				N3.NodeName = FText::FromString(TEXT("Chiến Binh Kim Cương"));
				N3.BranchIndex = 0;
				N3.Tier = 3;
				N3.RequiredLevel = 10;
				N3.PrerequisiteNodeId = FName("Vanguard_Ironclad_2");
				N3.BonusMaxPosture = 30.0f;
				N3.BonusArmor = 15.0f;
				Nodes.Add(N3);
			}
			// --- Branch 1: Duelist (Phản Đòn / Sát Thương) ---
			{
				FPATalentNode N1;
				N1.NodeId = FName("Vanguard_Duelist_1");
				N1.NodeName = FText::FromString(TEXT("Lưỡi Gươm Nhạy Bén"));
				N1.BranchIndex = 1;
				N1.Tier = 1;
				N1.RequiredLevel = 1;
				N1.BonusAttackPower = 5.0f;
				Nodes.Add(N1);

				FPATalentNode N2;
				N2.NodeId = FName("Vanguard_Duelist_2");
				N2.NodeName = FText::FromString(TEXT("Phản Đòn Sấm Sét"));
				N2.BranchIndex = 1;
				N2.Tier = 2;
				N2.RequiredLevel = 5;
				N2.PrerequisiteNodeId = FName("Vanguard_Duelist_1");
				N2.BonusAttackPower = 10.0f;
				N2.BonusMaxPosture = 15.0f;
				Nodes.Add(N2);

				FPATalentNode N3;
				N3.NodeId = FName("Vanguard_Duelist_3");
				N3.NodeName = FText::FromString(TEXT("Khai Môn Trảm"));
				N3.BranchIndex = 1;
				N3.Tier = 3;
				N3.RequiredLevel = 10;
				N3.PrerequisiteNodeId = FName("Vanguard_Duelist_2");
				N3.BonusAttackPower = 20.0f;
				N3.BonusMaxPosture = 25.0f;
				Nodes.Add(N3);
			}
			// --- Branch 2: Juggernaut (Công Kích / Giảm Hồi Chiêu Dash) ---
			{
				FPATalentNode N1;
				N1.NodeId = FName("Vanguard_Juggernaut_1");
				N1.NodeName = FText::FromString(TEXT("Xung Kích"));
				N1.BranchIndex = 2;
				N1.Tier = 1;
				N1.RequiredLevel = 1;
				N1.DashCooldownReduction = 0.05f;
				N1.BonusAttackPower = 5.0f;
				Nodes.Add(N1);

				FPATalentNode N2;
				N2.NodeId = FName("Vanguard_Juggernaut_2");
				N2.NodeName = FText::FromString(TEXT("Càn Quét Tật Phong"));
				N2.BranchIndex = 2;
				N2.Tier = 2;
				N2.RequiredLevel = 5;
				N2.PrerequisiteNodeId = FName("Vanguard_Juggernaut_1");
				N2.DashCooldownReduction = 0.10f;
				N2.BonusAttackPower = 10.0f;
				Nodes.Add(N2);

				FPATalentNode N3;
				N3.NodeId = FName("Vanguard_Juggernaut_3");
				N3.NodeName = FText::FromString(TEXT("Chiến Thần Giáng Lâm"));
				N3.BranchIndex = 2;
				N3.Tier = 3;
				N3.RequiredLevel = 10;
				N3.PrerequisiteNodeId = FName("Vanguard_Juggernaut_2");
				N3.DashCooldownReduction = 0.15f;
				N3.BonusAttackPower = 15.0f;
				N3.BonusArmor = 10.0f;
				Nodes.Add(N3);
			}
		return Nodes;
	}

	static TArray<FPATalentNode> BuildRangerTree()
	{
		TArray<FPATalentNode> Nodes;
		// --- Branch 0: Marksman (Sát Thương Tầm Xa) ---
		{
			FPATalentNode N1;
			N1.NodeId = FName("Ranger_Marksman_1");
			N1.NodeName = FText::FromString(TEXT("Tâm Nhãn"));
			N1.BranchIndex = 0;
			N1.Tier = 1;
			N1.RequiredLevel = 1;
			N1.BonusAttackPower = 6.0f;
			Nodes.Add(N1);

			FPATalentNode N2;
			N2.NodeId = FName("Ranger_Marksman_2");
			N2.NodeName = FText::FromString(TEXT("Xuyên Giáp Tiễn"));
			N2.BranchIndex = 0;
			N2.Tier = 2;
			N2.RequiredLevel = 5;
			N2.PrerequisiteNodeId = FName("Ranger_Marksman_1");
			N2.BonusAttackPower = 12.0f;
			N2.BonusMaxPosture = 10.0f;
			Nodes.Add(N2);

			FPATalentNode N3;
			N3.NodeId = FName("Ranger_Marksman_3");
			N3.NodeName = FText::FromString(TEXT("Bách Bộ Xuyên Dương"));
			N3.BranchIndex = 0;
			N3.Tier = 3;
			N3.RequiredLevel = 10;
			N3.PrerequisiteNodeId = FName("Ranger_Marksman_2");
			N3.BonusAttackPower = 25.0f;
			Nodes.Add(N3);
		}
		// --- Branch 1: Windrunner (Cơ Động / Giảm Hồi Chiêu Dash) ---
		{
			FPATalentNode N1;
			N1.NodeId = FName("Ranger_Windrunner_1");
			N1.NodeName = FText::FromString(TEXT("Bộ Pháp Tật Phong"));
			N1.BranchIndex = 1;
			N1.Tier = 1;
			N1.RequiredLevel = 1;
			N1.DashCooldownReduction = 0.05f;
			Nodes.Add(N1);

			FPATalentNode N2;
			N2.NodeId = FName("Ranger_Windrunner_2");
			N2.NodeName = FText::FromString(TEXT("Hư Ảnh Lướt"));
			N2.BranchIndex = 1;
			N2.Tier = 2;
			N2.RequiredLevel = 5;
			N2.PrerequisiteNodeId = FName("Ranger_Windrunner_1");
			N2.DashCooldownReduction = 0.10f;
			N2.BonusAttackPower = 5.0f;
			Nodes.Add(N2);

			FPATalentNode N3;
			N3.NodeId = FName("Ranger_Windrunner_3");
			N3.NodeName = FText::FromString(TEXT("Cơn Lốc Tử Thần"));
			N3.BranchIndex = 1;
			N3.Tier = 3;
			N3.RequiredLevel = 10;
			N3.PrerequisiteNodeId = FName("Ranger_Windrunner_2");
			N3.DashCooldownReduction = 0.15f;
			N3.BonusAttackPower = 15.0f;
			Nodes.Add(N3);
		}
		// --- Branch 2: Trapper (Bẫy / Posture) ---
		{
			FPATalentNode N1;
			N1.NodeId = FName("Ranger_Trapper_1");
			N1.NodeName = FText::FromString(TEXT("Cương Tác Bẫy"));
			N1.BranchIndex = 2;
			N1.Tier = 1;
			N1.RequiredLevel = 1;
			N1.BonusMaxPosture = 10.0f;
			Nodes.Add(N1);

			FPATalentNode N2;
			N2.NodeId = FName("Ranger_Trapper_2");
			N2.NodeName = FText::FromString(TEXT("Bẫy Gai Độc"));
			N2.BranchIndex = 2;
			N2.Tier = 2;
			N2.RequiredLevel = 5;
			N2.PrerequisiteNodeId = FName("Ranger_Trapper_1");
			N2.BonusMaxPosture = 20.0f;
			N2.BonusArmor = 5.0f;
			Nodes.Add(N2);

			FPATalentNode N3;
			N3.NodeId = FName("Ranger_Trapper_3");
			N3.NodeName = FText::FromString(TEXT("Tử Vực Săn Bắt"));
			N3.BranchIndex = 2;
			N3.Tier = 3;
			N3.RequiredLevel = 10;
			N3.PrerequisiteNodeId = FName("Ranger_Trapper_2");
			N3.BonusMaxPosture = 30.0f;
			N3.BonusAttackPower = 10.0f;
			Nodes.Add(N3);
		}
		return Nodes;
	}

	static TArray<FPATalentNode> BuildArcanistTree()
	{
		TArray<FPATalentNode> Nodes;
		// --- Branch 0: Pyromancer (Sát Thương Ma Pháp) ---
		{
			FPATalentNode N1;
			N1.NodeId = FName("Arcanist_Pyromancer_1");
			N1.NodeName = FText::FromString(TEXT("Hỏa Cầu Thuật"));
			N1.BranchIndex = 0;
			N1.Tier = 1;
			N1.RequiredLevel = 1;
			N1.BonusAttackPower = 8.0f;
			Nodes.Add(N1);

			FPATalentNode N2;
			N2.NodeId = FName("Arcanist_Pyromancer_2");
			N2.NodeName = FText::FromString(TEXT("Cháy Bùng Nổ"));
			N2.BranchIndex = 0;
			N2.Tier = 2;
			N2.RequiredLevel = 5;
			N2.PrerequisiteNodeId = FName("Arcanist_Pyromancer_1");
			N2.BonusAttackPower = 15.0f;
			N2.BonusMaxPosture = 10.0f;
			Nodes.Add(N2);

			FPATalentNode N3;
			N3.NodeId = FName("Arcanist_Pyromancer_3");
			N3.NodeName = FText::FromString(TEXT("Địa Ngục Liệt Hỏa"));
			N3.BranchIndex = 0;
			N3.Tier = 3;
			N3.RequiredLevel = 10;
			N3.PrerequisiteNodeId = FName("Arcanist_Pyromancer_2");
			N3.BonusAttackPower = 30.0f;
			Nodes.Add(N3);
		}
		// --- Branch 1: Chronomancer (Thời Gian / Giảm Hồi Chiêu Dash) ---
		{
			FPATalentNode N1;
			N1.NodeId = FName("Arcanist_Chronomancer_1");
			N1.NodeName = FText::FromString(TEXT("Ngưng Đọng Thời Không"));
			N1.BranchIndex = 1;
			N1.Tier = 1;
			N1.RequiredLevel = 1;
			N1.DashCooldownReduction = 0.05f;
			Nodes.Add(N1);

			FPATalentNode N2;
			N2.NodeId = FName("Arcanist_Chronomancer_2");
			N2.NodeName = FText::FromString(TEXT("Thiểm Ảnh Dịch Chuyển"));
			N2.BranchIndex = 1;
			N2.Tier = 2;
			N2.RequiredLevel = 5;
			N2.PrerequisiteNodeId = FName("Arcanist_Chronomancer_1");
			N2.DashCooldownReduction = 0.10f;
			N2.BonusAttackPower = 5.0f;
			Nodes.Add(N2);

			FPATalentNode N3;
			N3.NodeId = FName("Arcanist_Chronomancer_3");
			N3.NodeName = FText::FromString(TEXT("Dòng Thời Gian Nghịch"));
			N3.BranchIndex = 1;
			N3.Tier = 3;
			N3.RequiredLevel = 10;
			N3.PrerequisiteNodeId = FName("Arcanist_Chronomancer_2");
			N3.DashCooldownReduction = 0.15f;
			N3.BonusAttackPower = 15.0f;
			Nodes.Add(N3);
		}
		// --- Branch 2: Leyline (Địa Mạch / Thế Đứng & Phòng Hộ) ---
		{
			FPATalentNode N1;
			N1.NodeId = FName("Arcanist_Leyline_1");
			N1.NodeName = FText::FromString(TEXT("Hộ Thể Ma Đạo"));
			N1.BranchIndex = 2;
			N1.Tier = 1;
			N1.RequiredLevel = 1;
			N1.BonusMaxPosture = 10.0f;
			N1.BonusArmor = 5.0f;
			Nodes.Add(N1);

			FPATalentNode N2;
			N2.NodeId = FName("Arcanist_Leyline_2");
			N2.NodeName = FText::FromString(TEXT("Cộng Hưởng Địa Mạch"));
			N2.BranchIndex = 2;
			N2.Tier = 2;
			N2.RequiredLevel = 5;
			N2.PrerequisiteNodeId = FName("Arcanist_Leyline_1");
			N2.BonusMaxPosture = 20.0f;
			N2.BonusMaxHealth = 50.0f;
			Nodes.Add(N2);

			FPATalentNode N3;
			N3.NodeId = FName("Arcanist_Leyline_3");
			N3.NodeName = FText::FromString(TEXT("Bất Hoại Ma Giáp"));
			N3.BranchIndex = 2;
			N3.Tier = 3;
			N3.RequiredLevel = 10;
			N3.PrerequisiteNodeId = FName("Arcanist_Leyline_2");
			N3.BonusMaxPosture = 30.0f;
			N3.BonusArmor = 15.0f;
			Nodes.Add(N3);
		}
		return Nodes;
	}

public:
	static const FPATalentNode* FindNode(EPACharacterClass CharacterClass, FName NodeId)
	{
		const TArray<FPATalentNode>& Tree = GetTalentTreeForClass(CharacterClass);
		for (const FPATalentNode& Node : Tree)
		{
			if (Node.NodeId == NodeId)
			{
				return &Node;
			}
		}
		return nullptr;
	}
};
