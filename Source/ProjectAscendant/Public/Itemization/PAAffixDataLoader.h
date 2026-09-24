// Copyright Project Ascendant. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Engine/DataTable.h"
#include "Itemization/PAAffixTypes.h"
#include "PAAffixDataLoader.generated.h"

/**
 * UPAAffixDataLoader
 *
 * Tiện ích tải và đăng ký 28 Affix từ entities.yaml:475 vào DataTable DT_AffixDefinitions.
 */
UCLASS()
class PROJECTASCENDANT_API UPAAffixDataLoader : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Khởi tạo hoặc cập nhật DataTable DT_AffixDefinitions với đầy đủ 28 Affix
	 * (14 Tiền tố + 14 Hậu tố) theo chuẩn Itemization GDD.
	 */
	UFUNCTION(BlueprintCallable, Category = "Itemization|Data")
	static UDataTable* CreateAffixDefinitionsTable(UObject* Outer = nullptr);

	/**
	 * Đổ dữ liệu 28 dòng affix vào DataTable đã có sẵn.
	 */
	UFUNCTION(BlueprintCallable, Category = "Itemization|Data")
	static void PopulateAffixDefinitionsTable(UDataTable* InDataTable);

	/**
	 * Tạo danh sách toàn bộ 28 định nghĩa Affix ở dạng struct C++.
	 */
	static TArray<FPAAffixDefinitionRow> GetCanonicalAffixDefinitions();
};
