#include "NetworkConnectionHandler.h"
#include "Log.h"

NetworkConnectionHandler::NetworkConnectionHandler(void (*postToDo)())
{
	LOG_CALLBACK("NetworkConnectionHandler", "NetworkConnectionHandler");
	postToDo_ = postToDo;
}

void NetworkConnectionHandler::onProxyDetectComplete()
{
	LOG_CALLBACK("NetworkConnectionHandler", "onProxyDetectComplete");
	if (postToDo_) postToDo_();
}

void NetworkConnectionHandler::onProxySettingNotification(IProxySettingHandler* handler)
{
	LOG_CALLBACK("NetworkConnectionHandler", "onProxySettingNotification");
}

void NetworkConnectionHandler::onSSLCertVerifyNotification(ISSLCertVerificationHandler* handler)
{
	LOG_CALLBACK("NetworkConnectionHandler", "onSSLCertVerifyNotification");
}