// Taken from the exercise given in class 8.


#include "TargetLKPInfo.h"

TargetLKPInfo::TargetLKPInfo()
    :m_currentLKPState(ELKPState::LKPState_Invalid)
    , m_pos(FVector::ZeroVector)
{

}

TargetLKPInfo::TargetLKPInfo(const FString& targetLabel)
{
    m_targetLabel = targetLabel;
}

TargetLKPInfo::~TargetLKPInfo()
{
}
