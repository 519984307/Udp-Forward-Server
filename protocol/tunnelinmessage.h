#ifndef TUNNELINMESSAGE_H
#define TUNNELINMESSAGE_H

#include "payloadmessagebase.h"

namespace UdpFwdProto {

struct Q_UDP_FWD_PROTOCOL_EXPORT PrivateReplyInfo : public ReplyInfo
{
	Q_GADGET

	Q_PROPERTY(UdpFwdProto::PrivateKey privKey MEMBER privKey)

public:
	PrivateKey privKey;

	inline PrivateReplyInfo() = default;
	PrivateReplyInfo(PrivateKey key, quint16 limit = 1);
};

struct Q_UDP_FWD_PROTOCOL_EXPORT TunnelInMessage : public PayloadMessageBase
{
	Q_GADGET

	Q_PROPERTY(QByteArray peer MEMBER peer)
	Q_PROPERTY(bool lastReply MEMBER lastReply)
	Q_PROPERTY(QByteArray signature MEMBER signature)

public:
	static constexpr quint8 MessageCode = 0x03;

	QByteArray peer;
	bool lastReply = false;
	QByteArray signature;

	static TunnelInMessage createEncrypted(CryptoPP::RandomNumberGenerator &rng,
										   const PublicKey &key,
										   const QByteArray &data,
										   PrivateReplyInfo replyInfo = {},
										   bool lastReply = false);
	bool verifySignature() const;
};

}

Q_DECLARE_METATYPE(UdpFwdProto::PrivateReplyInfo)
Q_DECLARE_METATYPE(UdpFwdProto::TunnelInMessage)

#endif // TUNNELINMESSAGE_H
