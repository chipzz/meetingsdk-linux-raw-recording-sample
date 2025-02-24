#include "AuthServiceEventListener.h"
#include <string>

std::string ToString(LOGINSTATUS &status)
{
	switch (status)
	{
		case LOGINSTATUS::LOGIN_IDLE: return "IDLE"; break;
		case LOGINSTATUS::LOGIN_PROCESSING: return "PROCESSING"; break;
		case LOGINSTATUS::LOGIN_SUCCESS: return "SUCCESS"; break;
		case LOGINSTATUS::LOGIN_FAILED: return "FAILED"; break;
	}
	return "";
}

std::string ToJSONString(LOGINSTATUS &status)
{
	return ToString(status); // FIXME: Should be int
}

std::string ToString(LoginFailReason &reason)
{
	switch (reason)
	{
		case LoginFailReason::LoginFail_None: return "None"; break;
		case LoginFailReason::LoginFail_EmailLoginDisable: return "EmailLoginDisable"; break;
		case LoginFailReason::LoginFail_UserNotExist: return "UserNotExist"; break;
		case LoginFailReason::LoginFail_WrongPassword: return "WrongPassword"; break;
		case LoginFailReason::LoginFail_AccountLocked: return "AccountLocked"; break;
		case LoginFailReason::LoginFail_SDKNeedUpdate: return "SDKNeedUpdate"; break;
		case LoginFailReason::LoginFail_TooManyFailedAttempts: return "TooManyFailedAttempts"; break;
		case LoginFailReason::LoginFail_SMSCodeError: return "SMSCodeError"; break;
		case LoginFailReason::LoginFail_SMSCodeExpired: return "SMSCodeExpired"; break;
		case LoginFailReason::LoginFail_PhoneNumberFormatInValid: return "PhoneNumberFormatInValid"; break;
		case LoginFailReason::LoginFail_LoginTokenInvalid: return "LoginTokenInvalid"; break;
		case LoginFailReason::LoginFail_UserDisagreeLoginDisclaimer: return "UserDisagreeLoginDisclaimer"; break;
		case LoginFailReason::LoginFail_Mfa_Required: return "Mfa_Required"; break;
		case LoginFailReason::LoginFail_Need_Bitrthday_ask: return "Need_Bitrthday_ask"; break;
		case LoginFailReason::LoginFail_OtherIssue: return "OtherIssue"; break;
	}
	return "";
}

std::string ToJSONString(LoginFailReason &reason)
{
	return ToString(reason); // FIXME: Should be int
}

std::string ToString(AuthResult &result)
{
	switch (result)
	{
		case AuthResult::AUTHRET_SUCCESS: return "SUCCESS"; break;
		case AuthResult::AUTHRET_KEYORSECRETEMPTY: return "KEYORSECRETEMPTY"; break;
		case AuthResult::AUTHRET_KEYORSECRETWRONG: return "KEYORSECRETWRONG"; break;
		case AuthResult::AUTHRET_ACCOUNTNOTSUPPORT: return "ACCOUNTNOTSUPPORT"; break;
		case AuthResult::AUTHRET_ACCOUNTNOTENABLESDK: return "ACCOUNTNOTENABLESDK"; break;
		case AuthResult::AUTHRET_UNKNOWN: return "UNKNOWN"; break;
		case AuthResult::AUTHRET_SERVICE_BUSY: return "SERVICE_BUSY"; break;
		case AuthResult::AUTHRET_NONE: return "NONE"; break;
		case AuthResult::AUTHRET_OVERTIME: return "OVERTIME"; break;
		case AuthResult::AUTHRET_NETWORKISSUE: return "NETWORKISSUE"; break;
		case AuthResult::AUTHRET_CLIENT_INCOMPATIBLE: return "CLIENT_INCOMPATIBLE"; break;
		case AuthResult::AUTHRET_JWTTOKENWRONG: return "JWTTOKENWRONG"; break;
		case AuthResult::AUTHRET_LIMIT_EXCEEDED_EXCEPTION: return "LIMIT_EXCEEDED_EXCEPTION"; break;
	}
	return "";
}

std::string ToJSONString(AuthResult &result)
{
	return ToString(result); // FIXME: Should be int
}

/*
#include <ostream>

std::ostream &operator<<(std::ostream &os, AuthResult &result)
{
	os << ToString(result);
	return os;
}

std::ostream &operator<<(std::ostream &os, LOGINSTATUS &status)
{
	os << ToString(status);
	return os;
}

std::ostream &operator<<(std::ostream &os, LoginFailReason &reason)
{
	os << ToString(reason);
	return os;
}
*/

#include "Log.h"

AuthServiceEventListener::AuthServiceEventListener(void(*onAuthSuccess)())
{
    LOG_CALLBACK("AuthServiceEventListener", "AuthServiceEventListener");
    onAuthSuccess_ = onAuthSuccess;
}

void AuthServiceEventListener::onAuthenticationReturn(AuthResult result)
{
    LOG_CALLBACK("AuthServiceEventListener", "onAuthenticationReturn", result);
    if (result == AuthResult::AUTHRET_SUCCESS && onAuthSuccess_)
        onAuthSuccess_();
}

void AuthServiceEventListener::onLoginReturnWithReason(LOGINSTATUS status, IAccountInfo* pAccountInfo, LoginFailReason reason)
{
    LOG_CALLBACK("AuthServiceEventListener", "onLoginReturnWithReason", status, reason);
}

void AuthServiceEventListener::onLogout()
{
    LOG_CALLBACK("AuthServiceEventListener", "onLogout");
}

void AuthServiceEventListener::onZoomIdentityExpired()
{
    LOG_CALLBACK("AuthServiceEventListener", "onZoomIdentityExpired");
}

void AuthServiceEventListener::onZoomAuthIdentityExpired()
{
    LOG_CALLBACK("AuthServiceEventListener", "onZoomAuthIdentityExpired");
}

/*
void AuthServiceEventListener::onNotificationServiceStatus(SDKNotificationServiceStatus status)
{
    LOG_CALLBACK("AuthServiceEventListener", "onNotificationServiceStatus", status);
}
*/
