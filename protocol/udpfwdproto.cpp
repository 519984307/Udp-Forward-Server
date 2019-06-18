#include "udpfwdproto.h"
#include <cryptopp/osrng.h>
#include <qiodevicesource.h>
#include <qiodevicesink.h>
#include "announcepeermessage.h"
#include "dismisspeermessage.h"
#include "tunnelinmessage.h"
#include "tunneloutmessage.h"
#include "errormessage.h"
using namespace CryptoPP;
using namespace CryptoQQ;
using namespace UdpFwdProto;

void qdep_udp_fwd_protocol_init();
#ifndef QDEP_BUILD
#include <QCoreApplication>
Q_COREAPP_STARTUP_FUNCTION(qdep_udp_fwd_protocol_init)
#endif

void qdep_udp_fwd_protocol_init()
{
	UdpFwdProto::registerTypes();
}

void UdpFwdProto::registerTypes()
{
	qRegisterMetaType<PublicKey>();
	qRegisterMetaType<PrivateKey>();
	qRegisterMetaType<AnnouncePeerMessage>();
	qRegisterMetaType<DismissPeerMessage>();
	qRegisterMetaType<TunnelInMessage>();
	qRegisterMetaType<TunnelOutMessage>();
	qRegisterMetaType<ErrorMessage>();

	// special types
	qRegisterMetaType<std::optional<PublicKey>>();
	qRegisterMetaType<std::optional<PrivateKey>>();

	// operators
	qRegisterMetaTypeStreamOperators<PublicKey>();
	qRegisterMetaTypeStreamOperators<PrivateKey>();
	qRegisterMetaTypeStreamOperators<std::optional<PublicKey>>();
	qRegisterMetaTypeStreamOperators<std::optional<PrivateKey>>();
}

OID UdpFwdProto::defaultCurve()
{
	return ASN1::brainpoolP256r1();
}

QDataStream &UdpFwdProto::operator<<(QDataStream &stream, const CryptoMaterial &key)
{
	QByteArray data;
	QByteArraySink sink{data};
	key.Save(sink);
	stream.writeBytes(data.data(), static_cast<uint>(data.size()));
	return stream;
}

QDataStream &UdpFwdProto::operator>>(QDataStream &stream, CryptoMaterial &key)
{
	thread_local AutoSeededRandomPool rng;

	stream.startTransaction();
	char *data = nullptr;
	uint len = 0;
	stream.readBytes(data, len);
	QByteArraySource source{QByteArray::fromRawData(data, static_cast<int>(len)), true};
	key.Load(source);
	if (key.Validate(rng, 3))
		stream.commitTransaction();
	else
		stream.abortTransaction();
	return stream;
}

QByteArray UdpFwdProto::fingerPrint(const PublicKey &key)
{
	QByteArray keydata;
	QByteArraySink sink{keydata};
	key.Save(sink);

	QByteArray fingerprint;
	SHA3_256 hash;
	QByteArraySource {
		keydata, true, new HashFilter {
			hash, new QByteArraySink {
				fingerprint
			}
		}
	};
	return fingerprint;
}

QByteArray UdpFwdProto::fingerPrint(const PrivateKey &key)
{
	PublicKey pubKey;
	key.MakePublicKey(pubKey);
	return fingerPrint(pubKey);
}

std::optional<UdpFwdProto::PrivateKey> UdpFwdProto::loadPrivateKey(QIODevice *device, RandomNumberGenerator &rng)
{
	PrivateKey key;
	QIODeviceSource source{device, true};
	key.Load(source);
	return key.Validate(rng, 3) ? std::optional<PrivateKey>{std::move(key)} : std::nullopt;
}

std::optional<UdpFwdProto::PublicKey> UdpFwdProto::loadPublicKey(QIODevice *device, RandomNumberGenerator &rng)
{
	PublicKey key;
	QIODeviceSource source{device, true};
	key.Load(source);
	return key.Validate(rng, 3) ? std::optional<PublicKey>{std::move(key)} : std::nullopt;
}

void UdpFwdProto::savePrivateKey(QIODevice *device, const PrivateKey &key)
{
	QIODeviceSink sink{device};
	key.Save(sink);
}

void UdpFwdProto::savePublicKey(QIODevice *device, const PublicKey &key)
{
	QIODeviceSink sink{device};
	key.Save(sink);
}

UdpFwdProto::PrivateKey UdpFwdProto::generateKey(RandomNumberGenerator &rng, const OID &curve)
{
	PrivateKey key;
	do {
		key.Initialize(rng, curve);
	} while(!key.Validate(rng, 3));
	return key;
}

bool CryptoPP::operator!=(const UdpFwdProto::PublicKey &lhs, const UdpFwdProto::PublicKey &rhs)
{
	return fingerPrint(lhs) != fingerPrint(rhs);
}
