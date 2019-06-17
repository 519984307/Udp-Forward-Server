#include "tunnelinmessage.h"
#include "message_p.h"
using namespace CryptoPP;
using namespace UdpFwdProto;
using namespace UdpFwdProto::__internal;

TunnelInMessage TunnelInMessage::createEncrypted(RandomNumberGenerator &rng, const PublicKey &key, const QByteArray &data, std::optional<PrivateKey> replyKey)
{
	std::optional<PublicKey> pubReplyKey;
	if (replyKey) {
		pubReplyKey = PublicKey{};
		replyKey->MakePublicKey(*pubReplyKey);
	}

	TunnelInMessage message;
	message.PayloadMessageBase::createEncrypted(rng, key, data, std::move(pubReplyKey));
	message.peer = fingerPrint(key);

	if (replyKey) {
		SignatureStream stream;
		stream << message.peer
			   << message.encryptedKey
			   << message.iv
			   << message.encryptedPayload
			   << message.replyKey;
		message.signature = stream.sign(rng, *replyKey);
	}

	return message;
}

bool TunnelInMessage::verifySignature() const
{
	if (replyKey) {
		SignatureStream stream;
		stream << peer
			   << encryptedKey
			   << iv
			   << encryptedPayload
			   << replyKey;
		return stream.verify(signature, *replyKey);
	} else
		return true;  // returns true as messages without a key do not need to be signed and thus are valid without a signature
}
