#ifndef PAYLOADMESSAGEBASE_H
#define PAYLOADMESSAGEBASE_H

#include "message.h"

#include <cryptoqq.h>

namespace UdpFwdProto {

struct Q_UDP_FWD_PROTOCOL_EXPORT PayloadMessageBase : public Message
{
	Q_GADGET

	Q_PROPERTY(QByteArray encryptedKey MEMBER encryptedKey)
	Q_PROPERTY(QByteArray iv MEMBER iv)
	Q_PROPERTY(QByteArray encryptedPayload MEMBER encryptedPayload)
	Q_PROPERTY(std::optional<UdpFwdProto::PublicKey> replyKey MEMBER replyKey)

public:
	QByteArray encryptedKey;
	CryptoQQ::ByteArray iv;
	QByteArray encryptedPayload;
	std::optional<PublicKey> replyKey;

	QByteArray decrypt(CryptoPP::RandomNumberGenerator &rng, const PrivateKey &key) const;

protected:
	PayloadMessageBase() = default;

	void createEncrypted(CryptoPP::RandomNumberGenerator &rng,
						 const PublicKey &key,
						 const QByteArray &data,
						 std::optional<PublicKey> &&replyKey);
};

}

#endif // PAYLOADMESSAGEBASE_H
