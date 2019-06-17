#ifndef ANNOUNCEPEERMESSAGE_H
#define ANNOUNCEPEERMESSAGE_H

#include "message.h"

namespace UdpFwdProto {

struct Q_UDP_FWD_PROTOCOL_EXPORT AnnouncePeerMessage : public Message
{
	Q_GADGET

	Q_PROPERTY(PublicKey key MEMBER key)
	Q_PROPERTY(bool oneTime MEMBER oneTime)
	Q_PROPERTY(QByteArray signature MEMBER signature)

public:
	static constexpr quint8 MessageCode = 0x01;

	PublicKey key;
	bool oneTime = false;
	QByteArray signature;

	static AnnouncePeerMessage createSigned(CryptoPP::RandomNumberGenerator &rng,
											const PrivateKey &key,
											bool oneTime = false);
	bool verifySignature() const;
};

}

Q_DECLARE_METATYPE(UdpFwdProto::AnnouncePeerMessage)

#endif // ANNOUNCEPEERMESSAGE_H
