// Fill out your copyright notice in the Description page of Project Settings.


#include "Utility/Logger.h"

void FLogger::DebugLog(const FString& Msg, const UObject* Context) const
{
	if (bDebugMode)
	{
		FString Prefix = Context
			? FString::Printf(TEXT("[%s] "), *Context->GetClass()->GetName())
			: TEXT("[Debug] ");

		UE_LOG(LogTemp, Warning, TEXT("%s%s"), *Prefix, *Msg);
	}
}

void FLogger::ErrorLog(const FString& Msg, const UObject* Context) const
{
	FString Prefix = Context
		? FString::Printf(TEXT("[%s] "), *Context->GetClass()->GetName())
		: TEXT("[Error] ");
	UE_LOG(LogTemp, Error, TEXT("%s%s"), *Prefix, *Msg);
}

void FLogger::DebugTraces()
{
}