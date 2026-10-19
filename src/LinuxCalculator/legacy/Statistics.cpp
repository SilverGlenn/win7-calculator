// SPDX-License-Identifier: MIT
#include "Statistics.h"
#include "Header Files/RationalMath.h"
#include "Ratpack/CalcErr.h"
namespace Win7Calc {
NumericResult calculateStatistic(std::span<const CalcEngine::Rational> samples,Statistic stat){
 using R=CalcEngine::Rational;
 if(samples.empty()||(stat==Statistic::SampleDeviation&&samples.size()<2))return {std::nullopt,NumericError::InsufficientData};
 initializeNumericRuntime();
 try{
  R sum(0);for(const auto&value:samples)sum+=value;
  if(stat==Statistic::Sum)return {sum,NumericError::None};
  const R count(static_cast<uint64_t>(samples.size())),mean=sum/count;
  if(stat==Statistic::Mean)return {mean,NumericError::None};
  if(stat==Statistic::MeanSquare||stat==Statistic::SumSquare){
   R squares(0);for(const auto&value:samples)squares+=value*value;
   return {stat==Statistic::SumSquare?squares:squares/count,NumericError::None};
  }
  if(stat==Statistic::PopulationDeviation||stat==Statistic::SampleDeviation){
   // Center first to avoid subtracting huge nearly equal aggregates.
   R variance(0);for(const auto&value:samples){const R delta=value-mean;variance+=delta*delta;}
   const R divisor=stat==Statistic::SampleDeviation?count-R(1):count;
   return {CalcEngine::RationalMath::Root(variance/divisor,R(2)),NumericError::None};
  }
  return {std::nullopt,NumericError::InvalidInput};
 }catch(uint32_t error){return {std::nullopt,error==CALC_E_OVERFLOW?NumericError::Overflow:NumericError::Domain};}
}
}
