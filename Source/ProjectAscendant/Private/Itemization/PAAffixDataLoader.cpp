// Copyright Project Ascendant. All Rights Reserved.

#include "Itemization/PAAffixDataLoader.h"

UDataTable* UPAAffixDataLoader::CreateAffixDefinitionsTable(UObject* Outer)
{
	UDataTable* DataTable = NewObject<UDataTable>(Outer ? Outer : GetTransientPackage(), FName("DT_AffixDefinitions"));
	DataTable->RowStruct = FPAAffixDefinitionRow::StaticStruct();
	PopulateAffixDefinitionsTable(DataTable);
	return DataTable;
}

void UPAAffixDataLoader::PopulateAffixDefinitionsTable(UDataTable* InDataTable)
{
	if (!InDataTable)
	{
		return;
	}

	InDataTable->RowStruct = FPAAffixDefinitionRow::StaticStruct();
	InDataTable->EmptyTable();

	const TArray<FPAAffixDefinitionRow> Rows = GetCanonicalAffixDefinitions();
	for (const FPAAffixDefinitionRow& Row : Rows)
	{
		InDataTable->AddRow(Row.AffixName, Row);
	}
}

TArray<FPAAffixDefinitionRow> UPAAffixDataLoader::GetCanonicalAffixDefinitions()
{
	TArray<FPAAffixDefinitionRow> Rows;
	Rows.Reserve(28);

	// =========================================================================
	// 14 TIỀN TỐ (PREFIXES)
	// =========================================================================

	// --- BẬC 1: TIỀN TRẠM (OUTPOST FORGE BRACKET - ILVL 1-20) --- 8 Tiền Tố
	{
		FPAAffixDefinitionRow Row;
		Row.AffixName = FName("prefix_phys_flat");
		Row.DisplayName = FText::FromString(TEXT("Tiền Tố: Sát Thương Sắt Thép (Flat Phys Damage)"));
		Row.AffixType = EPAAffixType::Prefix;
		Row.MinForgeTier = EPAForgeTier::Tier1_Outpost;
		Row.TargetStat = FName("PhysicalAttack");
		Row.Tier1Range = FVector2D(3.0f, 6.0f);
		Row.Tier2Range = FVector2D(8.0f, 14.0f);
		Row.Tier3Range = FVector2D(18.0f, 26.0f);
		Row.Tier4Range = FVector2D(32.0f, 45.0f);
		Rows.Add(Row);
	}
	{
		FPAAffixDefinitionRow Row;
		Row.AffixName = FName("prefix_phys_pct");
		Row.DisplayName = FText::FromString(TEXT("Tiền Tố: Tàn Bạo Tăng Cường (% Phys Damage)"));
		Row.AffixType = EPAAffixType::Prefix;
		Row.MinForgeTier = EPAForgeTier::Tier1_Outpost;
		Row.TargetStat = FName("PhysicalDamagePct");
		Row.Tier1Range = FVector2D(4.0f, 7.0f);
		Row.Tier2Range = FVector2D(8.0f, 13.0f);
		Row.Tier3Range = FVector2D(15.0f, 22.0f);
		Row.Tier4Range = FVector2D(25.0f, 35.0f);
		Rows.Add(Row);
	}
	{
		FPAAffixDefinitionRow Row;
		Row.AffixName = FName("prefix_elemental_fire");
		Row.DisplayName = FText::FromString(TEXT("Tiền Tố: Hỏa Diệm Tôi Luyện (Fire Damage)"));
		Row.AffixType = EPAAffixType::Prefix;
		Row.MinForgeTier = EPAForgeTier::Tier1_Outpost;
		Row.TargetStat = FName("ElementalDamageFire");
		Row.Tier1Range = FVector2D(4.0f, 7.0f);
		Row.Tier2Range = FVector2D(9.0f, 16.0f);
		Row.Tier3Range = FVector2D(20.0f, 30.0f);
		Row.Tier4Range = FVector2D(35.0f, 50.0f);
		Rows.Add(Row);
	}
	{
		FPAAffixDefinitionRow Row;
		Row.AffixName = FName("prefix_elemental_ice");
		Row.DisplayName = FText::FromString(TEXT("Tiền Tố: Sương Băng Giá Lạnh (Ice Damage)"));
		Row.AffixType = EPAAffixType::Prefix;
		Row.MinForgeTier = EPAForgeTier::Tier1_Outpost;
		Row.TargetStat = FName("ElementalDamageIce");
		Row.Tier1Range = FVector2D(3.0f, 6.0f);
		Row.Tier2Range = FVector2D(8.0f, 14.0f);
		Row.Tier3Range = FVector2D(18.0f, 26.0f);
		Row.Tier4Range = FVector2D(30.0f, 44.0f);
		Rows.Add(Row);
	}
	{
		FPAAffixDefinitionRow Row;
		Row.AffixName = FName("prefix_posture_dmg");
		Row.DisplayName = FText::FromString(TEXT("Tiền Tố: Trọng Lực Phá Khớp (% Posture Break Damage)"));
		Row.AffixType = EPAAffixType::Prefix;
		Row.MinForgeTier = EPAForgeTier::Tier1_Outpost;
		Row.TargetStat = FName("PostureDamagePct");
		Row.HardCapPct = 35.0f;
		Row.Tier1Range = FVector2D(5.0f, 8.0f);
		Row.Tier2Range = FVector2D(10.0f, 16.0f);
		Row.Tier3Range = FVector2D(18.0f, 25.0f);
		Row.Tier4Range = FVector2D(28.0f, 35.0f);
		Rows.Add(Row);
	}
	{
		FPAAffixDefinitionRow Row;
		Row.AffixName = FName("prefix_armor_flat");
		Row.DisplayName = FText::FromString(TEXT("Tiền Tố: Vảy Thép Kiên Cố (Flat Armor)"));
		Row.AffixType = EPAAffixType::Prefix;
		Row.MinForgeTier = EPAForgeTier::Tier1_Outpost;
		Row.TargetStat = FName("Defense");
		Row.Tier1Range = FVector2D(8.0f, 15.0f);
		Row.Tier2Range = FVector2D(20.0f, 35.0f);
		Row.Tier3Range = FVector2D(45.0f, 70.0f);
		Row.Tier4Range = FVector2D(85.0f, 120.0f);
		Rows.Add(Row);
	}
	{
		FPAAffixDefinitionRow Row;
		Row.AffixName = FName("prefix_elemental_lightning");
		Row.DisplayName = FText::FromString(TEXT("Tiền Tố: Lôi Điện Hoang Dã (Flat Lightning DMG)"));
		Row.AffixType = EPAAffixType::Prefix;
		Row.MinForgeTier = EPAForgeTier::Tier1_Outpost;
		Row.TargetStat = FName("LightningDamage");
		Row.Tier1Range = FVector2D(4.0f, 7.0f);
		Row.Tier2Range = FVector2D(9.0f, 16.0f);
		Row.Tier3Range = FVector2D(20.0f, 30.0f);
		Row.Tier4Range = FVector2D(35.0f, 50.0f);
		Rows.Add(Row);
	}
	{
		FPAAffixDefinitionRow Row;
		Row.AffixName = FName("prefix_mana_flat");
		Row.DisplayName = FText::FromString(TEXT("Tiền Tố: Ma Lực Tinh Khiết (Flat Max Mana)"));
		Row.AffixType = EPAAffixType::Prefix;
		Row.MinForgeTier = EPAForgeTier::Tier1_Outpost;
		Row.TargetStat = FName("MaxMana");
		Row.Tier1Range = FVector2D(15.0f, 25.0f);
		Row.Tier2Range = FVector2D(35.0f, 55.0f);
		Row.Tier3Range = FVector2D(70.0f, 100.0f);
		Row.Tier4Range = FVector2D(120.0f, 160.0f);
		Rows.Add(Row);
	}

	// --- BẬC 2: DÃ NGOẠI (FIELD FORGE BRACKET - ILVL 21-35) --- 3 Tiền Tố mới
	{
		FPAAffixDefinitionRow Row;
		Row.AffixName = FName("prefix_armor_pen_pct");
		Row.DisplayName = FText::FromString(TEXT("Tiền Tố: Xuyên Giáp Xương Tủy (% Armor Penetration)"));
		Row.AffixType = EPAAffixType::Prefix;
		Row.MinForgeTier = EPAForgeTier::Tier2_Field;
		Row.TargetStat = FName("ArmorPenetration");
		Row.Tier2Range = FVector2D(6.0f, 10.0f);
		Row.Tier3Range = FVector2D(12.0f, 18.0f);
		Row.Tier4Range = FVector2D(20.0f, 28.0f);
		Rows.Add(Row);
	}
	{
		FPAAffixDefinitionRow Row;
		Row.AffixName = FName("prefix_dot_bleed");
		Row.DisplayName = FText::FromString(TEXT("Tiền Tố: Lưỡi Cưa Rách Thịt (Bleed DoT DMG/s)"));
		Row.AffixType = EPAAffixType::Prefix;
		Row.MinForgeTier = EPAForgeTier::Tier2_Field;
		Row.TargetStat = FName("BleedDamageBonus");
		Row.StackingRule = TEXT("Personal Outgoing (Max 3 stacks per player, Global Cap 10 players on Boss, Grace Period 2.0s, 100% damage ledger retained)");
		Row.Tier2Range = FVector2D(15.0f, 20.0f);
		Row.Tier3Range = FVector2D(21.0f, 25.0f);
		Row.Tier4Range = FVector2D(26.0f, 32.0f);
		Rows.Add(Row);
	}
	{
		FPAAffixDefinitionRow Row;
		Row.AffixName = FName("prefix_stagger_duration");
		Row.DisplayName = FText::FromString(TEXT("Tiền Tố: Chấn Động Kéo Dài (Stagger Duration Seconds)"));
		Row.AffixType = EPAAffixType::Prefix;
		Row.MinForgeTier = EPAForgeTier::Tier2_Field;
		Row.TargetStat = FName("StaggerDurationExtension");
		Row.HardCapPct = 0.5f; // Cap 0.5s extension
		Row.StackingRule = TEXT("Max Rule (Non-additive across players, Hard Cap +0.5s, Max Boss Stagger 3.5s)");
		Row.Tier2Range = FVector2D(0.2f, 0.3f);
		Row.Tier3Range = FVector2D(0.3f, 0.4f);
		Row.Tier4Range = FVector2D(0.4f, 0.5f);
		Rows.Add(Row);
	}

	// --- BẬC 3: CẤM ĐỊA (FORBIDDEN FORGE BRACKET - ILVL 36-50) --- 3 Tiền Tố mới
	{
		FPAAffixDefinitionRow Row;
		Row.AffixName = FName("prefix_execution_dmg_pct");
		Row.DisplayName = FText::FromString(TEXT("Tiền Tố: Trảm Quyết Tử Thần (% Execution True DMG Amp)"));
		Row.AffixType = EPAAffixType::Prefix;
		Row.MinForgeTier = EPAForgeTier::Tier3_Forbidden;
		Row.TargetStat = FName("ExecutionDamageBonus");
		Row.StackingRule = TEXT("Single Instigator Calculation (Exclusive to executing player at Socket_Execution, does not stack from other players)");
		Row.Tier3Range = FVector2D(12.0f, 18.0f);
		Row.Tier4Range = FVector2D(20.0f, 25.0f);
		Rows.Add(Row);
	}
	{
		FPAAffixDefinitionRow Row;
		Row.AffixName = FName("prefix_staggered_target_dmg");
		Row.DisplayName = FText::FromString(TEXT("Tiền Tố: Áp Chế Trọng Thương (% DMG to Staggered Targets)"));
		Row.AffixType = EPAAffixType::Prefix;
		Row.MinForgeTier = EPAForgeTier::Tier3_Forbidden;
		Row.TargetStat = FName("StaggeredTargetDamageBonus");
		Row.StackingRule = TEXT("Personal Outgoing (Evaluated per individual attack, no cross-buffing)");
		Row.Tier3Range = FVector2D(12.0f, 18.0f);
		Row.Tier4Range = FVector2D(20.0f, 28.0f);
		Rows.Add(Row);
	}
	{
		FPAAffixDefinitionRow Row;
		Row.AffixName = FName("prefix_all_ele_pct");
		Row.DisplayName = FText::FromString(TEXT("Tiền Tố: Hỗn Nguyên Nguyên Tố (% All Elemental DMG)"));
		Row.AffixType = EPAAffixType::Prefix;
		Row.MinForgeTier = EPAForgeTier::Tier3_Forbidden;
		Row.TargetStat = FName("AllElementalDamageBonus");
		Row.Tier3Range = FVector2D(10.0f, 16.0f);
		Row.Tier4Range = FVector2D(18.0f, 26.0f);
		Rows.Add(Row);
	}

	// =========================================================================
	// 14 HẬU TỐ (SUFFIXES)
	// =========================================================================

	// --- BẬC 1: TIỀN TRẠM (OUTPOST FORGE BRACKET - ILVL 1-20) --- 8 Hậu Tố
	{
		FPAAffixDefinitionRow Row;
		Row.AffixName = FName("suffix_max_hp");
		Row.DisplayName = FText::FromString(TEXT("Hậu Tố: Sinh Lực Dồi Dào (Flat Max HP)"));
		Row.AffixType = EPAAffixType::Suffix;
		Row.MinForgeTier = EPAForgeTier::Tier1_Outpost;
		Row.TargetStat = FName("MaxHealth");
		Row.Tier1Range = FVector2D(15.0f, 25.0f);
		Row.Tier2Range = FVector2D(35.0f, 55.0f);
		Row.Tier3Range = FVector2D(70.0f, 110.0f);
		Row.Tier4Range = FVector2D(130.0f, 180.0f);
		Rows.Add(Row);
	}
	{
		FPAAffixDefinitionRow Row;
		Row.AffixName = FName("suffix_stamina_regen");
		Row.DisplayName = FText::FromString(TEXT("Hậu Tố: Tật Phong Hồi Thể (% Stamina Regen Rate)"));
		Row.AffixType = EPAAffixType::Suffix;
		Row.MinForgeTier = EPAForgeTier::Tier1_Outpost;
		Row.TargetStat = FName("StaminaRegenRatePct");
		Row.Tier1Range = FVector2D(5.0f, 8.0f);
		Row.Tier2Range = FVector2D(10.0f, 15.0f);
		Row.Tier3Range = FVector2D(18.0f, 25.0f);
		Row.Tier4Range = FVector2D(28.0f, 38.0f);
		Rows.Add(Row);
	}
	{
		FPAAffixDefinitionRow Row;
		Row.AffixName = FName("suffix_parry_window");
		Row.DisplayName = FText::FromString(TEXT("Hậu Tố: Phản Xạ Thần Tốc (Parry Window Extension)"));
		Row.AffixType = EPAAffixType::Suffix;
		Row.MinForgeTier = EPAForgeTier::Tier1_Outpost;
		Row.TargetStat = FName("ParryWindowDuration");
		Row.Tier1Range = FVector2D(0.01f, 0.01f);
		Row.Tier2Range = FVector2D(0.02f, 0.02f);
		Row.Tier3Range = FVector2D(0.03f, 0.03f);
		Row.Tier4Range = FVector2D(0.04f, 0.04f);
		Rows.Add(Row);
	}
	{
		FPAAffixDefinitionRow Row;
		Row.AffixName = FName("suffix_move_speed");
		Row.DisplayName = FText::FromString(TEXT("Hậu Tố: Bước Chân Lữ Hành (Movement Speed Flat)"));
		Row.AffixType = EPAAffixType::Suffix;
		Row.MinForgeTier = EPAForgeTier::Tier1_Outpost;
		Row.TargetStat = FName("MovementSpeed");
		Row.Tier1Range = FVector2D(10.0f, 18.0f);
		Row.Tier2Range = FVector2D(22.0f, 35.0f);
		Row.Tier3Range = FVector2D(40.0f, 60.0f);
		Row.Tier4Range = FVector2D(70.0f, 95.0f);
		Rows.Add(Row);
	}
	{
		FPAAffixDefinitionRow Row;
		Row.AffixName = FName("suffix_crit_chance");
		Row.DisplayName = FText::FromString(TEXT("Hậu Tố: Tử Huyệt Chuẩn Xác (% Crit Chance)"));
		Row.AffixType = EPAAffixType::Suffix;
		Row.MinForgeTier = EPAForgeTier::Tier1_Outpost;
		Row.TargetStat = FName("CritChance");
		Row.Tier1Range = FVector2D(2.0f, 3.5f);
		Row.Tier2Range = FVector2D(4.0f, 6.5f);
		Row.Tier3Range = FVector2D(7.0f, 10.0f);
		Row.Tier4Range = FVector2D(11.0f, 15.0f);
		Rows.Add(Row);
	}
	{
		FPAAffixDefinitionRow Row;
		Row.AffixName = FName("suffix_cooldown_red");
		Row.DisplayName = FText::FromString(TEXT("Hậu Tố: Dòng Chảy Ma Lực (% Cooldown Reduction)"));
		Row.AffixType = EPAAffixType::Suffix;
		Row.MinForgeTier = EPAForgeTier::Tier1_Outpost;
		Row.TargetStat = FName("CooldownReduction");
		Row.Tier1Range = FVector2D(2.0f, 4.0f);
		Row.Tier2Range = FVector2D(5.0f, 8.0f);
		Row.Tier3Range = FVector2D(9.0f, 13.0f);
		Row.Tier4Range = FVector2D(14.0f, 20.0f);
		Rows.Add(Row);
	}
	{
		FPAAffixDefinitionRow Row;
		Row.AffixName = FName("suffix_max_stamina");
		Row.DisplayName = FText::FromString(TEXT("Hậu Tố: Bền Bỉ Trường Kỳ (Flat Max Stamina)"));
		Row.AffixType = EPAAffixType::Suffix;
		Row.MinForgeTier = EPAForgeTier::Tier1_Outpost;
		Row.TargetStat = FName("MaxStamina");
		Row.Tier1Range = FVector2D(8.0f, 12.0f);
		Row.Tier2Range = FVector2D(15.0f, 22.0f);
		Row.Tier3Range = FVector2D(28.0f, 38.0f);
		Row.Tier4Range = FVector2D(45.0f, 60.0f);
		Rows.Add(Row);
	}
	{
		FPAAffixDefinitionRow Row;
		Row.AffixName = FName("suffix_poise_flat");
		Row.DisplayName = FText::FromString(TEXT("Hậu Tố: Thế Đứng Kiên Định (Flat Max Posture Poise)"));
		Row.AffixType = EPAAffixType::Suffix;
		Row.MinForgeTier = EPAForgeTier::Tier1_Outpost;
		Row.TargetStat = FName("MaxPosture");
		Row.Tier1Range = FVector2D(10.0f, 18.0f);
		Row.Tier2Range = FVector2D(25.0f, 40.0f);
		Row.Tier3Range = FVector2D(50.0f, 75.0f);
		Row.Tier4Range = FVector2D(90.0f, 120.0f);
		Rows.Add(Row);
	}

	// --- BẬC 2: DÃ NGOẠI (FIELD FORGE BRACKET - ILVL 21-35) --- 3 Hậu Tố mới
	{
		FPAAffixDefinitionRow Row;
		Row.AffixName = FName("suffix_crit_mult");
		Row.DisplayName = FText::FromString(TEXT("Hậu Tố: Tàn Khốc Bạo Liệt (% Crit Multiplier)"));
		Row.AffixType = EPAAffixType::Suffix;
		Row.MinForgeTier = EPAForgeTier::Tier2_Field;
		Row.TargetStat = FName("CritMultiplier");
		Row.Tier2Range = FVector2D(12.0f, 18.0f);
		Row.Tier3Range = FVector2D(20.0f, 30.0f);
		Row.Tier4Range = FVector2D(35.0f, 50.0f);
		Rows.Add(Row);
	}
	{
		FPAAffixDefinitionRow Row;
		Row.AffixName = FName("suffix_dash_stamina_cost");
		Row.DisplayName = FText::FromString(TEXT("Hậu Tố: Khinh Thân Tật Bộ (Dash Stamina Cost Reduction)"));
		Row.AffixType = EPAAffixType::Suffix;
		Row.MinForgeTier = EPAForgeTier::Tier2_Field;
		Row.TargetStat = FName("DashStaminaCostMod");
		Row.Tier2Range = FVector2D(2.0f, 3.0f);
		Row.Tier3Range = FVector2D(4.0f, 6.0f);
		Row.Tier4Range = FVector2D(7.0f, 10.0f);
		Rows.Add(Row);
	}
	{
		FPAAffixDefinitionRow Row;
		Row.AffixName = FName("suffix_cc_resist");
		Row.DisplayName = FText::FromString(TEXT("Hậu Tố: Ý Chí Bất Khuất (% Crowd Control Resistance)"));
		Row.AffixType = EPAAffixType::Suffix;
		Row.MinForgeTier = EPAForgeTier::Tier2_Field;
		Row.TargetStat = FName("CrowdControlResistance");
		Row.Tier2Range = FVector2D(10.0f, 15.0f);
		Row.Tier3Range = FVector2D(18.0f, 25.0f);
		Row.Tier4Range = FVector2D(30.0f, 40.0f);
		Rows.Add(Row);
	}

	// --- BẬC 3: CẤM ĐỊA (FORBIDDEN FORGE BRACKET - ILVL 36-50) --- 3 Hậu Tố mới
	{
		FPAAffixDefinitionRow Row;
		Row.AffixName = FName("suffix_parry_posture_reflect");
		Row.DisplayName = FText::FromString(TEXT("Hậu Tố: Kình Lực Nghịch Chuyển (% Parry Posture Reflect)"));
		Row.AffixType = EPAAffixType::Suffix;
		Row.MinForgeTier = EPAForgeTier::Tier3_Forbidden;
		Row.TargetStat = FName("ParryPostureReflectBonus");
		Row.StackingRule = TEXT("Event-Driven with Diminishing Returns (100% -> 50% -> 25% on simultaneous parries within 1.0s)");
		Row.Tier3Range = FVector2D(8.0f, 15.0f);
		Row.Tier4Range = FVector2D(16.0f, 25.0f);
		Rows.Add(Row);
	}
	{
		FPAAffixDefinitionRow Row;
		Row.AffixName = FName("suffix_perfect_dodge_buff");
		Row.DisplayName = FText::FromString(TEXT("Hậu Tố: Ảo Ảnh Phản Kích (% Next Attack Buff after Perfect Dodge)"));
		Row.AffixType = EPAAffixType::Suffix;
		Row.MinForgeTier = EPAForgeTier::Tier3_Forbidden;
		Row.TargetStat = FName("PerfectDodgeEmpowerPct");
		Row.Tier3Range = FVector2D(10.0f, 16.0f);
		Row.Tier4Range = FVector2D(18.0f, 25.0f);
		Rows.Add(Row);
	}
	{
		FPAAffixDefinitionRow Row;
		Row.AffixName = FName("suffix_leech_on_stagger");
		Row.DisplayName = FText::FromString(TEXT("Hậu Tố: Huyết Tế Đoạt Hồn (% HP/Mana Leech on Execution)"));
		Row.AffixType = EPAAffixType::Suffix;
		Row.MinForgeTier = EPAForgeTier::Tier3_Forbidden;
		Row.TargetStat = FName("LeechOnExecutionPct");
		Row.StackingRule = TEXT("Single Instigator Calculation (Restores HP/Mana exclusively to the executing player)");
		Row.Tier3Range = FVector2D(8.0f, 12.0f);
		Row.Tier4Range = FVector2D(14.0f, 20.0f);
		Rows.Add(Row);
	}

	return Rows;
}
