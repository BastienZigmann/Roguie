#pragma once

// To be used on inheritance
class FLogger
{
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug", meta = (AllowPrivateAccess = "true"))
	bool bDebugMode = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug", meta = (AllowPrivateAccess = "true"))
	bool bDebugTraces = false;

public:
	virtual void EnableDebug() { bDebugMode = true;}
	virtual void EnableDebugTraces() { bDebugTraces = true; }
	virtual void DisableDebug() { bDebugMode = false; bDebugTraces = false; }
	virtual bool IsDebugOn() const { return bDebugMode; }
	virtual bool IsDebugTracesOn() const { return bDebugTraces; }

	// /!\ Use the macro instead of this !!
	void DebugLog(const FString& Msg, const UObject* Context = nullptr) const;
	void ErrorLog(const FString& Msg, const UObject* Context = nullptr) const;

	virtual void DebugTraces();
	void DebugTraceSphere(UWorld* World, const FVector& Location, float Radius, const FColor& Color, float Thickness = 1.0f, float Duration = 5.0f, bool Persistent = false) const;
	void DebugTraceCircle(UWorld* World, const FVector& Location, float Radius, const FColor& Color, float Thickness = 1.0f, float Duration = 5.0f, bool Persistent = false) const;
	void DebugTraceRectangle(UWorld* World, const FVector& Location, const FRotator& Rotation, const FVector& Extent, const FColor& Color, float Thickness = 1.0f, float Duration = 5.0f, bool Persistent = false) const;
	void DebugTraceLine(UWorld* World, const FVector& Start, const FVector& End, const FColor& Color, float Thickness = 1.0f, float Duration = 5.0f, bool Persistent = false) const;

private:
	FString GeneratePrefix(const UObject* Context) const;

};