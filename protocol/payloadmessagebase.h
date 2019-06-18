#ifndef PAYLOADMESSAGEBASE_H
#define PAYLOADMESSAGEBASE_H

#include "message.h"

#include <cryptoqq.h>

namespace UdpFwdProto {

struct Q_UDP_FWD_PROTOCOL_EXPORT ReplyInfo
{
	Q_GADGET

	Q_PROPERTY(quint16 limit MEMBER limit)
	Q_PROPERTY(UdpFwdProto::PublicKey key MEMBER key)

public:
	quint16 limit = 0;
	PublicKey key;

	inline ReplyInfo() = default;
	ReplyInfo(PublicKey key, quint16 limit = 1);

	operator bool() const;
	bool operator!() const;
};

struct Q_UDP_FWD_PROTOCOL_EXPORT PayloadMessageBase : public Message
{
	Q_GADGET

	Q_PROPERTY(QByteArray encryptedKey MEMBER encryptedKey)
	Q_PROPERTY(QByteArray iv MEMBER iv)
	Q_PROPERTY(QByteArray encryptedPayload MEMBER encryptedPayload)
	Q_PROPERTY(UdpFwdProto::ReplyInfo replyInfo MEMBER replyInfo)

public:
	QByteArray encryptedKey;
	CryptoQQ::ByteArray iv;
	QByteArray encryptedPayload;
	ReplyInfo replyInfo;

	QByteArray decrypt(CryptoPP::RandomNumberGenerator &rng, const PrivateKey &key) const;

protected:
	PayloadMessageBase() = default;

	void createEncrypted(CryptoPP::RandomNumberGenerator &rng,
						 const PublicKey &key,
						 const QByteArray &data,
						 ReplyInfo replyInfo = {});
};

}

Q_UDP_FWD_PROTOCOL_EXPORT QDataStream &operator<<(QDataStream &stream, const UdpFwdProto::ReplyInfo &info);
Q_UDP_FWD_PROTOCOL_EXPORT QDataStream &operator>>(QDataStream &stream, UdpFwdProto::ReplyInfo &info);

Q_DECLARE_METATYPE(UdpFwdProto::ReplyInfo)

#endif // PAYLOADMESSAGEBASE_H
