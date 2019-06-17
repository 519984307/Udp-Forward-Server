#ifndef DISMISSPEERMESSAGE_H
#define DISMISSPEERMESSAGE_H

#include "message.h"

namespace UdpFwdProto {

struct Q_UDP_FWD_PROTOCOL_EXPORT DismissPeerMessage : public Message
{
	Q_GADGET

	Q_PROPERTY(bool clearReplies MEMBER clearReplies)
	Q_PROPERTY(QByteArray signature MEMBER signature)

public:
	static constexpr quint8 MessageCode = 0x02;

	bool clearReplies = true;
	QByteArray signature;

	static DismissPeerMessage createSigned(CryptoPP::RandomNumberGenerator &rng,
										   const PrivateKey &key,
										   bool clearReplies = false);
	bool verifySignature(const PublicKey &key) const;
};

}

Q_DECLARE_METATYPE(UdpFwdProto::DismissPeerMessage)

#endif // DISMISSPEERMESSAGE_H
