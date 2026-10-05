#pragma once
#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "GameStateViewModel.generated.h"

UCLASS(BlueprintType)
class UGameStateViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	// ===== Setter（核心：触发通知的地方）=====
	void SetCountDownLeftTime(int32 NewCountDownLeftTime);
	void SetCurrentAICount(int32 NewAICount);
	void SetCurrentDefeatCount(int32 NewDefeatCount);
	void SetCurrentRoundIndex(int32 NewRoundIndex);

	// ===== Getter =====
	int32 GetCountDownLeftTime() const { return CountDownLeftTime; }
	int32 GetCurrentAICount() const { return CurrentAICount; }
	int32 GetCurrentDefeatCount() const { return CurrentDefeatCount; }
	int32 GetCurrentRoundIndex() const { return CurrentRoundIndex; }

protected:
	// FieldNotify 宏：告诉引擎"这个属性需要支持变更通知"
	// Setter/Getter 宏：自动生成绑定所需的元数据
	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter)
	int32 CountDownLeftTime;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter)
	int32 CurrentAICount;
	
	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter)
	int32 CurrentDefeatCount;
	
	UPROPERTY(BlueprintReadWrite, FieldNotify, Setter, Getter)
	int32 CurrentRoundIndex;
};