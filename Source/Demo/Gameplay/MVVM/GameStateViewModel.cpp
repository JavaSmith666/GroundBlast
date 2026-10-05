#include "GameStateViewModel.h"

void UGameStateViewModel::SetCountDownLeftTime(int32 NewCountDownLeftTime)
{
	// 核心宏：如果值确实变了，自动赋值并触发通知
	// 内部等价于：if (PlayerName != NewName) { PlayerName = NewName; BroadcastFieldValueChanged(...); }
	UE_MVVM_SET_PROPERTY_VALUE(CountDownLeftTime, NewCountDownLeftTime);
}

void UGameStateViewModel::SetCurrentAICount(int32 NewAICount)
{
	UE_MVVM_SET_PROPERTY_VALUE(CurrentAICount, NewAICount);
}

void UGameStateViewModel::SetCurrentDefeatCount(int32 NewDefeatCount)
{
	UE_MVVM_SET_PROPERTY_VALUE(CurrentDefeatCount, NewDefeatCount);
}

void UGameStateViewModel::SetCurrentRoundIndex(int32 NewRoundIndex)
{
	UE_MVVM_SET_PROPERTY_VALUE(CurrentRoundIndex, NewRoundIndex);
}
