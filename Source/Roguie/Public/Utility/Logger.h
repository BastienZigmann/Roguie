#pragma once

// To be used on inheritance
class FLogger
{
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug", meta = (AllowPrivateAccess = "true"))
	bool bDebugMode = false;

public:
	virtual void EnableDebug() { bDebugMode = true; }
	virtual void DisableDebug() { bDebugMode = false; }
	virtual bool IsDebugOn() const { return bDebugMode; }

	// /!\ Use the macro instead of this !!
	void DebugLog(const FString& Msg, const UObject* Context = nullptr) const;
	void ErrorLog(const FString& Msg, const UObject* Context = nullptr) const;

	virtual void DebugTraces();

private:
	FString GeneratePrefix(const UObject* Context) const;

};