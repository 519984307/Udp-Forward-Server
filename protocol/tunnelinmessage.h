#ifndef TUNNELINMESSAGE_H
#define TUNNELINMESSAGE_H

#include "payloadmessagebase.h"

namespace UdpFwdProto {

struct Q_UDP_FWD_PROTOCOL_EXPORT TunnelInMessage : public PayloadMessageBase
{
	Q_GADGET

	Q_PROPERTY(QByteArray peer MEMBER peer)
	Q_PROPERTY(QByteArray signature MEMBER signature)

public:
	static constexpr quint8 MessageCode = 0x03;

	QByteArray peer;
	QByteArray signature;

	static TunnelInMessage createEncrypted(CryptoPP::RandomNumberGenerator &rng,
										   const PublicKey &key,
										   const QByteArray &data,
										   std::optional<PrivateKey> replyKey = std::nullopt);
	bool verifySignature() const;
};

}

Q_DECLARE_METATYPE(UdpFwdProto::TunnelInMessage)

#endif // TUNNELINMESSAGE_H
