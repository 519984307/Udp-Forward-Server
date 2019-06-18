#include "tunnelinmessage.h"
#include "message_p.h"
using namespace CryptoPP;
using namespace UdpFwdProto;
using namespace UdpFwdProto::__internal;

TunnelInMessage TunnelInMessage::createEncrypted(RandomNumberGenerator &rng, const PublicKey &key, const QByteArray &data, PrivateReplyInfo replyInfo, bool lastReply)
{
	TunnelInMessage message;
	message.PayloadMessageBase::createEncrypted(rng, key, data, std::move(replyInfo)); // will only move out the public part, private stays intact
	message.peer = fingerPrint(key);
	message.lastReply = lastReply;
	if (message.replyInfo) {
		SignatureStream stream;
		stream << message.encryptedKey
			   << message.iv
			   << message.encryptedPayload
			   << message.replyInfo
			   << message.peer
			   << message.lastReply;
		message.signature = stream.sign(rng, replyInfo.privKey); // okay because only base was moved
	}

	return message;
}

bool TunnelInMessage::verifySignature() const
{
	if (replyInfo) {
		SignatureStream stream;
		stream << encryptedKey
			   << iv
			   << encryptedPayload
			   << replyInfo
			   << peer
			   << lastReply;
		return stream.verify(signature, replyInfo.key);
	} else
		return false;
}



PrivateReplyInfo::PrivateReplyInfo(PrivateKey key, quint16 limit) :
	ReplyInfo{{}, limit},
	privKey{std::move(key)}
{
	privKey.MakePublicKey(this->key);
}
