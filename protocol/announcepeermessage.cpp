#include "announcepeermessage.h"
#include "message_p.h"
using namespace CryptoPP;
using namespace UdpFwdProto;
using namespace UdpFwdProto::__internal;

AnnouncePeerMessage AnnouncePeerMessage::createSigned(RandomNumberGenerator &rng, const PrivateKey &key, bool oneTime)
{
	AnnouncePeerMessage message;
	key.MakePublicKey(message.key);
	message.oneTime = oneTime;

	SignatureStream sigStream;
	sigStream << message.key
			  << message.oneTime;
	message.signature = sigStream.sign(rng, key);
	return message;
}

bool AnnouncePeerMessage::verifySignature() const
{
	SignatureStream stream;
	stream << key
		   << oneTime;
	return stream.verify(signature, key);
}
