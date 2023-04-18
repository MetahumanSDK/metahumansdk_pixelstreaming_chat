#pragma once

#include "MetahumanSDKResponses.h"
#include "IDetailCustomization.h"

class FMetahumanSDKSettingsDetails : public IDetailCustomization
{
public:
	/** Makes a new instance of this detail layout class for a specific detail view requesting it */
	static TSharedRef<IDetailCustomization> MakeInstance();

	/** IDetailCustomization interface */
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

private:
	static bool GenerateAPIToken(FString& OutAPIToken);
};

struct FAPITokenResponse : public FGenericResponse
{
    virtual void ParseResponse(FString&& Content) override
    {
        TSharedRef<TJsonReader<>> Reader = FJsonStringReader::Create(MoveTemp(Content));

        TSharedPtr<FJsonObject> RootJson;
        if (FJsonSerializer::Deserialize(Reader, RootJson))
        {
            RootJson->TryGetStringField(TEXT("token"), Token);
        }

        if (!Validate())
        {
            bSuccess = false;
            Error = FString::Printf(TEXT("Invalid API token response json: %s"), *Content);
        }
    }

    virtual bool PassContentAsBytes() const override
    {
        return false;
    }

    bool Validate() const
    {
        return Token.Len() > 0;
    }

    FString Token = "";
};