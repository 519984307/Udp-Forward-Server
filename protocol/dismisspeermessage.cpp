#include "dismisspeermessage.h"
#include "message_p.h"
using namespace CryptoPP;
using namespace UdpFwdProto;
using namespace UdpFwdProto::__internal;

DismissPeerMessage DismissPeerMessage::createSigned(RandomNumberGenerator &rng, const PrivateKey &key, bool clearReplies)
{
	DismissPeerMessage message;
	message.clearReplies = clearReplies;

	SignatureStream stream;
	stream << message.clearReplies;
	message.signature = stream.sign(rng, key);
	return message;
}

bool DismissPeerMessage::verifySignature(const PublicKey &key) const
{
	SignatureStream stream;
	stream << clearReplies;
	return stream.verify(signature, key);
}
